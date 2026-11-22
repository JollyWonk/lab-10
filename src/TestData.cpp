#include "TestData.h"
#include "SQLiteCashRegisterRepository.h"
#include "ReceiptRepository.h"
#include <array>
#include <stdexcept>
#include <string>

void seedTestData(CashStorage& storage) {
    SQLiteCashRegisterRepository registers(storage);
    ReceiptRepository receipts(storage);
    const std::array<CashRegister, 5> examples = {{
        {0, "Каса №1", "Торговий зал", "Олена Коваль", "Відкрита"},
        {0, "Каса №2", "Торговий зал", "Ігор Петренко", "Відкрита"},
        {0, "Каса №3", "Торговий зал", "Марія Савчук", "Закрита"},
        {0, "Каса №4", "Експрес-зона", "Андрій Шевченко", "Відкрита"},
        {0, "Каса №5", "Самообслуговування", "Система самообслуговування", "Відкрита"}
    }};
    const std::array<std::array<double, 3>, 5> amounts = {{
        {{125.50, 240.00, 89.90}},
        {{320.00, 75.25, 146.75}},
        {{58.40, 612.30, 199.99}},
        {{410.00, 135.60, 92.50}},
        {{220.00, 45.80, 310.40}}
    }};
    const std::array<std::string, 3> payment = {"Картка", "Готівка", "Картка"};
    const std::array<int, 3> items = {3, 6, 2};
    for (size_t i = 0; i < examples.size(); ++i) {
        const int registerId = registers.add(examples[i]);
        for (int j = 0; j < 3; ++j) {
            const auto number = static_cast<int>(i * 3 + j + 1);
            const std::string code = std::string("CHK-23-") + (number < 10 ? "00" : "0") + std::to_string(number);
            const std::string time = "2026-11-08 09:" + std::string(j == 0 ? "00:00" : (j == 1 ? "10:00" : "20:00"));
            receipts.add(Receipt{0, registerId, code, time, amounts[i][j], payment[j], items[j]});
        }
    }
    if (storage.count<CashRegister>() != 5 || storage.count<Receipt>() != 15) {
        throw std::runtime_error("Некоректна кількість тестових даних (очікується 5 кас та 15 чеків)");
    }
}
