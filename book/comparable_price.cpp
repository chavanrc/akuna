#include "comparable_price.hpp"

namespace akuna::book {
    ComparablePrice::ComparablePrice(bool buy_side, Price price) : price_(price), buy_side_(buy_side) {
    }

    auto ComparablePrice::Matches(Price rhs) const -> bool {
        if (price_ == rhs) {
            return true;
        }
        if (buy_side_) {
            return rhs < price_ || price_ == MARKET_ORDER_PRICE;
        }
        return price_ < rhs || rhs == MARKET_ORDER_PRICE;
    }

    auto ComparablePrice::operator<(Price rhs) const -> bool {
        if (price_ == MARKET_ORDER_PRICE) {
            return rhs != MARKET_ORDER_PRICE;
        } else if (rhs == MARKET_ORDER_PRICE) {
            return false;
        } else if (buy_side_) {
            return rhs < price_;
        } else {
            return price_ < rhs;
        }
    }

    auto ComparablePrice::operator==(Price rhs) const -> bool {
        return price_ == rhs;
    }

    auto ComparablePrice::operator!=(Price rhs) const -> bool {
        return price_ != rhs;
    }

    auto ComparablePrice::operator>(Price rhs) const -> bool {
        return price_ != MARKET_ORDER_PRICE &&
               ((rhs == MARKET_ORDER_PRICE) || (buy_side_ ? (rhs > price_) : (price_ > rhs)));
    }

    auto ComparablePrice::operator<=(Price rhs) const -> bool {
        return *this < rhs || *this == rhs;
    }

    auto ComparablePrice::operator>=(Price rhs) const -> bool {
        return *this > rhs || *this == rhs;
    }

    auto ComparablePrice::operator<(const ComparablePrice &rhs) const -> bool {
        return *this < rhs.price_;
    }

    auto ComparablePrice::operator==(const ComparablePrice &rhs) const -> bool {
        return *this == rhs.price_;
    }

    auto ComparablePrice::operator!=(const ComparablePrice &rhs) const -> bool {
        return *this != rhs.price_;
    }

    auto ComparablePrice::operator>(const ComparablePrice &rhs) const -> bool {
        return *this > rhs.price_;
    }

    auto ComparablePrice::GetPrice() const -> Price {
        return price_;
    }

    auto ComparablePrice::IsBuy() const -> bool {
        return buy_side_;
    }

    auto ComparablePrice::IsMarket() const -> bool {
        return price_ == MARKET_ORDER_PRICE;
    }

    auto operator<<(std::ostream &os, const ComparablePrice &price) -> std::ostream & {
        os << "at ";
        if (price.IsMarket()) {
            os << "Market";
        } else {
            os << price.GetPrice();
        }
        return os;
    }
}    // namespace akuna::book
