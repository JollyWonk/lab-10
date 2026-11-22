#include "ReceiptRepository.h"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace {
std::string currentTimestamp() {
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

ReceiptRepository::ReceiptRepository(CashStorage& storage) : storage_(storage) {}

int ReceiptRepository::add(const Receipt& item) {
    return storage_.insert(item);
}

std::vector<Receipt> ReceiptRepository::findByCashRegisterId(int id) {
    using namespace sqlite_orm;
    return storage_.get_all<Receipt>(where(c(&Receipt::cashRegisterId) == id),
                                     order_by(&Receipt::issuedAt).asc());
}

std::optional<Receipt> ReceiptRepository::findById(int id) {
    return storage_.get_optional<Receipt>(id);
}

std::vector<ReceiptStatusHistory> ReceiptRepository::findHistory(int receiptId) {
    using namespace sqlite_orm;
    return storage_.get_all<ReceiptStatusHistory>(
        where(c(&ReceiptStatusHistory::receiptId) == receiptId),
        order_by(&ReceiptStatusHistory::id).asc());
}

int ReceiptRepository::addWithInitialHistory(const Receipt& item) {
    int receiptId = 0;
    const bool committed = storage_.transaction([&] {
        receiptId = storage_.insert(item);
        storage_.insert(ReceiptStatusHistory{0, receiptId, "", item.status, currentTimestamp()});
        return true;
    });
    if (!committed) throw std::runtime_error("Транзакцію створення чека скасовано");
    return receiptId;
}

void ReceiptRepository::changeStatusWithHistory(int id, const std::string& from, const std::string& to) {
    const bool committed = storage_.transaction([&] {
        auto item = storage_.get<Receipt>(id);
        if (item.status != from) throw std::runtime_error("Стан чека було змінено іншим процесом");
        item.status = to;
        storage_.update(item);
        storage_.insert(ReceiptStatusHistory{0, id, from, to, currentTimestamp()});
        return true;
    });
    if (!committed) throw std::runtime_error("Транзакцію зміни стану скасовано");
}
