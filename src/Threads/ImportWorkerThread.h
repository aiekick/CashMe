#pragma once

#include <imguipack.h>
#include <apis/CashMePluginApi.h>

#include <mutex>
#include <atomic>
#include <thread>
#include <string>
#include <vector>
#include <functional>

class ImportWorkerThread {
private:
    static std::mutex s_Mutex;
    static std::atomic<float> a_Progress;  // [0.0:1.0]
    static std::atomic<bool> a_Working;
    static std::atomic<float> a_GenerationTime;  // secondes
    static std::atomic<uint32_t> a_ErrorsCount;  // count

private:
    std::thread m_WorkerThread;
    std::function<void()> m_FinishFunc;
    std::function<void()> m_CancelFunc;
    const char* m_Title = nullptr;
    uint32_t m_CountFiles = 0U;

    // thread shared data (need a mutex lock)
    std::string m_CurrentParsedFile;
    std::string m_CurrentPhase;

public:
    void start(              //
        const char* vTitle,  //
        Cash::BankStatementModuleWeak vBrocker,
        std::vector<std::string> vFiles,
        std::function<void()> vFinishFunc = nullptr,
        std::function<void()> vCancelFunc = nullptr);
    bool stop();
    bool cancel();
    bool isJoinable();
    void join();
    void finishIfNeeded();
    void drawDialog(const ImVec2& vPos);

private:
    void m_setCurrentParsedFile(const std::string& vFile);
    void m_setCurrentPhase(const std::string& vPhase);
    void m_drawPhase();
    void m_drawProgressBar();
    void m_worker(  //
        std::atomic<float>& vProgress,
        std::atomic<bool>& vWorking,
        std::atomic<float>& vGenerationTime,
        std::atomic<uint32_t>& vErrorsCount,
        Cash::BankStatementModuleWeak vBrocker,
        std::vector<std::string> vFiles);
};
