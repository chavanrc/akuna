#include "market.hpp"

#include "logger.hpp"

namespace akuna::me {
    auto Market::AddBook(Symbol symbol) -> bool {
        LOG_INFO("Create new depth order book for " << symbol);
        auto [iter, inserted] = books_.insert_or_assign(symbol, std::make_shared<OrderBook>(symbol));
        return inserted;
    }

    auto Market::RemoveBook(Symbol symbol) -> bool {
        bool result = false;
        auto book   = books_.find(symbol);
        if (book != books_.end()) {
            auto order_id_list = book->second->AllOrderCancel();
            result             = books_.erase(symbol) == 1;
            for (const auto &order_id : order_id_list) {
                RemoveOrder(order_id);
            }
        }
        return result;
    }

    auto Market::Validate(const OrderPtr &order) -> bool {
        bool result = false;
        if (!order) {
            LOG_ERROR("Invalid order ref.");
            return result;
        }
        auto symbol = order->GetSymbol();
        auto book   = GetBook(symbol);
        if (!book) {
            LOG_ERROR("Symbol: " << symbol << "book not found.");
            return result;
        }
        return !result;
    }

    auto Market::OrderEntryValidate(const OrderPtr &order) -> bool {
        return Validate(order);
    }

    auto Market::OrderEntry(const OrderPtr &order, OrderConditions conditions) -> bool {
        if (!OrderEntryValidate(order)) {
            return false;
        }
        auto symbol   = order->GetSymbol();
        auto book     = GetBook(symbol);
        auto order_id = order->GetOrderId();
        LOG_INFO("ADDING order: " << *order);
        auto [iter, inserted] = orders_.insert_or_assign(order_id, order);
        if (inserted && book->Add(order, conditions)) {
            LOG_INFO(order_id << " matched");
            for (const auto &event : order->GetTrades()) {
                auto matched_order = GetOrder(event.matched_order_id_);
                if (matched_order && matched_order->QuantityOnMarket() == 0) {
                    if (matched_order->QuantityOnMarket() == 0) {
                        RemoveOrder(matched_order->GetOrderId());
                    }
                }
            }
            if (order->QuantityOnMarket() == 0) {
                RemoveOrder(order_id);
            }
        }
        return inserted;
    }

    auto Market::OrderModifyValidate(const OrderPtr &order) -> bool {
        bool result = false;
        if (!Validate(order)) {
            return result;
        }
        if (!FindExistingOrder(order->GetOrderId())) {
            return result;
        }
        return !result;
    }

    auto Market::OrderModify(const OrderPtr &order) -> bool {
        if (!OrderModifyValidate(order)) {
            return false;
        }
        auto passivated_order = GetOrder(order->GetOrderId());
        LOG_INFO("MODIFYING passivated order: " << *passivated_order << " with order: " << *order);
        auto book = GetBook(order->GetSymbol());
        return book->Replace(passivated_order, order);
    }

    auto Market::OrderCancel(OrderId order_id) -> bool {
        bool result = false;
        auto order  = GetOrder(order_id);
        if (order) {
            LOG_INFO("Requesting Cancel: " << *order);
            auto book = GetBook(order->GetSymbol());
            if (book) {
                book->Cancel(order);
                result = RemoveOrder(order_id);
            }
        }
        return result;
    }

    auto Market::RemoveOrder(OrderId order_id) -> bool {
        return orders_.erase(order_id) == 1;
    }

    auto Market::GetBook(Symbol symbol) -> OrderBookPtr {
        auto entry = books_.find(symbol);
        if (books_.find(symbol) != books_.end()) {
            return entry->second;
        }
        return {};
    }

    auto Market::GetOrder(OrderId order_id) -> OrderPtr {
        if (FindExistingOrder(order_id)) {
            return orders_.at(order_id);
        }
        return {};
    }

    auto Market::FindExistingOrder(OrderId order_id) -> bool {
        bool result = true;
        if (orders_.find(order_id) == orders_.end()) {
            LOG_DEBUG("--Can't find OrderID #" << order_id);
            result = false;
        }
        return result;
    }

    auto Market::Log() const -> void {
        for (const auto &[symbol, book] : books_) {
            book->Log();
        }
    }
}    // namespace akuna::me