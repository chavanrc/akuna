#pragma once

#include <cstdint>

namespace akuna::book {
    using Price           = std::size_t;
    using Quantity        = std::size_t;
    using Cost            = std::size_t;
    using FillId          = std::size_t;
    using ChangeId        = std::size_t;
    using OrderId         = std::string;
    using Symbol          = std::size_t;
    using Delta           = int64_t;
    using OrderConditions = size_t;

    enum OrderCondition {
        OC_NO_CONDITIONS       = 0,
        OC_ALL_OR_NONE         = 1,
        OC_IMMEDIATE_OR_CANCEL = OC_ALL_OR_NONE << 1,
    };

    namespace {
        constexpr Price   MARKET_ORDER_PRICE(0);
        constexpr Price   PRICE_UNCHANGED(0);
        constexpr int64_t SIZE_UNCHANGED(0);
        constexpr Price   INVALID_LEVEL_PRICE(0);
        constexpr Price   MARKET_ORDER_BID_SORT_PRICE(UINT32_MAX);
        constexpr Price   MARKET_ORDER_ASK_SORT_PRICE(0);
        constexpr Symbol  DEFAULT_SYMBOL{1};
    }    // namespace

    static const int32_t BOOK_DEPTH = 10;
}    // namespace akuna::book