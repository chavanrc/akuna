#include "order.hpp"

namespace akuna::book {
    Order::Order(OrderId id, bool buy_side, Symbol symbol, Quantity quantity, Price price)
        : id_{std::move(id)}, buy_side_{buy_side}, symbol_{symbol}, quantity_{quantity}, price_{price} {
    }

    auto Order::GetOrderId() const -> OrderId {
        return id_;
    }

    auto Order::IsBuy() const -> bool {
        return buy_side_;
    }

    auto Order::GetSymbol() const -> Symbol {
        return symbol_;
    }

    auto Order::GetPrice() const -> Price {
        return price_;
    }

    auto Order::GetQuantity() const -> Quantity {
        return quantity_;
    }

    auto Order::QuantityOnMarket() const -> Quantity {
        return quantity_on_market_;
    }

    auto Order::QuantityFilled() const -> Quantity {
        return quantity_filled_;
    }

    auto Order::FillCost() const -> Cost {
        return fill_cost_;
    }

    auto Order::GetTrades() const -> const Trades & {
        return trades_;
    }

    auto Order::OnAccepted() -> void {
        quantity_on_market_ = quantity_;
    }

    auto Order::OnFilled(Quantity fill_qty, Cost fill_cost) -> void {
        quantity_on_market_ -= fill_qty;
        fill_cost_ += fill_cost;
        quantity_filled_ += fill_qty;
    }

    auto Order::AddTradeHistory(const OrderId &matched_order_id) -> void {
        trades_.emplace_back(matched_order_id);
    }

    auto Order::OnCancelled() -> void {
        quantity_on_market_ = 0;
    }

    auto Order::OnReplaced(const Delta &size_delta, Price new_price) -> void {
        if (size_delta != akuna::book::SIZE_UNCHANGED) {
            quantity_ += size_delta;
            quantity_on_market_ += size_delta;
        }
        if (new_price != akuna::book::PRICE_UNCHANGED) {
            price_ = new_price;
        }
    }

    auto operator<<(std::ostream &os, const Order &order) -> std::ostream & {
        os << "[#" << order.GetOrderId();
        os << ' ' << (order.IsBuy() ? "BUY" : "SELL");
        os << ' ' << order.GetSymbol();
        os << ' ' << order.GetQuantity();
        if (order.GetPrice() == 0) {
            os << " MKT";
        } else {
            os << " $" << order.GetPrice();
        }

        auto on_market = order.QuantityOnMarket();
        if (on_market != 0) {
            os << " Open: " << on_market;
        }

        auto filled = order.QuantityFilled();
        if (filled != 0) {
            os << " FILLED: " << filled;
        }

        auto cost = order.FillCost();
        if (cost != 0) {
            os << " Cost: " << cost;
        }

        os << ']';

        return os;
    }
}    // namespace akuna::book