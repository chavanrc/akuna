#pragma once
#include "comparable_price.hpp"
#include "types.hpp"

namespace akuna::book {
    template <typename OrderPtr>
    class OrderTracker {
    public:
        explicit OrderTracker(const OrderPtr& order,
                              OrderConditions conditions = book::OrderCondition::OC_NO_CONDITIONS)
            : order_{order}, open_qty_{order->GetQuantity()}, conditions_{conditions} {
        }

        auto Fill(Quantity qty) -> void {
            if (qty > open_qty_) {
                throw std::runtime_error("Fill size larger than open quantity");
            }
            open_qty_ -= qty;
        }

        [[nodiscard]] auto Filled() const -> bool {
            return open_qty_ == 0;
        }

        [[nodiscard]] auto FilledQty() const -> Quantity {
            return order_->GetQuantity() - OpenQty();
        }

        [[nodiscard]] auto OpenQty() const -> Quantity {
            return open_qty_;
        }

        auto Ptr() const -> const OrderPtr& {
            return order_;
        }

        auto Ptr() -> OrderPtr& {
            return order_;
        }

        [[nodiscard]] auto ImmediateOrCancel() const -> bool {
            return static_cast<OrderCondition>(conditions_) == OrderCondition::OC_IMMEDIATE_OR_CANCEL;
        }

    private:
        OrderPtr        order_{nullptr};
        Quantity        open_qty_{0};
        OrderConditions conditions_;
    };
}    // namespace akuna::book