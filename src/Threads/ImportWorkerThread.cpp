#include <Threads/ImportWorkerThread.h>
#include <Models/DataBase.h>
#include <ezlibs/ezLog.hpp>
#include <ezlibs/ezTime.hpp>

#include <chrono>

#define DIALOG_WIDTH_ALIGN 80.0f
#define DIALOG_WIDTH_ITEM 300.0f

std::mutex ImportWorkerThread::s_Mutex;
std::atomic<float> ImportWorkerThread::a_Progress(0.0f);
std::atomic<bool> ImportWorkerThread::a_Working(false);
std::atomic<float> ImportWorkerThread::a_GenerationTime(0.0f);
std::atomic<uint32_t> ImportWorkerThread::a_ErrorsCount(0U);

void ImportWorkerThread::start(  //
    const char* vTitle,          //
    Cash::BankStatementModuleWeak vBrocker,
    std::vector<std::string> vFiles,
    std::function<void()> vFinishFunc,
    std::function<void()> vCancelFunc) {
    if (!stop()) {
        m_CountFiles = static_cast<uint32_t>(vFiles.size());
        m_Title = vTitle;
        m_FinishFunc = vFinishFunc;
        m_CancelFunc = vCancelFunc;
        m_WorkerThread = std::thread(  //
            &ImportWorkerThread::m_worker,
            this,
            std::ref(a_Progress),
            std::ref(a_Working),
            std::ref(a_GenerationTime),
            std::ref(a_ErrorsCount),
            vBrocker,
            vFiles);
    }
}

bool ImportWorkerThread::stop() {
    bool res = false;
    res = isJoinable();
    if (res) {
        a_Working = false;  // must be done before join
        join();
        if (m_FinishFunc != nullptr) {
            m_FinishFunc();
        }
    }
    return res;
}

bool ImportWorkerThread::cancel() {
    bool res = false;
    res = isJoinable();
    if (res) {
        a_Working = false;  // must be done before join
        join();
        if (m_CancelFunc != nullptr) {
            m_CancelFunc();
        }
    }
    return res;
}

bool ImportWorkerThread::isJoinable() {
    return m_WorkerThread.joinable();
}

void ImportWorkerThread::join() {
    m_WorkerThread.join();
}

void ImportWorkerThread::finishIfNeeded() {
    if (isJoinable() && a_Working == false) {
        join();
        if (m_FinishFunc != nullptr) {
            m_FinishFunc();
        }
    }
}

void ImportWorkerThread::drawDialog(const ImVec2& vPos) {
    if (a_Working == true) {
        ImGui::OpenPopup(m_Title);
        ImGui::SetNextWindowPos(vPos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        if (ImGui::BeginPopupModal(                      //
                m_Title,                                 //
                (bool*)nullptr,                          //
                ImGuiWindowFlags_NoTitleBar |            //
                    ImGuiWindowFlags_NoResize |          //
                    ImGuiWindowFlags_AlwaysAutoResize |  //
                    ImGuiWindowFlags_NoDocking)) {
            ImGui::Header(m_Title);
            ImGui::Separator();
            m_drawPhase();
            m_drawProgressBar();
            ImGui::Separator();
            if (ImGui::ContrastedButton("Stop")) {
                cancel();
            }
            ImGui::EndPopup();
        }
    }
}

void ImportWorkerThread::m_setCurrentParsedFile(const std::string& vFile) {
    s_Mutex.lock();
    m_CurrentParsedFile = vFile;
    s_Mutex.unlock();
}

void ImportWorkerThread::m_setCurrentPhase(const std::string& vPhase) {
    s_Mutex.lock();
    m_CurrentPhase = vPhase;
    s_Mutex.unlock();
}

void ImportWorkerThread::m_drawPhase() {
    s_Mutex.lock();
    if (!m_CurrentParsedFile.empty()) {
        ImGui::DisplayAlignedWidget(DIALOG_WIDTH_ITEM, "Current file", DIALOG_WIDTH_ALIGN, [this]() {  //
            ImGui::TextWrapped("%s", m_CurrentParsedFile.c_str());
        });
    }
    ImGui::DisplayAlignedWidget(DIALOG_WIDTH_ITEM, "Phase", DIALOG_WIDTH_ALIGN, [this]() {  //
        ImGui::Text("%s", m_CurrentPhase.c_str());
    });
    s_Mutex.unlock();
    if (a_ErrorsCount > 0) {
        ImGui::DisplayAlignedWidget(DIALOG_WIDTH_ITEM, "Errors", DIALOG_WIDTH_ALIGN, [this]() {  //
            const uint32_t errors = a_ErrorsCount;
            ImGui::Text("%u/%u", errors, m_CountFiles);
        });
    }
}

void ImportWorkerThread::m_drawProgressBar() {
    ImGui::DisplayAlignedWidget(DIALOG_WIDTH_ITEM, "Progres", DIALOG_WIDTH_ALIGN, [/*this*/]() {  //
        float progress = a_Progress;
        float elapsed_time = a_GenerationTime;
        const char* text;
        ImFormatStringToTempBuffer(&text, nullptr, "%.3f s", elapsed_time);
        ImGui::ProgressBar(progress, ImVec2(300.0f, 0.0f), text);
    });
}

void ImportWorkerThread::m_worker(  //
    std::atomic<float>& vProgress,
    std::atomic<bool>& vWorking,
    std::atomic<float>& vGenerationTime,
    std::atomic<uint32_t>& vErrorsCount,
    Cash::BankStatementModuleWeak vBrocker,
    std::vector<std::string> vFiles) {
    vProgress = 0.0f;
    vGenerationTime = 0.0f;
    vWorking = true;
    const auto mt0 = ez::time::getTicks();
    auto ptr = vBrocker.lock();
    if (ptr != nullptr) {
        if (!vFiles.empty()) {
            // N Files for Parsing
            // + N Files for DB writing
            // + 1 Transaction Commit
            // => N Files * 2U + 1
            const float progress_steps = 1.0f / static_cast<float>(vFiles.size() * 2U + 1U);
            std::vector<Cash::AccountTransactions> stmts;
            // 1) parsing
            if (vWorking) {
                m_setCurrentPhase("Parsing");
                for (const auto& file : vFiles) {
                    m_setCurrentParsedFile(file);
                    const auto& stmt = ptr->importBankStatement(file);
                    if (!stmt.statements.empty()) {
                        stmts.push_back(stmt);
                    }
                    vGenerationTime = (ez::time::getTicks() - mt0) / 1000.0f;
                    vProgress = vProgress + progress_steps;
                    if (!vWorking) {
                        break;
                    }
                }
            }
            m_setCurrentParsedFile({});  // clear current file
            // 2) database writing
            if (vWorking && !stmts.empty()) {
                m_setCurrentPhase("DataBase Transaction Insertion");
                if (DataBase::ref().BeginDBTransaction()) {
                    for (const auto& stmt : stmts) {
                        RowID account_id = 0U;
                        if (DataBase::ref().GetAccount(stmt.account.number, account_id)) {
                            for (const auto& stm : stmt.statements) {
                                TransactionInput ti;
                                ti.entity.name = stm.entity;
                                ti.category.name = stm.category;
                                ti.operation.name = stm.operation;
                                ti.source.name = stm.source;
                                ti.source.type = stm.source_type;
                                ti.source.sha = stm.source_sha1;
                                ti.date = stm.date;
                                ti.description = stm.description;
                                ti.comment = stm.comment;
                                ti.amount = stm.amount;
                                ti.confirmed = stm.confirmed;
                                ti.sha = stm.sha;
                                DataBase::ref().AddTransaction(account_id, ti);
                                vGenerationTime = (ez::time::getTicks() - mt0) / 1000.0f;
                                if (!vWorking) {
                                    break;
                                }
                            }
                        } else {
                            LogVarError(                                                        //
                                "Import interrupted for the file %s, no account found for %s",  //
                                stmt.source.c_str(),                                            //
                                stmt.account.number.c_str());
                            ++vErrorsCount;
                        }
                        vProgress = vProgress + progress_steps;
                        if (!vWorking) {
                            break;
                        }
                    }
                    m_setCurrentPhase("DataBase Transaction Commit");
                    DataBase::ref().CommitDBTransaction();
                }
            }
        }
    }
    // For the thread can be joined, the flow must quit the function
    // The vWorking atomic control boolean is the way for quit during the run
    // in a normal case (no user stop), the join will be called only if the atomic vWorking is false
    vWorking = false;
}
