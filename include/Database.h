#pragma once
#include <string>
#include <stdexcept>
#include <sqlite3.h>
#include <sqlite_orm/sqlite_orm.h>
#include "CashRegister.h"
#include "Receipt.h"
#include "ReceiptStatusHistory.h"

inline auto createStorage(const std::string& databaseFile) {
    using namespace sqlite_orm;
    return make_storage(databaseFile,
        make_table("CashRegister",
            make_column("id", &CashRegister::id, primary_key().autoincrement()),
            make_column("registerNumber", &CashRegister::registerNumber),
            make_column("location", &CashRegister::location),
            make_column("cashierName", &CashRegister::cashierName),
            make_column("status", &CashRegister::status)
        ),
        make_table("Receipt",
            make_column("id", &Receipt::id, primary_key().autoincrement()),
            make_column("cashRegisterId", &Receipt::cashRegisterId),
            make_column("receiptNumber", &Receipt::receiptNumber),
            make_column("issuedAt", &Receipt::issuedAt),
            make_column("totalAmount", &Receipt::totalAmount),
            make_column("paymentMethod", &Receipt::paymentMethod),
            make_column("itemCount", &Receipt::itemCount),
            make_column("status", &Receipt::status),
            foreign_key(&Receipt::cashRegisterId).references(&CashRegister::id)
        ),
        make_table("ReceiptStatusHistory",
            make_column("id", &ReceiptStatusHistory::id, primary_key().autoincrement()),
            make_column("receiptId", &ReceiptStatusHistory::receiptId),
            make_column("previousStatus", &ReceiptStatusHistory::previousStatus),
            make_column("newStatus", &ReceiptStatusHistory::newStatus),
            make_column("changedAt", &ReceiptStatusHistory::changedAt),
            foreign_key(&ReceiptStatusHistory::receiptId).references(&Receipt::id)
        )
    );
}

using CashStorage = decltype(createStorage(""));

inline void initializeStorage(CashStorage& storage) {
    storage.on_open = [](sqlite3* db) {
        int enabled = 0;
        const int rc = sqlite3_db_config(db, SQLITE_DBCONFIG_ENABLE_FKEY, 1, &enabled);
        if (rc != SQLITE_OK || enabled != 1) {
            throw std::runtime_error("Не вдалося увімкнути контроль зовнішніх ключів SQLite");
        }
    };
    storage.open_forever();
    storage.sync_schema();
}
