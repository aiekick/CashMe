#pragma once

#include <memory>
#include <string>
#include <cstdint>

#include <imguipack.h>

#include <Systems/FrameActionSystem.h>

#include <Threads/ImportWorkerThread.h>

#include <Frontend/Dialogs/BankDialog.h>
#include <Frontend/Dialogs/AccountDialog.h>
#include <Frontend/Dialogs/EntityDialog.h>
#include <Frontend/Dialogs/CategoryDialog.h>
#include <Frontend/Dialogs/OperationDialog.h>
#include <Frontend/Dialogs/TransactionDialog.h>
#include <Frontend/Tables/TransactionsTable.h>

class ProjectFile;
class StatementsPane : public AbstractPane {
private:
    TransactionsTable m_TransactionsTable;
    ImportWorkerThread m_ImportThread;
    BankDialog m_BankDialog;
    AccountDialog m_AccountDialog;
    EntityDialog m_EntityDialog;
    CategoryDialog m_CategoryDialog;
    OperationDialog m_OperationDialog;
    DataBrockerContainer m_DataBrokerModules;
    Cash::BankStatementModuleWeak m_SelectedBroker;

public:
    bool Init() override;
    void Unit() override;
    bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawPanes(const uint32_t& vCurrentFrame, bool* vOpened = nullptr, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawDialogsAndPopups(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;

    void DoBackend();

    void Load();

private:
    void m_importFromFiles(const std::vector<std::string>& vFiles);
    void m_drawAccountMenu(const AccountOutput& vAccount);
    void m_drawCreationMenu();
    void m_drawImportMenu(FrameActionSystem& vFrameActionSystem);
    void m_getAvailableDataBrokers();
    void m_clear();

public:  // singleton
    static std::shared_ptr<StatementsPane> Instance() {
        static std::shared_ptr<StatementsPane> _instance = std::make_shared<StatementsPane>();
        return _instance;
    }

public:
    StatementsPane();                              // Prevent construction
    StatementsPane(const StatementsPane&) = delete;  // Prevent construction by copying
    StatementsPane& operator=(const StatementsPane&) {
        return *this;
    };                       // Prevent assignment
    virtual ~StatementsPane();  // Prevent unwanted destruction};
};
