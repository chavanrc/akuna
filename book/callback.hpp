#pragma once
#include <iostream>
#include <type_traits>

#include "types.hpp"

namespace akuna::book {
    template <class OrderPtr>
    class OrderBook;

    template <typename T>
    std::ostream& operator<<(typename std::enable_if<std::is_enum<T>::value, std::ostream>::type& stream, const T& e) {
        return stream << static_cast<typename std::underlying_type<T>::type>(e);
    }

    template <typename OrderPtr>
    class Callback {
    public:
        using TypedOrderBook = OrderBook<OrderPtr>;

        enum class CbType : int16_t {
            CB_UNKNOWN,
            CB_ORDER_ACCEPT,
            CB_ORDER_FILL,
            CB_ORDER_CANCEL,
            CB_ORDER_REPLACE
        };

        static auto Accept(const OrderPtr& order) -> Callback<OrderPtr> {
            Callback<OrderPtr> result;
            result.type_  = CbType::CB_ORDER_ACCEPT;
            result.order_ = order;
            return result;
        }

        static auto Fill(const OrderPtr& inbound_order, const OrderPtr& matched_order, const Quantity& fill_qty,
                         const Price& fill_price) -> Callback<OrderPtr> {
            Callback<OrderPtr> result;
            result.type_          = CbType::CB_ORDER_FILL;
            result.order_         = inbound_order;
            result.matched_order_ = matched_order;
            result.quantity_      = fill_qty;
            result.price_         = fill_price;
            return result;
        }

        static auto Cancel(const OrderPtr& order, const Quantity& open_qty) -> Callback<OrderPtr> {
            Callback<OrderPtr> result;
            result.type_     = CbType::CB_ORDER_CANCEL;
            result.order_    = order;
            result.quantity_ = open_qty;
            return result;
        }

        static auto Replace(const OrderPtr& passivated_order, const Quantity& open_qty, const OrderPtr& new_order)
                -> Callback<OrderPtr> {
            Callback<OrderPtr> result;
            result.type_     = CbType::CB_ORDER_REPLACE;
            result.order_    = passivated_order;
            result.quantity_ = open_qty;
            result.delta_    = new_order->GetQuantity() - passivated_order->GetQuantity();
            result.price_    = new_order->GetPrice();
            return result;
        }

        CbType      type_{CbType::CB_UNKNOWN};
        OrderPtr    order_{nullptr};
        OrderPtr    matched_order_{nullptr};
        Quantity    quantity_{0};
        Price       price_{0};
        Delta       delta_{0};
    };
}    // namespace akuna::book