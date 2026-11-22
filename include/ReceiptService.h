#pragma once
#include "ReceiptObserver.h"
#include "ReceiptRepository.h"
#include "ReceiptState.h"

class ReceiptService {
public:
    ReceiptService(ReceiptRepository& repository, ReceiptEventPublisher& events);
    int createReceipt(const Receipt& receipt);
    void changeStatus(int receiptId, const std::string& newStatus);
private:
    ReceiptRepository& repository_;
    ReceiptEventPublisher& events_;
};
