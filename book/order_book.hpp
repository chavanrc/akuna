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
        using Tracker         = OrderTracker<OrderPtr>;
        using TypedCallback   = Callback<OrderPtr>;
        using TrackerMap      = std::multimap<ComparablePrice, Tracker>;
        using Callbacks       = std::vector<TypedCallback>;

        explicit OrderBook(Symbol symbol = 0);

        auto SetSymbol(Symbol symbol) -> void;

        [[nodiscard]] auto GetSymbol() const -> Symbol;

        [[nodiscard]] auto Add(const OrderPtr &order, OrderConditions conditions) -> bool;

        auto Cancel(const OrderPtr &order) -> void;

        auto Replace(const OrderPtr &passivated_order, const OrderPtr &new_order) -> bool;

        auto MarketPrice(Price price) -> void;

        [[nodiscard]] auto MarketPrice() const -> Price;

        auto GetBids() const -> const TrackerMap &;

        auto GetAsks() const -> const TrackerMap &;

        auto MatchOrder(Tracker &inbound, Price inbound_price, TrackerMap &current_orders) -> bool;

        auto MatchRegularOrder(Tracker &inbound, Price inbound_price, TrackerMap &current_orders) -> bool;

        auto CreateTrade(Tracker &inbound_tracker, Tracker &current_tracker, Quantity max_quantity = UINT64_MAX)
                -> Quantity;

        auto FindOnMarket(const OrderPtr &order, typename TrackerMap::iterator &result) -> bool;

        auto AllOrderCancel() -> std::vector<OrderId>;

        auto CallbackNow() -> void;

        auto PerformCallback(TypedCallback &cb) -> void;

        auto Log() const -> void;

    private:
        auto SubmitOrder(Tracker &inbound) -> bool;

        auto AddOrder(Tracker &inbound, Price order_price) -> bool;

        auto OnAccept(const OrderPtr &order, Quantity quantity) -> void;

        auto OnReject(const OrderPtr &order, const char *reason) -> void;

        auto OnFill(const OrderPtr &order, const OrderPtr &matched_order, Quantity fill_qty, Cost fill_cost,
                    FillId fill_id) -> void;

        auto OnCancel(const OrderPtr &order, Quantity quantity) -> void;

        auto OnCancelReject(const OrderPtr &order, const char *reason) -> void;

        auto OnReplace(const OrderPtr& order, Delta delta, Price new_price) -> void;

        auto OnReplaceReject(const OrderPtr &order, const char *reason) -> void;

        auto OnOrderBookChange() -> void;

        auto OnStopLossTriggered(const OrderId &order_id) -> void;

        auto OnTrade(const OrderBook *book, const OrderId &id_1, const OrderId &id_2, Quantity qty, Price price,
                     bool buyer_maker) -> void;

        Symbol     symbol_{0};
        TrackerMap bids_{};
        TrackerMap asks_{};
        Price      market_price_{MARKET_ORDER_PRICE};
        Callbacks  callbacks_{};
        Callbacks  working_callbacks_{};
        bool       handling_callbacks_{false};
    };
}    // namespace akuna::book

#include "order_book.inl"