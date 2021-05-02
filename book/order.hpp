#pragma once

#include <optional>
#include <ostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "types.hpp"

namespace akuna::book {
    class Order {
    public:
        using Trades = std::vector<OrderId>;

        Order(OrderId id, bool buy_side, Quantity quantity, Price price)
            : id_{std::move(id)}, buy_side_{buy_side}, quantity_{quantity}, price_{price} {
            trades_.reserve(8);
        }

        [[nodiscard]] auto GetOrderId() const -> OrderId {
            return id_;
        }

        [[nodiscard]] auto IsBuy() const -> bool {
            return buy_side_;
        }

        [[nodiscard]] auto GetSymbol() const -> Symbol {
            return symbol_;
        }

        [[nodiscard]] auto GetPrice() const -> Price {
            return price_;
        }

        [[nodiscard]] auto GetQuantity() const -> Quantity {
            return quantity_;
        }

        [[nodiscard]] auto QuantityOnMarket() const -> Quantity {
            return quantity_on_market_;
        }

        [[nodiscard]] auto QuantityFilled() const -> Quantity {
            return quantity_filled_;
        }

        [[nodiscard]] auto GetTrades() const -> const Trades & {
            return trades_;
        }

        auto OnAccepted() -> void {
            quantity_on_market_ = quantity_;
        }

        auto OnFilled(Quantity fill_qty) -> void {
            quantity_on_market_ -= fill_qty;
            quantity_filled_ += fill_qty;
        }

        auto AddTradeHistory(const OrderId &matched_order_id) -> void {
            trades_.emplace_back(matched_order_id);
        }

        auto OnCancelled() -> void {
            quantity_on_market_ = 0;
        }

        auto OnReplaced(const Delta &size_delta, Price new_price) -> void {
            quantity_ += size_delta;
            quantity_on_market_ += size_delta;
            price_ = new_price;
        }

        friend auto operator<<(std::ostream &os, const Order &order) -> std::ostream & {
            os << "[#" << order.GetOrderId();
            os << ' ' << (order.IsBuy() ? "BUY" : "SELL");
            os << ' ' << order.GetSymbol();
            os << ' ' << order.GetQuantity();
            if (order.GetPrice() == 0) {
                os << " MKT";
            } else {
                os << " $" << order.GetPrice();
            }

            auto on_market = order.QuantityOnMarket();
            if (on_market != 0) {
                os << " Open: " << on_market;
            }

            auto filled = order.QuantityFilled();
            if (filled != 0) {
                os << " FILLED: " << filled;
            }

            os << ']';

            return os;
        }

    private:
        OrderId  id_{0};
        bool     buy_side_{};
        Symbol   symbol_{DEFAULT_SYMBOL};
        Quantity quantity_{0};
        Price    price_{0};
        Quantity quantity_filled_{0};
        Quantity quantity_on_market_{0};
        Trades   trades_{};
    };
}    // namespace akuna::book