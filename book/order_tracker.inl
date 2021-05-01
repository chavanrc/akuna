namespace akuna::book {
    template <typename OrderPtr>
    OrderTracker<OrderPtr>::OrderTracker(const OrderPtr& order, OrderConditions conditions)
        : order_{order}, open_qty_{order->GetQuantity()}, conditions_{conditions} {
    }

    template <typename OrderPtr>
    auto OrderTracker<OrderPtr>::Fill(Quantity qty) -> void {
        if (qty > open_qty_) {
            throw std::runtime_error("Fill size larger than open quantity");
        }
        open_qty_ -= qty;
    }

    template <typename OrderPtr>
    auto OrderTracker<OrderPtr>::Filled() const -> bool {
        return open_qty_ == 0;
    }

    template <typename OrderPtr>
    auto OrderTracker<OrderPtr>::FilledQty() const -> Quantity {
        return order_->GetQuantity() - OpenQty();
    }

    template <typename OrderPtr>
    auto OrderTracker<OrderPtr>::OpenQty() const -> Quantity {
        return open_qty_;
    }

    template <typename OrderPtr>
    auto OrderTracker<OrderPtr>::Ptr() const -> const OrderPtr& {
        return order_;
    }

    template <typename OrderPtr>
    auto OrderTracker<OrderPtr>::Ptr() -> OrderPtr& {
        return order_;
    }

    template <typename OrderPtr>
    bool OrderTracker<OrderPtr>::AllOrNone() const {
        return static_cast<OrderCondition>(conditions_) == OrderCondition::OC_ALL_OR_NONE;
    }

    template <typename OrderPtr>
    bool OrderTracker<OrderPtr>::ImmediateOrCancel() const {
        return static_cast<OrderCondition>(conditions_) == OrderCondition::OC_IMMEDIATE_OR_CANCEL;
    }
}    // namespace akuna::book