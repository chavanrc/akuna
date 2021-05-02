#include <fstream>
#include <ostream>

#include "book/market.hpp"

struct Order {
    char                  msg_type_{'\0'};
    akuna::book::OrderId  order_id_{0};
    akuna::book::Symbol   symbol_{akuna::book::DEFAULT_SYMBOL};
    bool                  is_buy_{false};
    bool                  aon_{false};
    bool                  ioc_{false};
    akuna::book::Quantity quantity_{0};
    akuna::book::Price    price_{0};

    friend std::ostream& operator<<(std::ostream& os, const Order& order) {
        os << "msg_type : " << order.msg_type_ << " order_id : " << order.order_id_ << " symbol : " << order.symbol_
           << " is_buy : " << order.is_buy_ << " aon : " << order.aon_ << " ioc : " << order.ioc_
           << " quantity : " << order.quantity_ << " price : " << order.price_;
        return os;
    }
};

static void Trim(std::string& line) {
    if (!line.empty() && line[line.size() - 1] == '\r')
        line.erase(line.size() - 1);
}

static std::pair<bool, Order> ReadLine(const std::string& line) {
    std::string        s;
    std::istringstream iss(line);

    std::getline(iss, s, ' ');
    Order order;
    if (s == "BUY" || s == "SELL") {
        order.msg_type_ = 'A';

        if (s == "BUY") {
            order.is_buy_ = true;
        }

        std::getline(iss, s, ' ');
        if (s == "IOC") {
            order.ioc_ = true;
        }

        std::getline(iss, s, ' ');
        order.price_ = std::stoull(s);

        std::getline(iss, s, ' ');
        order.quantity_ = std::stoull(s);

        std::getline(iss, s, ' ');
        order.order_id_ = std::move(s);
        Trim(order.order_id_);
    } else if (s == "MODIFY") {
        order.msg_type_ = 'M';
        std::getline(iss, s, ' ');
        order.order_id_ = std::move(s);

        std::getline(iss, s, ' ');
        if (s == "BUY") {
            order.is_buy_ = true;
        }
        std::getline(iss, s, ' ');
        order.price_ = std::stoull(s);

        std::getline(iss, s, ' ');
        order.quantity_ = std::stoull(s);
    } else if (s == "CANCEL") {
        order.msg_type_ = 'X';
        std::getline(iss, s, ' ');

        order.order_id_ = std::move(s);
        Trim(order.order_id_);
    } else if (s == "PRINT") {
        order.msg_type_ = 'P';
    } else {
        std::cout << "ERROR: Invalid msg type: " << s << '\n';
        return {};
    }
    return {true, order};
}

int32_t main(int32_t argc, char* argv[]) {
    std::string line;
    std::string   filename{"input.csv"};
    std::ifstream infile(filename.c_str(), std::ifstream::in);
    auto market = std::make_unique<akuna::me::Market>();
    market->AddBook(akuna::book::DEFAULT_SYMBOL);
    while (std::getline(infile, line)) {
        auto data  = ReadLine(line);
        auto order = &data.second;
        if (data.first) {
            switch (order->msg_type_) {
                case 'A': {
                    akuna::book::OrderConditions conditions =
                            (order->aon_ ? akuna::book::OrderCondition::OC_ALL_OR_NONE
                                         : akuna::book::OrderCondition::OC_NO_CONDITIONS) |
                            (order->ioc_ ? akuna::book::OrderCondition::OC_IMMEDIATE_OR_CANCEL
                                         : akuna::book::OrderCondition::OC_NO_CONDITIONS);
                    market->OrderEntry(
                            std::make_shared<akuna::book::Order>(order->order_id_, order->is_buy_, order->symbol_,
                                                                 order->quantity_, order->price_),
                            conditions);
                } break;
                case 'M':
                    market->OrderModify(std::make_shared<akuna::book::Order>(
                            order->order_id_, order->is_buy_, order->symbol_, order->quantity_, order->price_));
                    break;
                case 'X':
                    market->OrderCancel(order->order_id_);
                    break;
                case 'P':
                    market->Log();
                    break;
                default:
                    std::cout << "Invalid msg type " << order->msg_type_ << '\n';
                    break;
            }
        }
    }
}