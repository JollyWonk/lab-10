#pragma once
#include <algorithm>
#include <cmath>
#include <memory>
#include <stdexcept>
#include <utility>

struct ReceiptCalculation {
    double itemsTotal = 0.0;
    double discount = 0.0;
    double additionalCharges = 0.0;
};

class ReceiptPricingStrategy {
public:
    virtual ~ReceiptPricingStrategy() = default;
    virtual double calculate(const ReceiptCalculation& params) const = 0;
};

class SequentialCorrection final : public ReceiptPricingStrategy {
public:
    double calculate(const ReceiptCalculation& p) const override {
        return std::max(0.0, p.itemsTotal - p.discount) + p.additionalCharges;
    }
};

class WeightedCorrection final : public ReceiptPricingStrategy {
public:
    double calculate(const ReceiptCalculation& p) const override {
        return std::max(0.0, p.itemsTotal - 0.8 * p.discount + 1.1 * p.additionalCharges);
    }
};

class CoefficientCorrection final : public ReceiptPricingStrategy {
public:
    double calculate(const ReceiptCalculation& p) const override {
        if (p.itemsTotal == 0.0) return p.additionalCharges;
        const double discountCoefficient = std::max(0.0, 1.0 - p.discount / p.itemsTotal);
        const double chargeCoefficient = 1.0 + p.additionalCharges / p.itemsTotal;
        return p.itemsTotal * discountCoefficient * chargeCoefficient;
    }
};

class ReceiptCalculator {
public:
    explicit ReceiptCalculator(std::unique_ptr<ReceiptPricingStrategy> strategy)
        : strategy_(std::move(strategy)) {
        if (!strategy_) throw std::invalid_argument("Стратегія не може бути порожньою");
    }

    void setStrategy(std::unique_ptr<ReceiptPricingStrategy> strategy) {
        if (!strategy) throw std::invalid_argument("Стратегія не може бути порожньою");
        strategy_ = std::move(strategy);
    }

    double calculate(const ReceiptCalculation& p) const {
        if (!std::isfinite(p.itemsTotal) || !std::isfinite(p.discount) ||
            !std::isfinite(p.additionalCharges) || p.itemsTotal < 0 ||
            p.discount < 0 || p.additionalCharges < 0) {
            throw std::invalid_argument("Параметри розрахунку повинні бути невід'ємними скінченними числами");
        }
        return strategy_->calculate(p);
    }
private:
    std::unique_ptr<ReceiptPricingStrategy> strategy_;
};
