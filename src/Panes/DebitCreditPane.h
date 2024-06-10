#pragma once

#include <memory>
#include <string>
#include <cstdint>

#include <ImGuiPack.h>

#include <Headers/DatasDef.h>

class ProjectFile;
class DebitCreditPane : public AbstractPane {
private:
    struct GroupedTransaction {
        RowID id = 0;
        TransactionDate date;
        TransactionDescription description;
        EntityName entity;
        CategoryName category;
        OperationName operation;
        TransactionDebit debit = 0.0;
        TransactionCredit credit = 0.0;
        TransactionAmount amount = 0.0;
    };
    std::vector<Account> m_Accounts;
    std::vector<GroupedTransaction> m_Transactions;
    ImWidgets::QuickStringCombo m_AccountsCombo;
    ImWidgets::QuickStringCombo m_DateFormatCombo;
    ImWidgets::QuickStringCombo m_GroupByCombo;
    ImGuiListClipper m_TransactionsListClipper;
    struct BarDatas {
        std::vector<const char*> labels;
        std::vector<double> dates;
        std::vector<double> values;
        std::vector<double> debits;
        std::vector<double> credits;
        std::vector<double> amounts;
        int32_t count = 0;
        float half_width = 0.0f;
        void clear() {
            labels.clear();
            dates.clear();
            values.clear();
            debits.clear();
            credits.clear();
            amounts.clear();
            half_width = 0.0f;
        }
    } m_BarDatas;
    double m_BarHalfWidthPercent = 0.3;

public:
    bool Init() override;
    void Unit() override;
    bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawOverlays(
        const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawPanes(
        const uint32_t& vCurrentFrame, bool* vOpened = nullptr, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawDialogsAndPopups(
        const uint32_t& vCurrentFrame, const ImRect& vRect,  ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    void Load();

private:
    void m_refreshDatas();
    void m_drawMenu();
    void m_drawBuySellGraph();
    void m_drawBuySellList();
    void m_UpdateAccounts();
    void m_UpdateTransactions(const RowID& vAccountID);
    void m_UpdateBarDatas();
    void m_ComptueBarsWidth();

public:  // singleton
    static std::shared_ptr<DebitCreditPane> Instance() {
        static auto _instance = std::make_shared<DebitCreditPane>();
        return _instance;
    }

public:
    DebitCreditPane();                              // Prevent construction
    DebitCreditPane(const DebitCreditPane&) = default;  // Prevent construction by copying
    DebitCreditPane& operator=(const DebitCreditPane&) {
        return *this;
    };                       // Prevent assignment
    virtual ~DebitCreditPane();  // Prevent unwanted destruction};
};
