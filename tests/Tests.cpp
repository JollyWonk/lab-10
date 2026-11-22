#include "CashDeskFacade.h"
#include "Database.h"
#include "ReceiptObserver.h"
#include "ReceiptRepository.h"
#include "ReceiptState.h"
#include "ReceiptStrategy.h"
#include "SQLiteCashRegisterRepository.h"
#include <cmath>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#ifdef _WIN32
#include <windows.h>
#endif

namespace {
void check(bool condition, const char* description) {
    if (!condition) throw std::runtime_error(description);
}

void testStrategy() {
    ReceiptCalculator calculator(std::make_unique<SequentialCorrection>());
    check(std::abs(calculator.calculate({500, 50, 20}) - 470) < 0.0001, "SequentialCorrection");
    calculator.setStrategy(std::make_unique<WeightedCorrection>());
    check(std::abs(calculator.calculate({500, 50, 20}) - 482) < 0.0001, "WeightedCorrection");
    calculator.setStrategy(std::make_unique<CoefficientCorrection>());
    check(std::abs(calculator.calculate({500, 50, 20}) - 468) < 0.0001, "CoefficientCorrection");
    bool rejected = false;
    try { calculator.calculate({-1, 0, 0}); }
    catch (const std::invalid_argument&) { rejected = true; }
    check(rejected, "Negative values rejected");
    std::cout << "[OK] Strategy\n";
}

void testStateAndFacade() {
    auto storage = createStorage(":memory:");
    initializeStorage(storage);
    SQLiteCashRegisterRepository registers(storage);
    const int registerId = registers.add({0, "Каса №1", "Зал", "Касир", "Відкрита"});
    ReceiptRepository repository(storage);
    ReceiptEventPublisher publisher;
    CashDeskFacade facade(repository, publisher);
    const int id = facade.createReceipt(registerId, "TEST-1", {300, 30, 10});
    check(repository.findById(id)->status == "Створений", "Initial state persisted");
    check(repository.findHistory(id).size() == 1, "Creation history persisted");

    bool rejected = false;
    try { facade.closeReceipt(id); }
    catch (const std::invalid_argument&) { rejected = true; }
    check(rejected && repository.findById(id)->status == "Створений", "Invalid transition blocked");
    check(repository.findHistory(id).size() == 1, "Invalid transition no history");

    facade.startProcessing(id);
    facade.closeReceipt(id);
    check(repository.findById(id)->status == "Закритий", "Closed state persisted");
    const auto history = repository.findHistory(id);
    check(history.size() == 3 && history[1].previousStatus == "Створений" &&
          history[1].newStatus == "Опрацьовується" &&
          history[2].newStatus == "Закритий", "Correct history order");
    rejected = false;
    try { facade.cancelReceipt(id); }
    catch (const std::invalid_argument&) { rejected = true; }
    check(rejected, "Closed is terminal");

    facade.setStrategy(std::make_unique<WeightedCorrection>());
    const int cancelledId = facade.createReceipt(registerId, "TEST-2", {500, 50, 20});
    check(std::abs(repository.findById(cancelledId)->totalAmount - 482) < 0.001,
          "Facade dynamic strategy");
    facade.cancelReceipt(cancelledId);
    check(repository.findById(cancelledId)->status == "Скасований", "Cancellation");
    std::cout << "[OK] State + Facade + SQLite history\n";
}

void testObserver() {
    auto storage = createStorage(":memory:");
    initializeStorage(storage);
    SQLiteCashRegisterRepository registers(storage);
    const int registerId = registers.add({0, "Каса №1", "Зал", "Касир", "Відкрита"});
    ReceiptRepository repository(storage);
    ReceiptEventPublisher publisher;
    auto journal = std::make_shared<CashRegisterJournal>();
    auto stats = std::make_shared<SalesStatistics>();
    publisher.subscribe(journal);
    publisher.subscribe(stats);
    CashDeskFacade facade(repository, publisher);
    const int first = facade.createReceipt(registerId, "EVENT-1", {100, 0, 0});
    facade.startProcessing(first);
    facade.closeReceipt(first);
    check(journal->events().size() == 2 && stats->createdCount() == 1 &&
          stats->closedCount() == 1 && stats->closedTotal() == 100, "Two independent observers");
    publisher.unsubscribe(stats);
    const int second = facade.createReceipt(registerId, "EVENT-2", {40, 0, 0});
    facade.startProcessing(second);
    facade.closeReceipt(second);
    check(journal->events().size() == 4 && stats->createdCount() == 1 &&
          stats->closedCount() == 1, "Observer unsubscribe");
    std::cout << "[OK] Observer\n";
}

void testAtomicRollback() {
    auto storage = createStorage(":memory:");
    initializeStorage(storage);
    SQLiteCashRegisterRepository registers(storage);
    const int registerId = registers.add({0, "Каса №1", "Зал", "Касир", "Відкрита"});
    ReceiptRepository repository(storage);
    ReceiptEventPublisher publisher;
    CashDeskFacade facade(repository, publisher);
    const int id = facade.createReceipt(registerId, "ROLLBACK", {100, 0, 0});
    const auto before = repository.findHistory(id).size();
    bool expectedFailure = false;
    try {
        storage.transaction([&] {
            auto item = storage.get<Receipt>(id);
            item.status = "Закритий";
            storage.update(item);
            // FOREIGN KEY: історія посилається на неіснуючий чек.
            storage.insert(ReceiptStatusHistory{0, -999, "Створений", "Закритий", "2026-11-22 12:00:00"});
            return true;
        });
    } catch (const std::exception&) { expectedFailure = true; }
    check(expectedFailure, "FOREIGN KEY failure raised");
    check(repository.findById(id)->status == "Створений" &&
          repository.findHistory(id).size() == before, "SQLite transaction rolled back both changes");
    std::cout << "[OK] Transaction rollback\n";
}
}

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    try {
        testStrategy();
        testStateAndFacade();
        testObserver();
        testAtomicRollback();
        std::cout << "Усі модульні тести пройдено успішно.\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Тест не пройдено: " << e.what() << '\n';
        return 1;
    }
}
