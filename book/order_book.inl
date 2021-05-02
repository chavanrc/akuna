namespace akuna::book {
    template <class OrderPtr>
    OrderBook<OrderPtr>::OrderBook(Symbol symbol) : symbol_(symbol) {
        callbacks_.reserve(16);
        working_callbacks_.reserve(callbacks_.capacity());
    }

    template <class OrderPtr>
    auto OrderBook<OrderPtr>::Add(const OrderPtr &order, OrderConditions conditions) -> bool {
        bool matched = false;

        if (order->GetQuantity() <= 0) {
            callbacks_.push_back(TypedCallback::Reject(order, "size must be positive"));
        } else {
            size_t accept_cb_index = callbacks_.size();
            callbacks_.push_back(TypedCallback::Accept(order));
            Tracker inbound(order, conditions);
            matched                               = SubmitOrder(inbound);
            callbacks_[accept_cb_index].quantity_ = inbound.FilledQty();
            if (inbound.ImmediateOrCancel() && !inbound.Filled()) {
                callbacks_.push_back(TypedCallback::Cancel(order, 0));
            }
            callbacks_.push_back(TypedCallback::BookUpdate(this));
        }
        CallbackNow();
        return matched;
    }

    template <class OrderPtr>
    auto OrderBook<OrderPtr>::Cancel(const OrderPtr &order) -> void {
        bool     found    = false;
        Quantity open_qty = 0;

        if (order->IsBuy()) {
            typename TrackerMap::iterator bid;
            if (FindOnMarket(order, bid) && bid != bids_.end()) {
                open_qty = bid->second.OpenQty();
                bids_.erase(bid);
                found = true;
            }
        } else {
            typename TrackerMap::iterator ask;
            if (FindOnMarket(order, ask) && ask != asks_.end()) {
                open_qty = ask->second.OpenQty();
                asks_.erase(ask);
                found = true;
            }
        }
        if (found) {
            callbacks_.push_back(TypedCallback::Cancel(order, open_qty));
            callbacks_.push_back(TypedCallback::BookUpdate(this));
        } else {
            callbacks_.push_back(TypedCallback::CancelReject(order, "not found"));
        }
        CallbackNow();
    }

    template <class OrderPtr>
    auto OrderBook<OrderPtr>::Replace(const OrderPtr &passivated_order, const OrderPtr &new_order) -> bool {
        bool                          matched = false;
        TrackerMap &                  market  = passivated_order->IsBuy() ? bids_ : asks_;
        typename TrackerMap::iterator pos;

        if (passivated_order->IsBuy() != new_order->IsBuy()) {
            if (FindOnMarket(passivated_order, pos)) {
                market.erase(pos);
                callbacks_.push_back(TypedCallback::BookUpdate(this));
                matched = Add(new_order, book::OrderCondition::OC_NO_CONDITIONS);
            } else {
                callbacks_.push_back(TypedCallback::ReplaceReject(new_order, "not found"));
            }
        } else {
            if (FindOnMarket(passivated_order, pos)) {
                callbacks_.push_back(TypedCallback::Accept(new_order));
                callbacks_.push_back(TypedCallback::Replace(passivated_order, pos->second.OpenQty(), new_order));
                market.erase(pos);
                Tracker inbound(new_order, book::OrderCondition::OC_NO_CONDITIONS);
                matched = AddOrder(inbound, new_order->GetPrice());
                callbacks_.push_back(TypedCallback::BookUpdate(this));
            } else {
                callbacks_.push_back(TypedCallback::ReplaceReject(new_order, "not found"));
            }
        }

        CallbackNow();
        return matched;
    }

    template <class OrderPtr>
    auto OrderBook<OrderPtr>::MarketPrice(Price price) -> void {
        market_price_ = price;
    }

    template <class OrderPtr>
    auto OrderBook<OrderPtr>::MatchOrder(Tracker &inbound, Price inbound_price, TrackerMap &current_orders) -> bool {
        if (inbound.AllOrNone()) {
            // TODO
            return false;
        } else {
            return MatchRegularOrder(inbound, inbound_price, current_orders);
        }
    }

    template <class OrderPtr>
    auto OrderBook<OrderPtr>::MatchRegularOrder(Tracker &inbound, Price inbound_price, TrackerMap &current_orders)
            -> bool {
        bool matched = false;
        auto pos     = current_orders.begin();
        while (pos != current_orders.end() && !inbound.Filled()) {
            auto                   entry         = pos++;
            const ComparablePrice &current_price = entry->first;
            if (!current_price.Matches(inbound_price)) {
                break;
            }

            Tracker &current_order = entry->second;
            Quantity traded        = CreateTrade(inbound, current_order);
            if (traded > 0) {
                matched = true;
                if (current_order.Filled()) {
                    current_orders.erase(entry);
                }
            }
        }
        return matched;
    }

    template <class OrderPtr>
    auto OrderBook<OrderPtr>::CreateTrade(Tracker &inbound_tracker, Tracker &current_tracker, Quantity max_quantity)
            -> Quantity {
        Price cross_price = current_tracker.Ptr()->GetPrice();
        // If current order is a market order, cross at inbound price
        if (MARKET_ORDER_PRICE == cross_price) {
            cross_price = inbound_tracker.Ptr()->GetPrice();
        }
        if (MARKET_ORDER_PRICE == cross_price) {
            cross_price = market_price_;
        }
        if (MARKET_ORDER_PRICE == cross_price) {
            // No price available for this order
            return 0;
        }
        Quantity fill_qty = std::min(max_quantity, std::min(inbound_tracker.OpenQty(), current_tracker.OpenQty()));
        if (fill_qty > 0) {
            inbound_tracker.Fill(fill_qty);
            current_tracker.Fill(fill_qty);
            MarketPrice(cross_price);
            callbacks_.push_back(
                    TypedCallback::Fill(inbound_tracker.Ptr(), current_tracker.Ptr(), fill_qty, cross_price));
        }
        return fill_qty;
    }

    template <class OrderPtr>
    auto OrderBook<OrderPtr>::FindOnMarket(const OrderPtr &order, typename TrackerMap::iterator &result) -> bool {
        const ComparablePrice KEY(order->IsBuy(), order->GetPrice());
        TrackerMap &          side_map = order->IsBuy() ? bids_ : asks_;

        for (result = side_map.find(KEY); result != side_map.end(); ++result) {
            if (result->second.Ptr() == order) {
                return true;
            } else if (KEY < result->first) {
                result = side_map.end();
                return false;
            }
        }
        return false;
    }

    template <class OrderPtr>
    auto OrderBook<OrderPtr>::SubmitOrder(Tracker &inbound) -> bool {
        Price order_price = inbound.Ptr()->GetPrice();
        return AddOrder(inbound, order_price);
    }

    template <class OrderPtr>
    auto OrderBook<OrderPtr>::AddOrder(Tracker &inbound, Price order_price) -> bool {
        bool      matched;
        OrderPtr &order = inbound.Ptr();
        if (order->IsBuy()) {
            matched = MatchOrder(inbound, order_price, asks_);
        } else {
            matched = MatchOrder(inbound, order_price, bids_);
        }

        if (inbound.OpenQty() && !inbound.ImmediateOrCancel()) {
            if (order->IsBuy()) {
                bids_.insert({ComparablePrice(true, order_price), inbound});
            } else {
                asks_.insert({ComparablePrice(false, order_price), inbound});
            }
        }
        return matched;
    }

    template <class OrderPtr>
    auto OrderBook<OrderPtr>::OnAccept(const OrderPtr &order, Quantity quantity) -> void {
        order->OnAccepted();
        LOG_DEBUG("Event: Accepted: " << *order);
    }

    template <class OrderPtr>
    auto OrderBook<OrderPtr>::OnReject(const OrderPtr &order, const char *reason) -> void {
        order->OnRejected(reason);
        LOG_DEBUG("Event: Rejected: " << *order << ' ' << reason);
    }

    template <class OrderPtr>
    auto OrderBook<OrderPtr>::OnFill(const OrderPtr &order, const OrderPtr &matched_order, Quantity fill_qty,
                                     Cost fill_cost, FillId fill_id) -> void {
        order->OnFilled(fill_qty, fill_cost);
        matched_order->OnFilled(fill_qty, fill_cost);

        std::stringstream out;
        out << "TRADE " << matched_order->GetOrderId() << ' ' << matched_order->GetPrice() << ' ' << fill_qty << ' '
            << order->GetOrderId() << ' ' << order->GetPrice() << ' ' << fill_qty;
        LOG_INFO(out.str());

        order->AddTradeHistory(fill_qty, matched_order->QuantityOnMarket(), fill_cost, matched_order->GetOrderId(),
                               matched_order->GetPrice(), fill_id);
        matched_order->AddTradeHistory(fill_qty, order->QuantityOnMarket(), fill_cost, order->GetOrderId(),
                                       order->GetPrice(), fill_id);
    }

    template <class OrderPtr>
    auto OrderBook<OrderPtr>::OnCancel(const OrderPtr &order, Quantity quantity) -> void {
        order->OnCancelled();
        LOG_DEBUG("Event: Canceled: " << *order);
    }

    template <class OrderPtr>
    auto OrderBook<OrderPtr>::OnCancelReject(const OrderPtr &order, const char *reason) -> void {
        order->OnCancelRejected(reason);
        LOG_DEBUG("Event: Cancel Reject: " << *order << ' ' << reason);
    }

    template <class OrderPtr>
    auto OrderBook<OrderPtr>::OnReplace(const OrderPtr &order, Delta delta, Price new_price) -> void {
        order->OnReplaced(delta, new_price);
        LOG_DEBUG("Event: Replaced: " << *order);
    }

    template <class OrderPtr>
    auto OrderBook<OrderPtr>::OnReplaceReject(const OrderPtr &order, const char *reason) -> void {
        order->OnReplaceRejected(reason);
        LOG_DEBUG("Event: Replace Reject: " << *order << ' ' << reason);
    }

    template <class OrderPtr>
    auto OrderBook<OrderPtr>::OnOrderBookChange() -> void {
        // TODO
    }

    template <class OrderPtr>
    auto OrderBook<OrderPtr>::OnTrade(const OrderBook *book, const OrderId &id_1, const OrderId &id_2, Quantity qty,
                                      Price price, bool buyer_maker) -> void {
        // TODO
    }

    template <class OrderPtr>
    auto OrderBook<OrderPtr>::CallbackNow() -> void {
        if (!handling_callbacks_) {
            handling_callbacks_ = true;
            while (!callbacks_.empty()) {
                working_callbacks_.reserve(callbacks_.capacity());
                working_callbacks_.swap(callbacks_);
                for (auto &cb : working_callbacks_) {
                    try {
                        PerformCallback(cb);
                    } catch (const std::exception &ex) {
                        LOG_ERROR("Caught exception during callback: " << ex.what());
                    } catch (...) {
                        LOG_ERROR("Caught unknown exception during callback");
                    }
                }
                working_callbacks_.clear();
            }
            handling_callbacks_ = false;
        }
    }

    template <class OrderPtr>
    void OrderBook<OrderPtr>::PerformCallback(TypedCallback &cb) {
        switch (cb.type_) {
            case TypedCallback::CbType::CB_ORDER_FILL: {
                Cost fill_cost = cb.price_ * cb.quantity_;
                // generate new trade id
                static FillId fill_id{0};
                ++fill_id;
                OnFill(cb.order_, cb.matched_order_, cb.quantity_, fill_cost, fill_id);
                OrderId buy_order_id, sell_order_id;
                if (cb.matched_order_->IsBuy()) {
                    buy_order_id  = cb.matched_order_->GetOrderId();
                    sell_order_id = cb.order_->GetOrderId();
                } else {
                    buy_order_id  = cb.order_->GetOrderId();
                    sell_order_id = cb.matched_order_->GetOrderId();
                }
                bool buyer_maker = cb.matched_order_->IsBuy();
                OnTrade(this, buy_order_id, sell_order_id, cb.quantity_, cb.price_, buyer_maker);
                break;
            }
            case TypedCallback::CbType::CB_ORDER_ACCEPT:
                OnAccept(cb.order_, cb.quantity_);
                break;
            case TypedCallback::CbType::CB_ORDER_REJECT:
                OnReject(cb.order_, cb.reject_reason_);
                break;
            case TypedCallback::CbType::CB_ORDER_CANCEL:
                OnCancel(cb.order_, cb.quantity_);
                break;
            case TypedCallback::CbType::CB_ORDER_CANCEL_REJECT:
                OnCancelReject(cb.order_, cb.reject_reason_);
                break;
            case TypedCallback::CbType::CB_ORDER_REPLACE:
                OnReplace(cb.order_, cb.delta_, cb.price_);
                break;
            case TypedCallback::CbType::CB_ORDER_REPLACE_REJECT:
                OnReplaceReject(cb.order_, cb.reject_reason_);
                break;
            case TypedCallback::CbType::CB_BOOK_UPDATE:
                OnOrderBookChange();
                break;
            default: {
                std::stringstream msg;
                msg << "Unexpected callback type " << cb.type_;
                throw std::runtime_error(msg.str());
            }
        }
    }

    template <class OrderPtr>
    auto OrderBook<OrderPtr>::Log() const -> void {
        LOG_INFO("SELL:");
        std::map<Price, Quantity> book;
        for (auto ask = asks_.begin(); ask != asks_.end(); ++ask) {
            book[ask->first.GetPrice()] += ask->second.OpenQty();
        }
        for (auto level = book.rbegin(); level != book.rend(); ++level) {
            LOG_INFO(level->first << ' ' << level->second);
        }
        book.clear();
        LOG_INFO("BUY:");
        for (auto bid = bids_.begin(); bid != bids_.end(); ++bid) {
            book[bid->first.GetPrice()] += bid->second.OpenQty();
        }
        for (auto level = book.rbegin(); level != book.rend(); ++level) {
            LOG_INFO(level->first << ' ' << level->second);
        }
    }
}    // namespace akuna::book