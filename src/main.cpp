#include "CashDeskFacade.h"
#include "Database.h"
#include "ReceiptObserver.h"
#include "ReceiptRepository.h"
#include "ReceiptState.h"
#include "ReceiptStrategy.h"
#include "TestData.h"
#include <cmath>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#ifdef _WIN32
#include <windows.h>
#endif

namespace {
void require(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error("Помилка перевірки: " + message);
    std::cout << "[OK] " << message << '\n';
}

void stateDemo(CashStorage& storage) {
    std::cout << "\n=== STATE: стани чека та історія в SQLite ===\n";
    ReceiptRepository repository(storage);
    ReceiptEventPublisher publisher;
    CashDeskFacade desk(repository, publisher);
    const int id = desk.createReceipt(1, "LAB10-STATE", {500, 50, 20}, 4);
    desk.startProcessing(id);
    desk.closeReceipt(id);
    bool blocked = false;
    try { desk.cancelReceipt(id); }
    catch (const std::invalid_argument& e) {
        blocked = true;
        std::cout << "Недопустимий перехід відхилено: " << e.what() << '\n';
    }
    require(blocked && repository.findById(id)->status == "Закритий",
            "Неприпустимий перехід не змінив стан");
    for (const auto& row : repository.findHistory(id)) {
        std::cout << "Історія: " << (row.previousStatus.empty() ? "(новий)" : row.previousStatus)
                  << " -> " << row.newStatus << " | " << row.changedAt << '\n';
    }
    require(repository.findHistory(id).size() == 3,
            "Створення та два переходи збережено в історії БД");
    const int cancelledId = desk.createReceipt(1, "LAB10-CANCEL", {100, 0, 0});
    desk.cancelReceipt(cancelledId);
    require(repository.findById(cancelledId)->status == "Скасований",
            "Окремий стан 'Скасований' працює");
}

void strategyDemo() {
    std::cout << "\n=== STRATEGY: три алгоритми суми чека ===\n";
    ReceiptCalculator calculator(std::make_unique<SequentialCorrection>());
    const ReceiptCalculation params{500.0, 50.0, 20.0};
    const auto sequential = calculator.calculate(params);
    std::cout << "Послідовне коригування: " << sequential << " грн\n";
    calculator.setStrategy(std::make_unique<WeightedCorrection>());
    const auto weighted = calculator.calculate(params);
    std::cout << "Зважене коригування: " << weighted << " грн\n";
    calculator.setStrategy(std::make_unique<CoefficientCorrection>());
    const auto coefficient = calculator.calculate(params);
    std::cout << "Коригування через коефіцієнти: " << coefficient << " грн\n";
    require(std::abs(sequential - 470.0) < 0.001 &&
            std::abs(weighted - 482.0) < 0.001 &&
            std::abs(coefficient - 468.0) < 0.001,
            "Усі три стратегії можна замінювати під час виконання");
}

void observerDemo(CashStorage& storage) {
    std::cout << "\n=== OBSERVER: журнал каси та статистика продажів ===\n";
    ReceiptEventPublisher publisher;
    auto journal = std::make_shared<CashRegisterJournal>();
    auto statistics = std::make_shared<SalesStatistics>();
    publisher.subscribe(journal);
    publisher.subscribe(statistics);
    ReceiptRepository repository(storage);
    CashDeskFacade desk(repository, publisher);
    const int first = desk.createReceipt(1, "LAB10-OBS-1", {100, 10, 5});
    desk.startProcessing(first);
    desk.closeReceipt(first);
    require(statistics->createdCount() == 1 && statistics->closedCount() == 1 &&
            journal->events().size() == 2,
            "Обидва підписники отримали створення та закриття чека");
    publisher.unsubscribe(statistics);
    std::cout << "Статистику відключено від подій.\n";
    const int second = desk.createReceipt(1, "LAB10-OBS-2", {200, 0, 0});
    desk.startProcessing(second);
    desk.closeReceipt(second);
    require(statistics->createdCount() == 1 && statistics->closedCount() == 1 &&
            journal->events().size() == 4,
            "Після відписки повідомлення отримує лише журнал каси");
}

void facadeDemo(CashStorage& storage) {
    std::cout << "\n=== FACADE: єдиний інтерфейс роботи з чеком ===\n";
    ReceiptRepository repository(storage);
    ReceiptEventPublisher publisher;
    auto journal = std::make_shared<CashRegisterJournal>();
    publisher.subscribe(journal);
    CashDeskFacade desk(repository, publisher);
    desk.setStrategy(std::make_unique<WeightedCorrection>());
    const int id = desk.createReceipt(2, "LAB10-FACADE", {500, 50, 20}, 5, "Картка");
    desk.startProcessing(id);
    desk.closeReceipt(id);
    const auto receipt = repository.findById(id);
    require(receipt && receipt->cashRegisterId == 2 &&
            receipt->status == "Закритий" && std::abs(receipt->totalAmount - 482.0) < 0.001,
            "Facade об'єднав Strategy, State, Observer та Repository");
    std::cout << "Чек №" << id << " | " << receipt->receiptNumber
              << " | " << receipt->totalAmount << " грн | " << receipt->status << '\n';
}
}

int main(int argc, char** argv) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    const std::string mode = argc > 1 ? argv[1] : "all";
    if (mode == "--help" || mode == "help") {
        std::cout << "Режими: state, strategy, observer, facade, all\n";
        return 0;
    }
    if (mode != "all" && mode != "state" && mode != "strategy" &&
        mode != "observer" && mode != "facade") {
        std::cerr << "Невідомий режим: " << mode << '\n';
        return 2;
    }
    try {
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "Лабораторна робота №10 | Варіант 23 | Робота кас супермаркету\n";
        if (mode == "strategy") {
            strategyDemo();
        } else {
            std::filesystem::remove("cash_registers_lab10.db");
            auto storage = createStorage("cash_registers_lab10.db");
            initializeStorage(storage);
            seedTestData(storage);
            if (mode == "state" || mode == "all") stateDemo(storage);
            if (mode == "all") strategyDemo();
            if (mode == "observer" || mode == "all") observerDemo(storage);
            if (mode == "facade" || mode == "all") facadeDemo(storage);
        }
        std::cout << "\nЗАВЕРШЕНО УСПІШНО.\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "ПОМИЛКА: " << e.what() << '\n';
        return 1;
    }
}
