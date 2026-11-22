#pragma once
#include <string>

struct Receipt {
    int id = 0;
    int cashRegisterId = 0;
    std::string receiptNumber;
    std::string issuedAt;
    double totalAmount = 0.0;
    std::string paymentMethod;
    int itemCount = 0;
    std::string status = "Створений";
};
