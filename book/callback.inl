namespace akuna::book {
    template <class OrderPtr>
    auto Callback<OrderPtr>::Accept(const OrderPtr& order) -> Callback<OrderPtr> {
        Callback<OrderPtr> result;
        result.type_  = CbType::CB_ORDER_ACCEPT;
        result.order_ = order;
        return result;
    }

    template <class OrderPtr>
    auto Callback<OrderPtr>::Reject(const OrderPtr& order, const char* reason) -> Callback<OrderPtr> {
        Callback<OrderPtr> result;
        result.type_          = CbType::CB_ORDER_REJECT;
        result.order_         = order;
        result.reject_reason_ = reason;
        return result;
    }

    template <class OrderPtr>
    auto Callback<OrderPtr>::Fill(const OrderPtr& inbound_order, const OrderPtr& matched_order,
                                  const Quantity& fill_qty, const Price& fill_price) -> Callback<OrderPtr> {
        Callback<OrderPtr> result;
        result.type_          = CbType::CB_ORDER_FILL;
        result.order_         = inbound_order;
        result.matched_order_ = matched_order;
        result.quantity_      = fill_qty;
        result.price_         = fill_price;
        return result;
    }

    template <class OrderPtr>
    auto Callback<OrderPtr>::Cancel(const OrderPtr& order, const Quantity& open_qty) -> Callback<OrderPtr> {
        Callback<OrderPtr> result;
        result.type_     = CbType::CB_ORDER_CANCEL;
        result.order_    = order;
        result.quantity_ = open_qty;
        return result;
    }

    template <class OrderPtr>
    auto Callback<OrderPtr>::CancelReject(const OrderPtr& order, const char* reason) -> Callback<OrderPtr> {
        Callback<OrderPtr> result;
        result.type_          = CbType::CB_ORDER_CANCEL_REJECT;
        result.order_         = order;
        result.reject_reason_ = reason;
        return result;
    }

    template <class OrderPtr>
    auto Callback<OrderPtr>::Replace(const OrderPtr& passivated_order, const Quantity& open_qty,
                                     const OrderPtr& new_order) -> Callback<OrderPtr> {
        Callback<OrderPtr> result;
        result.type_     = CbType::CB_ORDER_REPLACE;
        result.order_    = passivated_order;
        result.quantity_ = open_qty;
        result.delta_    = new_order->GetQuantity() - passivated_order->GetQuantity();
        result.price_    = new_order->GetPrice();
        return result;
    }

    template <class OrderPtr>
    auto Callback<OrderPtr>::ReplaceReject(const OrderPtr& order, const char* reason) -> Callback<OrderPtr> {
        Callback<OrderPtr> result;
        result.type_          = CbType::CB_ORDER_REPLACE_REJECT;
        result.order_         = order;
        result.reject_reason_ = reason;
        return result;
    }

    template <class OrderPtr>
    auto Callback<OrderPtr>::BookUpdate(const OrderBook<OrderPtr>* book) -> Callback<OrderPtr> {
        Callback<OrderPtr> result;
        result.type_ = CbType::CB_BOOK_UPDATE;
        return result;
    }
}    // namespace akuna::book