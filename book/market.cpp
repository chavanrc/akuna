#include "market.hpp"

#include "logger.hpp"

namespace akuna::me {
    auto Market::AddBook(Symbol symbol) -> bool {
        LOG_DEBUG("Create new depth order book for " << symbol);
        bool inserted = false;
        if (books_.find(symbol) == books_.end()) {
            inserted = books_.insert({symbol, std::make_shared<OrderBook>(symbol)}).second;
        } else {
            books_.at(symbol) = std::make_shared<OrderBook>(symbol);
        }
        return inserted;
    }

    auto Market::Validate(const OrderPtr &order) -> bool {
        bool result = false;
        if (!order) {
            LOG_ERROR("Invalid order ref.");
            return result;
        }
        if(order->GetPrice() == 0) {
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
        LOG_DEBUG("ADDING order: " << *order);
        auto symbol   = order->GetSymbol();
        auto book     = GetBook(symbol);
        auto order_id = order->GetOrderId();
        bool inserted = AddOrder(order);

        if (inserted && book->Add(order, conditions)) {
            LOG_DEBUG(order_id << " matched");
            for (const auto &matched_order_id : order->GetTrades()) {
                auto matched_order = GetOrder(matched_order_id);
                if (matched_order && matched_order->QuantityOnMarket() == 0 &&
                    RemoveOrder(matched_order->GetOrderId())) {
                    LOG_DEBUG("REMOVED order: " << *matched_order);
                }
            }
            if (order->QuantityOnMarket() == 0) {
                if (RemoveOrder(order_id)) {
                    LOG_DEBUG("REMOVED order: " << *order);
                }
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
        auto order_id         = order->GetOrderId();
        auto passivated_order = GetOrder(order_id);
        auto book             = GetBook(order->GetSymbol());
        LOG_DEBUG("MODIFYING passivated order: " << *passivated_order << " with order: " << *order);
        if (book->Replace(passivated_order, order)) {
            for (const auto &matched_order_id : order->GetTrades()) {
                auto matched_order = GetOrder(matched_order_id);
                if (RemoveOrder(matched_order)) {
                    LOG_DEBUG("REMOVED order: " << *matched_order);
                }
            }
            if (RemoveOrder(order)) {
                LOG_DEBUG("REMOVED order: " << *order);
            }
        }
        if (FindExistingOrder(order_id)) {
            orders_.at(order_id) = order;
        }
        return true;
    }

    auto Market::OrderCancel(const OrderId &order_id) -> bool {
        bool result = false;
        auto order  = GetOrder(order_id);
        if (order) {
            LOG_DEBUG("Requesting Cancel: " << *order);
            auto book = GetBook(order->GetSymbol());
            if (book) {
                book->Cancel(order);
                result = RemoveOrder(order_id);
            }
        }
        return result;
    }

    auto Market::AddOrder(const OrderPtr &order) -> bool {
        const auto &order_id = order->GetOrderId();
        // auto [iter, inserted] = orders_.insert_or_assign(order_id, order);
        bool inserted = false;
        if (orders_.find(order_id) == orders_.end()) {
            inserted = orders_.insert({order_id, order}).second;
        }
        return inserted;
    }

    auto Market::RemoveOrder(const OrderId &order_id) -> bool {
        return orders_.erase(order_id) == 1;
    }

    auto Market::RemoveOrder(const OrderPtr &order) -> bool {
        bool result = false;
        if (order->QuantityOnMarket() == 0) {
            result = RemoveOrder(order->GetOrderId());
        }
        return result;
    }

    auto Market::GetBook(Symbol symbol) -> OrderBookPtr {
        auto entry = books_.find(symbol);
        if (books_.find(symbol) != books_.end()) {
            return entry->second;
        }
        return {};
    }

    auto Market::GetOrder(const OrderId &order_id) -> OrderPtr {
        if (FindExistingOrder(order_id)) {
            return orders_.at(order_id);
        }
        return {};
    }

    auto Market::FindExistingOrder(const OrderId &order_id) -> bool {
        bool result = true;
        if (orders_.find(order_id) == orders_.end()) {
            LOG_DEBUG("--Can't find OrderID #" << order_id);
            result = false;
        }
        return result;
    }

    auto Market::Log() const -> void {
        // for (const auto &[symbol, book] : books_) {
        for (const auto &book : books_) {
            book.second->Log();
        }
    }
}    // namespace akuna::me