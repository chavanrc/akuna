#pragma once
#include "comparable_price.hpp"
#include "types.hpp"

namespace akuna::book {
    template <typename OrderPtr>
    class OrderTracker {
    public:
        explicit OrderTracker(const OrderPtr& order,
                              OrderConditions conditions = book::OrderCondition::OC_NO_CONDITIONS);

        auto Fill(Quantity qty) -> void;

        [[nodiscard]] auto Filled() const -> bool;

        [[nodiscard]] auto FilledQty() const -> Quantity;

        [[nodiscard]] auto OpenQty() const -> Quantity;

        auto Ptr() const -> const OrderPtr&;

        auto Ptr() -> OrderPtr&;

        [[nodiscard]] auto AllOrNone() const -> bool;

        [[nodiscard]] auto ImmediateOrCancel() const -> bool;

    private:
        OrderPtr        order_{nullptr};
        Quantity        open_qty_{0};
        OrderConditions conditions_;
    };
}    // namespace akuna::book

#include "order_tracker.inl"