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

        explicit OrderBook(Symbol symbol = 0);

        [[nodiscard]] auto Add(const OrderPtr &order, OrderConditions conditions) -> bool;

        auto Cancel(const OrderPtr &order) -> void;

        [[nodiscard]] auto Replace(const OrderPtr &passivated_order, const OrderPtr &new_order) -> bool;

        auto MarketPrice(Price price) -> void;

        auto MatchOrder(Tracker &inbound, Price inbound_price, TrackerMap &current_orders) -> bool;

        auto MatchRegularOrder(Tracker &inbound, Price inbound_price, TrackerMap &current_orders) -> bool;

        auto CreateTrade(Tracker &inbound_tracker, Tracker &current_tracker, Quantity max_quantity = UINT64_MAX)
                -> Quantity;

        [[nodiscard]] auto FindOnMarket(const OrderPtr &order, typename TrackerMap::iterator &result) -> bool;

        auto CallbackNow() -> void;

        auto PerformCallback(TypedCallback &cb) -> void;

        auto Log() const -> void;

    private:
        auto SubmitOrder(Tracker &inbound) -> bool;

        auto AddOrder(Tracker &inbound, Price order_price) -> bool;

        auto OnAccept(const OrderPtr &order, Quantity quantity) -> void;

        auto OnFill(const OrderPtr &order, const OrderPtr &matched_order, Quantity fill_qty, Cost fill_cost,
                    FillId fill_id) -> void;

        auto OnCancel(const OrderPtr &order, Quantity quantity) -> void;

        auto OnReplace(const OrderPtr &order, Delta delta, Price new_price) -> void;

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