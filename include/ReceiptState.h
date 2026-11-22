#pragma once
#include <memory>
#include <stdexcept>
#include <string>

class ReceiptState {
public:
    virtual ~ReceiptState() = default;
    virtual std::string name() const = 0;
    virtual bool canTransitionTo(const std::string& next) const = 0;
};

class CreatedReceiptState final : public ReceiptState {
public:
    std::string name() const override { return "Створений"; }
    bool canTransitionTo(const std::string& next) const override {
        return next == "Опрацьовується" || next == "Скасований";
    }
};

class ProcessingReceiptState final : public ReceiptState {
public:
    std::string name() const override { return "Опрацьовується"; }
    bool canTransitionTo(const std::string& next) const override {
        return next == "Закритий" || next == "Скасований";
    }
};

class ClosedReceiptState final : public ReceiptState {
public:
    std::string name() const override { return "Закритий"; }
    bool canTransitionTo(const std::string&) const override { return false; }
};

class CancelledReceiptState final : public ReceiptState {
public:
    std::string name() const override { return "Скасований"; }
    bool canTransitionTo(const std::string&) const override { return false; }
};

inline std::unique_ptr<ReceiptState> makeReceiptState(const std::string& status) {
    if (status == "Створений") return std::make_unique<CreatedReceiptState>();
    if (status == "Опрацьовується") return std::make_unique<ProcessingReceiptState>();
    if (status == "Закритий") return std::make_unique<ClosedReceiptState>();
    if (status == "Скасований") return std::make_unique<CancelledReceiptState>();
    throw std::invalid_argument("Невідомий стан чека: " + status);
}
