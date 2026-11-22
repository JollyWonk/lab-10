#pragma once
#include "CashRegister.h"
#include <optional>
#include <vector>

class ICashRegisterRepository {
public:
    virtual ~ICashRegisterRepository() = default;
    virtual int add(const CashRegister& registerData) = 0;
    virtual std::optional<CashRegister> findById(int id) = 0;
    virtual std::vector<CashRegister> findAll() = 0;
    virtual bool update(const CashRegister& registerData) = 0;
    virtual bool remove(int id) = 0;
};
