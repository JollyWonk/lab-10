#pragma once
#include "Database.h"
#include "ICashRegisterRepository.h"

class SQLiteCashRegisterRepository final : public ICashRegisterRepository {
public:
    explicit SQLiteCashRegisterRepository(CashStorage& storage);
    int add(const CashRegister& registerData) override;
    std::optional<CashRegister> findById(int id) override;
    std::vector<CashRegister> findAll() override;
    bool update(const CashRegister& registerData) override;
    bool remove(int id) override;
private:
    CashStorage& storage_;
};
