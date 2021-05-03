#pragma once
#include <memory>
#include <unordered_map>

#include "logger.hpp"
#include "order.hpp"
#include "order_book.hpp"

namespace akuna::me {
    class Market {
    public:
        using OrderId         = book::OrderId;
        using OrderConditions = book::OrderConditions;
        using OrderPtr        = std::shared_ptr<book::Order>;
        using OrderBook       = book::OrderBook<OrderPtr>;
        using OrderMap        = std::unordered_map<OrderId, OrderPtr>;

        auto OrderEntry(const OrderPtr& order, OrderConditions conditions = book::OrderCondition::OC_NO_CONDITIONS)
                -> bool {
            if (!Validate(order)) {
                return false;
            }
            LOG_DEBUG("ADDING order: " << *order);
            auto order_id = order->GetOrderId();
            bool inserted = AddOrder(order);

            if (inserted && book_.Add(order, conditions)) {
                LOG_DEBUG(order_id << " matched");
                for (const auto& matched_order_id : order->GetTrades()) {
                    auto matched_order = GetOrder(matched_order_id);
                    if (RemoveOrder(matched_order)) {
                        LOG_DEBUG("REMOVED order: " << *matched_order);
                    }
                }
                if (RemoveOrder(order)) {
                    LOG_DEBUG("REMOVED order: " << *order);
                }
            }
            return inserted;
        }

        auto OrderModify(const OrderPtr& order) -> bool {
            bool result = false;
            if (!OrderModifyValidate(order)) {
                return result;
            }
            auto order_id         = order->GetOrderId();
            auto passivated_order = GetOrder(order_id);
            LOG_DEBUG("MODIFYING passivated order: " << *passivated_order << " with order: " << *order);
            if (book_.Replace(passivated_order, order)) {
                for (const auto& matched_order_id : order->GetTrades()) {
                    auto matched_order = GetOrder(matched_order_id);
                    if (RemoveOrder(matched_order)) {
                        LOG_DEBUG("REMOVED order: " << *matched_order);
                    }
                }
                if (RemoveOrder(order)) {
                    LOG_DEBUG("REMOVED order: " << *order);
                }
            }
            if (FoundExistingOrder(order_id)) {
                orders_.at(order_id) = order;
            }
            return !result;
        }

        auto OrderCancel(const OrderId& order_id) -> bool {
            bool result = false;
            auto order  = GetOrder(order_id);
            if (order) {
                LOG_DEBUG("Requesting Cancel: " << *order);
                book_.Cancel(order);
                result = RemoveOrder(order_id);
            }
            return result;
        }

        auto Log() const -> void {
            book_.Log();
        }

    private:
        [[nodiscard]] auto Validate(const OrderPtr& order) -> bool {
            return order->GetPrice() != 0;
        }

        [[nodiscard]] auto OrderModifyValidate(const OrderPtr& order) -> bool {
            return Validate(order) && FoundExistingOrder(order->GetOrderId());
        }

        [[nodiscard]] auto GetOrder(const OrderId& order_id) -> OrderPtr {
            if (FoundExistingOrder(order_id)) {
                return orders_.at(order_id);
            }
            return {};
        }

        [[nodiscard]] auto FoundExistingOrder(const OrderId& order_id) -> bool {
            return orders_.find(order_id) != orders_.end();
        }

        [[nodiscard]] auto AddOrder(const OrderPtr& order) -> bool {
            const auto& order_id = order->GetOrderId();
            bool        inserted = false;
            if (!FoundExistingOrder(order_id)) {
                inserted = orders_.insert({order_id, order}).second;
            }
            return inserted;
        }

        [[nodiscard]] auto RemoveOrder(const OrderId& order_id) -> bool {
            return orders_.erase(order_id) == 1;
        }

        [[nodiscard]] auto RemoveOrder(const OrderPtr& order) -> bool {
            return order && order->QuantityOnMarket() == 0 && RemoveOrder(order->GetOrderId());
        }

        OrderMap  orders_{};
        OrderBook book_{};
    };
}    // namespace akuna::me