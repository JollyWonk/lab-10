#pragma once
#include <string>

struct ReceiptStatusHistory {
    int id = 0;
    int receiptId = 0;
    std::string previousStatus;
    std::string newStatus;
    std::string changedAt;
};
