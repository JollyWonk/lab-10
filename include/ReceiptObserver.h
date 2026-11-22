#pragma once
#include <algorithm>
#include <iostream>
#include <memory>
#include <vector>

// Два типи подій: створення чека й закриття чека.
enum class ReceiptEventType { Created, Closed };

struct ReceiptEvent {
    ReceiptEventType type;
    int receiptId;
    double totalAmount;
};

class IReceiptObserver {
public:
    virtual ~IReceiptObserver() = default;
    virtual void update(const ReceiptEvent& event) = 0;
};

class ReceiptEventPublisher {
public:
    void subscribe(const std::shared_ptr<IReceiptObserver>& observer) {
        if (!observer) return;
        for (const auto& item : observers_) {
            if (item.lock() == observer) return;
        }
        observers_.push_back(observer);
    }

    void unsubscribe(const std::shared_ptr<IReceiptObserver>& observer) {
        std::erase_if(observers_, [&](const std::weak_ptr<IReceiptObserver>& item) {
            const auto active = item.lock();
            return !active || active == observer;
        });
    }

    void publish(const ReceiptEvent& event) {
        std::erase_if(observers_, [](const std::weak_ptr<IReceiptObserver>& item) {
            return item.expired();
        });
        for (const auto& item : observers_) {
            if (const auto observer = item.lock()) observer->update(event);
        }
    }
private:
    std::vector<std::weak_ptr<IReceiptObserver>> observers_;
};

class CashRegisterJournal final : public IReceiptObserver {
public:
    void update(const ReceiptEvent& event) override {
        events_.push_back(event);
        std::cout << "Журнал каси: чек №" << event.receiptId
                  << (event.type == ReceiptEventType::Created ? " створено" : " закрито") << '\n';
    }
    const std::vector<ReceiptEvent>& events() const { return events_; }
private:
    std::vector<ReceiptEvent> events_;
};

class SalesStatistics final : public IReceiptObserver {
public:
    void update(const ReceiptEvent& event) override {
        if (event.type == ReceiptEventType::Created) {
            ++createdCount_;
        } else {
            ++closedCount_;
            closedTotal_ += event.totalAmount;
        }
        std::cout << "Статистика продажів: створено=" << createdCount_
                  << ", закрито=" << closedCount_ << ", сума=" << closedTotal_ << " грн\n";
    }
    int createdCount() const { return createdCount_; }
    int closedCount() const { return closedCount_; }
    double closedTotal() const { return closedTotal_; }
private:
    int createdCount_ = 0;
    int closedCount_ = 0;
    double closedTotal_ = 0.0;
};
