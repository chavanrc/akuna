#pragma once

#include <iostream>

#include "types.hpp"

namespace akuna::book {
    class ComparablePrice {
    public:
        ComparablePrice(bool buy_side, Price price) : price_(price), buy_side_(buy_side) {
        }

        [[nodiscard]] auto Matches(Price rhs) const -> bool {
            if (price_ == rhs) {
                return true;
            }
            if (buy_side_) {
                return rhs < price_ || price_ == MARKET_ORDER_PRICE;
            }
            return price_ < rhs || rhs == MARKET_ORDER_PRICE;
        }

        auto operator<(Price rhs) const -> bool {
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

        auto operator==(Price rhs) const -> bool {
            return price_ == rhs;
        }

        auto operator!=(Price rhs) const -> bool {
            return price_ != rhs;
        }

        auto operator>(Price rhs) const -> bool {
            return price_ != MARKET_ORDER_PRICE &&
                   ((rhs == MARKET_ORDER_PRICE) || (buy_side_ ? (rhs > price_) : (price_ > rhs)));
        }

        auto operator<=(Price rhs) const -> bool {
            return *this < rhs || *this == rhs;
        }

        auto operator>=(Price rhs) const -> bool {
            return *this > rhs || *this == rhs;
        }

        auto operator<(const ComparablePrice& rhs) const -> bool {
            return *this < rhs.price_;
        }

        auto operator==(const ComparablePrice& rhs) const -> bool {
            return *this == rhs.price_;
        }

        auto operator!=(const ComparablePrice& rhs) const -> bool {
            return *this != rhs.price_;
        }

        auto operator>(const ComparablePrice& rhs) const -> bool {
            return *this > rhs.price_;
        }

        [[nodiscard]] auto GetPrice() const -> Price {
            return price_;
        }

        [[nodiscard]] auto IsBuy() const -> bool {
            return buy_side_;
        }

        [[nodiscard]] auto IsMarket() const -> bool {
            return price_ == MARKET_ORDER_PRICE;
        }

        friend auto operator<<(std::ostream& os, const ComparablePrice& price) -> std::ostream& {
            os << "at ";
            if (price.IsMarket()) {
                os << "Market";
            } else {
                os << price.GetPrice();
            }
            return os;
        }

    private:
        Price price_;
        bool  buy_side_;
    };

    inline auto operator<(Price price, const ComparablePrice& key) -> bool {
        return key > price;
    }

    inline auto operator>(Price price, const ComparablePrice& key) -> bool {
        return key < price;
    }

    inline auto operator==(Price price, const ComparablePrice& key) -> bool {
        return key == price;
    }

    inline auto operator!=(Price price, const ComparablePrice& key) -> bool {
        return key != price;
    }

    inline auto operator<=(Price price, const ComparablePrice& key) -> bool {
        return key >= price;
    }

    inline auto operator>=(Price price, const ComparablePrice& key) -> bool {
        return key <= price;
    }
}    // namespace akuna::book
