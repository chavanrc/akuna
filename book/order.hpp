#pragma once

#include <optional>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

#include "types.hpp"

namespace akuna::book {
    enum class State {
        SUBMITTED,
        REJECTED,
        ACCEPTED,
        MODIFY_REQUESTED,
        MODIFY_REJECTED,
        MODIFIED,
        PARTIAL_FILLED,
        FILLED,
        CANCEL_REQUESTED,
        CANCEL_REJECTED,
        CANCELLED,
        UNKNOWN
    };

    enum TickType : char { ORDER_TICK = 'O', TRADE_EVENT_TICK = 'T', BOOK_UPDATE = 'B', BOOK_CHANGE = 'C' };

    struct StateChange {
        State       state_{State::UNKNOWN};
        std::string description_{};

        explicit StateChange(State state, const std::string &description = "")
            : state_(state), description_(description) {
        }

        friend std::ostream &operator<<(std::ostream &os, const StateChange &change) {
            os /*<< "State: " << change.state_*/ << " Description: " << change.description_;
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

        [[nodiscard]] auto IsLimit() const -> bool;

        [[nodiscard]] auto IsBuy() const -> bool;

        [[nodiscard]] auto GetSymbol() const -> Symbol;

        [[nodiscard]] auto GetPrice() const -> Price;

        [[nodiscard]] auto GetQuantity() const -> Quantity;

        [[nodiscard]] auto QuantityOnMarket() const -> Quantity;

        [[nodiscard]] auto QuantityFilled() const -> Quantity;

        [[nodiscard]] auto FillCost() const -> Cost;

        [[nodiscard]] auto GetHistory() const -> const History &;

        [[nodiscard]] auto GetTrades() const -> const Trades &;

        [[nodiscard]] auto CurrentState() const -> std::optional<StateChange>;

        auto OnSubmitted() -> void;

        auto OnAccepted() -> void;

        auto OnRejected(const char *reason) -> void;

        auto OnFilled(Quantity fill_qty, Cost fill_cost) -> void;

        auto AddTradeHistory(Quantity fill_qty, Quantity remaining_qty, Cost fill_cost, const OrderId &matched_order_id,
                             Price price, FillId fill_id) -> void;

        auto OnCancelRequested() -> void;

        auto OnCancelled() -> void;

        auto OnCancelRejected(const char *reason) -> void;

        auto OnReplaceRequested(const int64_t &size_delta, Price new_price) -> void;

        auto OnReplaced(const int64_t &size_delta, Price new_price) -> void;

        auto OnReplaceRejected(const char *reason) -> void;

        friend std::ostream &operator<<(std::ostream &os, const Order &order);

        [[nodiscard]] auto IsVerbose() const -> bool;

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
        bool     verbose_{false};
    };
}    // namespace akuna::book