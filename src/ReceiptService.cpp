#include "ReceiptService.h"
#include <stdexcept>

ReceiptService::ReceiptService(ReceiptRepository& repository, ReceiptEventPublisher& events)
    : repository_(repository), events_(events) {}

int ReceiptService::createReceipt(const Receipt& receipt) {
    if (receipt.status != "Створений") {
        throw std::invalid_argument("Новий чек повинен мати стан 'Створений'");
    }
    const int id = repository_.addWithInitialHistory(receipt);
    events_.publish({ReceiptEventType::Created, id, receipt.totalAmount});
    return id;
}

void ReceiptService::changeStatus(int receiptId, const std::string& newStatus) {
    const auto item = repository_.findById(receiptId);
    if (!item) throw std::out_of_range("Чек не знайдено");
    auto state = makeReceiptState(item->status);
    if (!state->canTransitionTo(newStatus)) {
        throw std::invalid_argument("Недопустимий перехід: " + state->name() + " -> " + newStatus);
    }
    repository_.changeStatusWithHistory(receiptId, item->status, newStatus);
    if (newStatus == "Закритий") {
        events_.publish({ReceiptEventType::Closed, receiptId, item->totalAmount});
    }
}
