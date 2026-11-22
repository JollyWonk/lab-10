#include "CashDeskFacade.h"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace {
std::string nowText() {
    const std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm result{};
#ifdef _WIN32
    localtime_s(&result, &now);
#else
    localtime_r(&now, &result);
#endif
    std::ostringstream out;
    out << std::put_time(&result, "%Y-%m-%d %H:%M:%S");
    return out.str();
}
}

int CashDeskFacade::createReceipt(int registerId, const std::string& receiptNumber,
                                  const ReceiptCalculation& amounts, int itemCount,
                                  const std::string& paymentMethod) {
    if (itemCount < 1) throw std::invalid_argument("Кількість позицій має бути додатною");
    const double total = calculator_.calculate(amounts);
    return service_.createReceipt(Receipt{
        0, registerId, receiptNumber, nowText(), total, paymentMethod, itemCount, "Створений"
    });
}
