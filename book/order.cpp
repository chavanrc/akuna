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

    auto Order::GetHistory() const -> const History & {
        return history_;
    }

    auto Order::GetTrades() const -> const Trades & {
        return trades_;
    }

    auto Order::OnAccepted() -> void {
        quantity_on_market_ = quantity_;
        history_.emplace_back(State::ACCEPTED);
    }

    auto Order::OnRejected(const char *reason) -> void {
        history_.emplace_back(State::REJECTED, reason);
    }

    auto Order::OnFilled(Quantity fill_qty, Cost fill_cost) -> void {
        quantity_on_market_ -= fill_qty;
        fill_cost_ += fill_cost;
        quantity_filled_ += fill_qty;

        std::stringstream msg;
        msg << fill_qty << " for " << fill_cost;
        history_.emplace_back(State::FILLED, msg.str());
    }

    auto Order::AddTradeHistory(Quantity fill_qty, Quantity remaining_qty, Cost fill_cost,
                                const OrderId &matched_order_id, Price price, FillId fill_id) -> void {
        MatchedTrade res{};
        res.matched_order_id_   = matched_order_id;
        res.fill_cost_          = fill_cost;
        res.quantity_           = fill_qty;
        res.price_              = price;
        res.quantity_on_market_ = remaining_qty;
        res.fill_id_            = fill_id;
        trades_.emplace_back(res);
    }

    auto Order::OnCancelled() -> void {
        quantity_on_market_ = 0;
        history_.emplace_back(State::CANCELLED);
    }

    auto Order::OnCancelRejected(const char *reason) -> void {
        history_.emplace_back(State::CANCEL_REJECTED, reason);
    }

    auto Order::OnReplaced(const Delta &size_delta, Price new_price) -> void {
        std::stringstream msg;
        if (size_delta != akuna::book::SIZE_UNCHANGED) {
            quantity_ += size_delta;
            quantity_on_market_ += size_delta;
            msg << "Quantity change: " << size_delta << ' ';
        }
        if (new_price != akuna::book::PRICE_UNCHANGED) {
            price_ = new_price;
            msg << "New Price " << new_price;
        }
        history_.emplace_back(State::MODIFIED, msg.str());
    }

    auto Order::OnReplaceRejected(const char *reason) -> void {
        history_.emplace_back(State::MODIFY_REJECTED, reason);
    }

    std::ostream &operator<<(std::ostream &os, const Order &order) {
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

        const auto &history = order.GetHistory();
        for (const auto &event : history) {
            os << "\n\t" << event;
        }

        os << ']';

        return os;
    }
}    // namespace akuna::book