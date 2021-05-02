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
        using Trades  = std::vector<OrderId>;

        Order(OrderId id, bool buy_side, Symbol symbol, Quantity quantity, Price price);

        [[nodiscard]] auto GetOrderId() const -> OrderId;

        [[nodiscard]] auto IsBuy() const -> bool;

        [[nodiscard]] auto GetSymbol() const -> Symbol;

        [[nodiscard]] auto GetPrice() const -> Price;

        [[nodiscard]] auto GetQuantity() const -> Quantity;

        [[nodiscard]] auto QuantityOnMarket() const -> Quantity;

        [[nodiscard]] auto QuantityFilled() const -> Quantity;

        [[nodiscard]] auto FillCost() const -> Cost;

        [[nodiscard]] auto GetTrades() const -> const Trades &;

        auto OnAccepted() -> void;

        auto OnFilled(Quantity fill_qty, Cost fill_cost) -> void;

        auto AddTradeHistory(const OrderId &matched_order_id) -> void;

        auto OnCancelled() -> void;

        auto OnReplaced(const Delta &size_delta, Price new_price) -> void;

        friend auto operator<<(std::ostream &os, const Order &order) -> std::ostream &;

    private:
        OrderId  id_{0};
        bool     buy_side_{};
        Symbol   symbol_{0};
        Quantity quantity_{0};
        Price    price_{0};
        Quantity quantity_filled_{0};
        Quantity quantity_on_market_{0};
        Cost     fill_cost_{0};
        Trades   trades_{};
    };
}    // namespace akuna::book