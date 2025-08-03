// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Budget Analyzer for C, C++ and C#: http://www.viva64.com

#include <Panes/StatsPane.h>

#include <cinttypes>  // printf zu

#include <ezlibs/ezLog.hpp>

#include <Models/DataBase.h>

#include <Project/ProjectFile.h>
#include <Systems/SettingsDialog.h>

StatsPane::StatsPane() = default;
StatsPane::~StatsPane() {
    Unit();
}

bool StatsPane::Init() {
    return true;
}

void StatsPane::Unit() {
    m_Entities.clear();
    m_Categories.clear();
    m_Operations.clear();
}

///////////////////////////////////////////////////////////////////////////////////
//// IMGUI PANE ///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

bool StatsPane::DrawPanes(const uint32_t& /*vCurrentFrame*/, bool* vOpened, ImGuiContext* vContextPtr, void* /*vUserDatas*/) {
    ImGui::SetCurrentContext(vContextPtr);
    bool change = false;
    if (vOpened != nullptr && *vOpened) {
        static ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_MenuBar;
        if (ImGui::Begin(GetName().c_str(), vOpened, flags)) {
#ifdef USE_DECORATIONS_FOR_RESIZE_CHILD_WINDOWS
            auto win = ImGui::GetCurrentWindowRead();
            if (win->Viewport->Idx != 0)
                flags |= ImGuiWindowFlags_NoResize;  // | ImGuiWindowFlags_NoTitleBar;
            else
                flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_MenuBar;
#endif

            if (ProjectFile::Instance()->IsProjectLoaded()) {
                m_drawMenu();
                m_drawEntitiesStats();
                m_drawCategoriesStats();
                m_drawOperationsStats();
            }
        }

        ImGui::End();
    }
    return change;
}

bool StatsPane::DrawOverlays(const uint32_t& /*vCurrentFrame*/, const ImRect& /*vRect*/, ImGuiContext* vContextPtr, void* /*vUserDatas*/) {
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

bool StatsPane::DrawDialogsAndPopups(const uint32_t& /*vCurrentFrame*/, const ImRect& /*vRect*/, ImGuiContext* vContextPtr, void* /*vUserDatas*/) {
    ImGui::SetCurrentContext(vContextPtr);    
    return false;
}

bool StatsPane::DrawWidgets(const uint32_t& /*vCurrentFrame*/, ImGuiContext* vContextPtr, void* /*vUserDatas*/) {
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

void StatsPane::Load() {
    m_refreshDatas();
}

void StatsPane::m_refreshDatas() {
    m_updateAccounts();
    m_updateEntities();
    m_updateCategories();
    m_updateOperations();
}

void StatsPane::m_drawMenu() {
    if (ImGui::BeginMenuBar()) {

        ImGui::EndMenuBar();
    }
}

void StatsPane::m_updateAccounts() {
    m_Accounts.clear();
    DataBase::Instance()->GetAccounts(  //
        [this](const RowID& vRowID,
               const BankName& vBankName,
               const BankAgency& vBankAgency,
               const AccountType& vAccountType,
               const AccountName& vAccountName,
               const AccountNumber& vAccountNumber,
               const AccountBaseSolde& vBaseSolde,
               const TransactionsCount& vCount) {  //
            Account a;
            a.id = vRowID;
            a.bank = vBankName;
            a.agency = vBankAgency;
            a.type = vAccountType;
            a.name = vAccountName;
            a.number = vAccountNumber;
            a.base_solde = vBaseSolde;
            a.count = vCount;
            m_Accounts.push_back(a);
        });
}

void StatsPane::m_drawEntitiesStats() {
}

void StatsPane::m_updateEntities() {
    if (m_SelectedAccountIdx < m_Accounts.size()) {
        const auto account_id = m_Accounts.at(m_SelectedAccountIdx).id;
        m_Entities.clear();
        DataBase::Instance()->GetEntitiesStats(  //
            account_id,
            [this](
                const RowID& vRowID,
                const EntityName& vEntityName,
                const TransactionDebit& vTransactionDebit,
                const TransactionCredit& vTransactionCredit,
                const TransactionsCount& vTransactionsCount) {  //
                Entity e;
                e.id = vRowID;
                e.name = vEntityName;
                e.debit = vTransactionDebit;
                e.credit = vTransactionCredit;
                e.count = vTransactionsCount;
                m_Entities.push_back(e);
            });
    }
}

void StatsPane::m_drawCategoriesStats() {

}

void StatsPane::m_updateCategories() {
    m_Categories.clear();
    DataBase::Instance()->GetCategories(             //
        [/*this*/](const CategoryName& vCategoryName) {  //
       //     m_Categories.push_back(vCategoryName);
        });
}

void StatsPane::m_drawOperationsStats() {
}

void StatsPane::m_updateOperations() {
    m_Operations.clear();
    DataBase::Instance()->GetOperations(               //
        [/*this*/](const OperationName& vOperationName) {  //
         //   m_Operations.push_back(vOperationName);
        });
}
