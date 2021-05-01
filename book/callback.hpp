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
            CB_ORDER_REJECT,
            CB_ORDER_FILL,
            CB_ORDER_CANCEL,
            CB_ORDER_CANCEL_REJECT,
            CB_ORDER_REPLACE,
            CB_ORDER_REPLACE_REJECT,
            CB_BOOK_UPDATE,
            CB_SL_TRIGGERED
        };

        enum FillFlags { FF_NEITHER_FILLED = 0, FF_INBOUND_FILLED = 1, FF_MATCHED_FILLED = 2, FF_BOTH_FILLED = 4 };

        static auto Accept(const OrderPtr& order) -> Callback<OrderPtr>;

        static auto StopLossTriggered(const OrderId& order_id) -> Callback<OrderPtr>;

        static auto Reject(const OrderPtr& order, const char* reason) -> Callback<OrderPtr>;

        static auto Fill(const OrderPtr& inbound_order, const OrderPtr& matched_order, const Quantity& fill_qty,
                         const Price& fill_price) -> Callback<OrderPtr>;

        static auto Cancel(const OrderPtr& order, const Quantity& open_qty) -> Callback<OrderPtr>;

        static auto CancelReject(const OrderPtr& order, const char* reason) -> Callback<OrderPtr>;

        static auto Replace(const OrderPtr& passivated_order, const Quantity& open_qty, const OrderPtr& new_order) -> Callback<OrderPtr>;

        static auto ReplaceReject(const OrderPtr& order, const char* reason) -> Callback<OrderPtr>;

        static auto BookUpdate(const TypedOrderBook* book = nullptr) -> Callback<OrderPtr>;

        CbType      type_{CbType::CB_UNKNOWN};
        OrderPtr    order_{nullptr};
        OrderPtr    matched_order_{nullptr};
        Quantity    quantity_{0};
        Price       price_{0};
        int64_t     delta_{0};
        OrderId     order_id_{0};
        const char* reject_reason_{nullptr};
    };
}    // namespace akuna::book

#include "callback.inl"