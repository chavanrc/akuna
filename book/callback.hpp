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

        static auto Accept(const OrderPtr& order) -> Callback<OrderPtr>;

        static auto Fill(const OrderPtr& inbound_order, const OrderPtr& matched_order, const Quantity& fill_qty,
                         const Price& fill_price) -> Callback<OrderPtr>;

        static auto Cancel(const OrderPtr& order, const Quantity& open_qty) -> Callback<OrderPtr>;

        static auto Replace(const OrderPtr& passivated_order, const Quantity& open_qty, const OrderPtr& new_order)
                -> Callback<OrderPtr>;

        CbType      type_{CbType::CB_UNKNOWN};
        OrderPtr    order_{nullptr};
        OrderPtr    matched_order_{nullptr};
        Quantity    quantity_{0};
        Price       price_{0};
        Delta       delta_{0};
    };
}    // namespace akuna::book

#include "callback.inl"