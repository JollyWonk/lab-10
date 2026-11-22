#pragma once
#include "ReceiptService.h"
#include "ReceiptStrategy.h"
#include <memory>
#include <string>
#include <utility>

class CashDeskFacade {
public:
    CashDeskFacade(ReceiptRepository& repository, ReceiptEventPublisher& events)
        : calculator_(std::make_unique<SequentialCorrection>()), service_(repository, events) {}

    void setStrategy(std::unique_ptr<ReceiptPricingStrategy> strategy) {
        calculator_.setStrategy(std::move(strategy));
    }

    int createReceipt(int registerId, const std::string& receiptNumber,
                      const ReceiptCalculation& amounts, int itemCount = 1,
                      const std::string& paymentMethod = "Картка");

    void startProcessing(int receiptId) { service_.changeStatus(receiptId, "Опрацьовується"); }
    void closeReceipt(int receiptId) { service_.changeStatus(receiptId, "Закритий"); }
    void cancelReceipt(int receiptId) { service_.changeStatus(receiptId, "Скасований"); }
private:
    ReceiptCalculator calculator_;
    ReceiptService service_;
};
