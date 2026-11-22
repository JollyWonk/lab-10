#pragma once
#include "Database.h"
#include <optional>
#include <vector>

class ReceiptRepository {
public:
    explicit ReceiptRepository(CashStorage& storage);
    int add(const Receipt& receipt);
    std::vector<Receipt> findByCashRegisterId(int cashRegisterId);
    std::optional<Receipt> findById(int receiptId);
    std::vector<ReceiptStatusHistory> findHistory(int receiptId);
    int addWithInitialHistory(const Receipt& receipt);
    void changeStatusWithHistory(int receiptId, const std::string& from, const std::string& to);
private:
    CashStorage& storage_;
};
