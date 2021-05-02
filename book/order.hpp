#pragma once

#include <optional>
#include <ostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "types.hpp"

namespace akuna::book {
    enum class State {
        REJECTED,
        ACCEPTED,
        MODIFY_REJECTED,
        MODIFIED,
        PARTIAL_FILLED,
        FILLED,
        CANCEL_REJECTED,
        CANCELLED,
        UNKNOWN
    };

    struct StateChange {
        State       state_{State::UNKNOWN};
        std::string description_{};

        explicit StateChange(State state, std::string description = "")
            : state_(state), description_(std::move(description)) {
        }

        friend auto operator<<(std::ostream &os, const StateChange &change) -> std::ostream & {
            os << "State: " << static_cast<int32_t>(change.state_) << " Description: " << change.description_;
            return os;
        }
    };

    struct MatchedTrade {
        OrderId  matched_order_id_{0};
        Cost     fill_cost_{0};
        Quantity quantity_{0};
        Quantity quantity_on_market_{0};
        Price    price_{0};
        FillId   fill_id_{0};
    };

    class Order {
    public:
        using History = std::vector<StateChange>;
        using Trades  = std::vector<MatchedTrade>;

        Order(OrderId id, bool buy_side, Symbol symbol, Quantity quantity, Price price);

        [[nodiscard]] auto GetOrderId() const -> OrderId;

        [[nodiscard]] auto IsBuy() const -> bool;

        [[nodiscard]] auto GetSymbol() const -> Symbol;

        [[nodiscard]] auto GetPrice() const -> Price;

        [[nodiscard]] auto GetQuantity() const -> Quantity;

        [[nodiscard]] auto QuantityOnMarket() const -> Quantity;

        [[nodiscard]] auto QuantityFilled() const -> Quantity;

        [[nodiscard]] auto FillCost() const -> Cost;

        [[nodiscard]] auto GetHistory() const -> const History &;

        [[nodiscard]] auto GetTrades() const -> const Trades &;

        auto OnAccepted() -> void;

        auto OnRejected(const char *reason) -> void;

        auto OnFilled(Quantity fill_qty, Cost fill_cost) -> void;

        auto AddTradeHistory(Quantity fill_qty, Quantity remaining_qty, Cost fill_cost, const OrderId &matched_order_id,
                             Price price, FillId fill_id) -> void;

        auto OnCancelled() -> void;

        auto OnCancelRejected(const char *reason) -> void;

        auto OnReplaced(const Delta &size_delta, Price new_price) -> void;

        auto OnReplaceRejected(const char *reason) -> void;

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
        History  history_{};
        Trades   trades_{};
    };
}    // namespace akuna::book