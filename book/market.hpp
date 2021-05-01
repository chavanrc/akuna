#pragma once
#include <memory>
#include <unordered_map>

#include "order.hpp"
#include "order_book.hpp"

namespace akuna::me {
    class Market {
    public:
        using OrderId         = book::OrderId;
        using Symbol          = book::Symbol;
        using OrderConditions = book::OrderConditions;
        using OrderPtr        = std::shared_ptr<book::Order>;
        using OrderBook       = book::OrderBook<OrderPtr>;
        using OrderBookPtr    = std::shared_ptr<OrderBook>;
        using OrderMap        = std::unordered_map<OrderId, OrderPtr>;
        using SymbolToBookMap = std::unordered_map<Symbol, OrderBookPtr>;

        auto AddBook(Symbol symbol) -> bool;

        auto OrderEntry(const OrderPtr& order, OrderConditions conditions = book::OrderCondition::OC_NO_CONDITIONS) -> bool;

        auto OrderModify(const OrderPtr& order) -> bool;

        auto OrderCancel(OrderId order_id) -> bool;

        auto Log() const -> void;

    private:
        auto GetBook(Symbol symbol) -> OrderBookPtr;

        auto GetOrder(OrderId order_id) -> OrderPtr;

        auto FindExistingOrder(OrderId order_id) -> bool;

        auto Validate(const OrderPtr& order) -> bool;

        auto OrderEntryValidate(const OrderPtr& order) -> bool;

        auto OrderModifyValidate(const OrderPtr& order) -> bool;

        auto RemoveOrder(OrderId order_id) -> bool;

        OrderMap        orders_{};
        SymbolToBookMap books_{};
    };
}    // namespace akuna::me