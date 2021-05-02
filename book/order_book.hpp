#pragma once

#include <list>
#include <map>
#include <vector>

#include "callback.hpp"
#include "logger.hpp"
#include "order_tracker.hpp"
#include "types.hpp"

namespace akuna::book {
    template <typename OrderPtr>
    class OrderBook {
    public:
        using Tracker       = OrderTracker<OrderPtr>;
        using TypedCallback = Callback<OrderPtr>;
        using TrackerMap    = std::multimap<ComparablePrice, Tracker>;
        using Callbacks     = std::vector<TypedCallback>;

        explicit OrderBook() {
            callbacks_.reserve(8);
        }

        [[nodiscard]] auto Add(const OrderPtr &order, OrderConditions conditions) -> bool {
            bool matched = false;

            if (order->GetQuantity() <= 0) {
                LOG_DEBUG(*order << " size must be positive");
            } else {
                size_t accept_cb_index = callbacks_.size();
                callbacks_.push_back(TypedCallback::Accept(order));
                Tracker inbound(order, conditions);
                matched                               = SubmitOrder(inbound);
                callbacks_[accept_cb_index].quantity_ = inbound.FilledQty();
                if (inbound.ImmediateOrCancel() && !inbound.Filled()) {
                    callbacks_.push_back(TypedCallback::Cancel(order, 0));
                }
            }
            CallbackNow();
            return matched;
        }

        auto Cancel(const OrderPtr &order) -> void {
            bool     found    = false;
            Quantity open_qty = 0;

            if (order->IsBuy()) {
                typename TrackerMap::iterator bid;
                if (FindOnMarket(order, bid) && bid != bids_.end()) {
                    open_qty = bid->second.OpenQty();
                    bids_.erase(bid);
                    found = true;
                }
            } else {
                typename TrackerMap::iterator ask;
                if (FindOnMarket(order, ask) && ask != asks_.end()) {
                    open_qty = ask->second.OpenQty();
                    asks_.erase(ask);
                    found = true;
                }
            }
            if (found) {
                callbacks_.push_back(TypedCallback::Cancel(order, open_qty));
            } else {
                LOG_DEBUG(*order << " not found");
            }
            CallbackNow();
        }

        [[nodiscard]] auto Replace(const OrderPtr &passivated_order, const OrderPtr &new_order) -> bool {
            bool                          matched = false;
            TrackerMap &                  market  = passivated_order->IsBuy() ? bids_ : asks_;
            typename TrackerMap::iterator pos;

            if (passivated_order->IsBuy() != new_order->IsBuy()) {
                if (FindOnMarket(passivated_order, pos)) {
                    market.erase(pos);
                    matched = Add(new_order, book::OrderCondition::OC_NO_CONDITIONS);
                } else {
                    LOG_DEBUG(*new_order << "not found");
                }
            } else {
                if (FindOnMarket(passivated_order, pos)) {
                    callbacks_.push_back(TypedCallback::Accept(new_order));
                    callbacks_.push_back(TypedCallback::Replace(passivated_order, pos->second.OpenQty(), new_order));
                    market.erase(pos);
                    Tracker inbound(new_order, book::OrderCondition::OC_NO_CONDITIONS);
                    matched = AddOrder(inbound, new_order->GetPrice());
                } else {
                    LOG_DEBUG(*new_order << "not found");
                }
            }

            CallbackNow();
            return matched;
        }

        auto MarketPrice(Price price) -> void {
            market_price_ = price;
        }

        auto MatchOrder(Tracker &inbound, Price inbound_price, TrackerMap &current_orders) -> bool {
            bool matched = false;
            auto pos     = current_orders.begin();
            while (pos != current_orders.end() && !inbound.Filled()) {
                auto                   entry         = pos++;
                const ComparablePrice &current_price = entry->first;
                if (!current_price.Matches(inbound_price)) {
                    break;
                }

                Tracker &current_order = entry->second;
                Quantity traded        = CreateTrade(inbound, current_order);
                if (traded > 0) {
                    matched = true;
                    if (current_order.Filled()) {
                        current_orders.erase(entry);
                    }
                }
            }
            return matched;
        }

        auto CreateTrade(Tracker &inbound_tracker, Tracker &current_tracker, Quantity max_quantity = UINT64_MAX)
                -> Quantity {
            Price cross_price = current_tracker.Ptr()->GetPrice();
            // If current order is a market order, cross at inbound price
            if (MARKET_ORDER_PRICE == cross_price) {
                cross_price = inbound_tracker.Ptr()->GetPrice();
            }
            if (MARKET_ORDER_PRICE == cross_price) {
                cross_price = market_price_;
            }
            if (MARKET_ORDER_PRICE == cross_price) {
                // No price available for this order
                return 0;
            }
            Quantity fill_qty = std::min(max_quantity, std::min(inbound_tracker.OpenQty(), current_tracker.OpenQty()));
            if (fill_qty > 0) {
                inbound_tracker.Fill(fill_qty);
                current_tracker.Fill(fill_qty);
                MarketPrice(cross_price);
                callbacks_.push_back(
                        TypedCallback::Fill(inbound_tracker.Ptr(), current_tracker.Ptr(), fill_qty, cross_price));
            }
            return fill_qty;
        }

        [[nodiscard]] auto FindOnMarket(const OrderPtr &order, typename TrackerMap::iterator &result) -> bool {
            const ComparablePrice KEY(order->IsBuy(), order->GetPrice());
            TrackerMap &          side_map = order->IsBuy() ? bids_ : asks_;

            for (result = side_map.find(KEY); result != side_map.end(); ++result) {
                if (result->second.Ptr() == order) {
                    return true;
                } else if (KEY < result->first) {
                    result = side_map.end();
                    return false;
                }
            }
            return false;
        }

        auto CallbackNow() -> void {
            for (auto &cb : callbacks_) {
                PerformCallback(cb);
            }
            callbacks_.clear();
        }

        auto PerformCallback(TypedCallback &cb) -> void {
            switch (cb.type_) {
                case TypedCallback::CbType::CB_ORDER_FILL:
                    OnFill(cb.order_, cb.matched_order_, cb.quantity_);
                    break;
                case TypedCallback::CbType::CB_ORDER_ACCEPT:
                    OnAccept(cb.order_);
                    break;
                case TypedCallback::CbType::CB_ORDER_CANCEL:
                    OnCancel(cb.order_);
                    break;
                case TypedCallback::CbType::CB_ORDER_REPLACE:
                    OnReplace(cb.order_, cb.delta_, cb.price_);
                    break;
                default: {
                    std::stringstream msg;
                    msg << "Unexpected callback type " << cb.type_;
                    throw std::runtime_error(msg.str());
                }
            }
        }

        auto Log() const -> void {
            LOG_INFO("SELL:");
            std::map<Price, Quantity> book;
            for (auto ask = asks_.begin(); ask != asks_.end(); ++ask) {
                book[ask->first.GetPrice()] += ask->second.OpenQty();
            }
            for (auto level = book.rbegin(); level != book.rend(); ++level) {
                LOG_INFO(level->first << ' ' << level->second);
            }
            book.clear();
            LOG_INFO("BUY:");
            for (auto bid = bids_.begin(); bid != bids_.end(); ++bid) {
                book[bid->first.GetPrice()] += bid->second.OpenQty();
            }
            for (auto level = book.rbegin(); level != book.rend(); ++level) {
                LOG_INFO(level->first << ' ' << level->second);
            }
        }

    private:
        auto SubmitOrder(Tracker &inbound) -> bool {
            Price order_price = inbound.Ptr()->GetPrice();
            return AddOrder(inbound, order_price);
        }

        auto AddOrder(Tracker &inbound, Price order_price) -> bool {
            bool      matched;
            OrderPtr &order = inbound.Ptr();
            if (order->IsBuy()) {
                matched = MatchOrder(inbound, order_price, asks_);
            } else {
                matched = MatchOrder(inbound, order_price, bids_);
            }

            if (inbound.OpenQty() && !inbound.ImmediateOrCancel()) {
                if (order->IsBuy()) {
                    bids_.insert({ComparablePrice(true, order_price), inbound});
                } else {
                    asks_.insert({ComparablePrice(false, order_price), inbound});
                }
            }
            return matched;
        }

        auto OnAccept(const OrderPtr &order) -> void {
            order->OnAccepted();
            LOG_DEBUG("Event: Accepted: " << *order);
        }

        auto OnFill(const OrderPtr &order, const OrderPtr &matched_order, Quantity fill_qty) -> void {
            order->OnFilled(fill_qty);
            matched_order->OnFilled(fill_qty);

            std::stringstream out;
            out << "TRADE " << matched_order->GetOrderId() << ' ' << matched_order->GetPrice() << ' ' << fill_qty << ' '
                << order->GetOrderId() << ' ' << order->GetPrice() << ' ' << fill_qty;
            LOG_INFO(out.str());

            order->AddTradeHistory(matched_order->GetOrderId());
            matched_order->AddTradeHistory(order->GetOrderId());
        }

        auto OnCancel(const OrderPtr &order) -> void {
            order->OnCancelled();
            LOG_DEBUG("Event: Canceled: " << *order);
        }

        auto OnReplace(const OrderPtr &order, Delta delta, Price new_price) -> void {
            order->OnReplaced(delta, new_price);
            LOG_DEBUG("Event: Replaced: " << *order);
        }

        TrackerMap bids_{};
        TrackerMap asks_{};
        Price      market_price_{MARKET_ORDER_PRICE};
        Callbacks  callbacks_{};
    };
}    // namespace akuna::book