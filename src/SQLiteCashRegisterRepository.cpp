#include "SQLiteCashRegisterRepository.h"

SQLiteCashRegisterRepository::SQLiteCashRegisterRepository(CashStorage& storage) : storage_(storage) {}

int SQLiteCashRegisterRepository::add(const CashRegister& item) {
    return storage_.insert(item);
}

std::optional<CashRegister> SQLiteCashRegisterRepository::findById(int id) {
    return storage_.get_optional<CashRegister>(id);
}

std::vector<CashRegister> SQLiteCashRegisterRepository::findAll() {
    return storage_.get_all<CashRegister>();
}

bool SQLiteCashRegisterRepository::update(const CashRegister& item) {
    if (!findById(item.id)) return false;
    storage_.update(item);
    return true;
}

bool SQLiteCashRegisterRepository::remove(int id) {
    if (!findById(id)) return false;
    storage_.remove<CashRegister>(id);
    return true;
}
