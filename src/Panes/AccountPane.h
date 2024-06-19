#pragma once

#include <memory>
#include <string>
#include <cstdint>

#include <ImGuiPack.h>
#include <ctools/ConfigAbstract.h>

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
class AccountPane : public AbstractPane, public conf::ConfigAbstract {
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

    std::string getXml(const std::string& vOffset, const std::string& vUserDatas) override;
    bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) override;

private:
    void m_importFromFiles(const std::vector<std::string>& vFiles);
    void m_drawAccountMenu(const Account& vAccount);
    void m_drawCreationMenu();
    void m_drawImportMenu(FrameActionSystem& vFrameActionSystem);
    void m_getAvailableDataBrokers();
    void m_clear();

public:  // singleton
    static std::shared_ptr<AccountPane> Instance() {
        static std::shared_ptr<AccountPane> _instance = std::make_shared<AccountPane>();
        return _instance;
    }

public:
    AccountPane();                              // Prevent construction
    AccountPane(const AccountPane&) = delete;  // Prevent construction by copying
    AccountPane& operator=(const AccountPane&) {
        return *this;
    };                       // Prevent assignment
    virtual ~AccountPane();  // Prevent unwanted destruction};
};
