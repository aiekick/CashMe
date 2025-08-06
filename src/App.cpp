// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "App.h"

#include <Headers/CashMeBuild.h>

#include <ezlibs/ezApp.hpp>
#include <ezlibs/ezFile.hpp>
#include <ezlibs/ezLog.hpp>

#include <Backend/MainBackend.h>
#include <Frontend/MainFrontend.h>

#include <Models/DataBase.h>

#include <imguipack.h>

#include <Panes/AccountsPane.h>
#include <Panes/BanksPane.h>
#include <Panes/BudgetPane.h>
#include <Panes/CategoriesPane.h>
#include <Panes/ConsolePane.h>
#include <Panes/EntitiesPane.h>
#include <Panes/IncomesPane.h>
#include <Panes/OperationsPane.h>
#include <Panes/TransactionsPane.h>

#include <Helpers/TranslationHelper.h>
#include <Systems/SettingsDialog.h>

// messaging
#define MESSAGING_CODE_INFOS 0
#define MESSAGING_LABEL_INFOS "Infos"
#define MESSAGING_CODE_WARNINGS 1
#define MESSAGING_LABEL_WARNINGS "Warnings"
#define MESSAGING_CODE_ERRORS 2
#define MESSAGING_CODE_DEBUG 3
#define MESSAGING_LABEL_ERRORS "Errors"
#define MESSAGING_LABEL_DEBUG "Debug"

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

bool App::init() {
    bool ret = true;
    ret &= (MainBackend::initSingleton() != nullptr);
    ret &= (LayoutManager::initSingleton() != nullptr);
    ret &= (Messaging::initSingleton() != nullptr);
    return true;
}

void App::unit() {
    Messaging::unitSingleton();
    LayoutManager::unitSingleton();
    MainBackend::unitSingleton();
}

int App::run(int argc, char** argv) {
    ez::App app(argc, argv);
    printf("-----------\n");
    printf("[[ %s Beta %s ]]\n", CashMe_Label, CashMe_BuildId);

#ifdef _DEBUG
    ez::file::createDirectoryIfNotExist("sqlite3");
#endif

    m_InitMessaging();

    MainBackend::ref().run(app);

    return 0;
}

void App::m_InitMessaging() {
    Messaging::ref().AddCategory(MESSAGING_CODE_INFOS, "Infos(s)", MESSAGING_LABEL_INFOS, ImVec4(0.0f, 0.8f, 0.0f, 1.0f));
    Messaging::ref().AddCategory(MESSAGING_CODE_WARNINGS, "Warnings(s)", MESSAGING_LABEL_WARNINGS, ImVec4(0.8f, 0.8f, 0.0f, 1.0f));
    Messaging::ref().AddCategory(MESSAGING_CODE_ERRORS, "Errors(s)", MESSAGING_LABEL_ERRORS, ImVec4(0.8f, 0.0f, 0.0f, 1.0f));
    Messaging::ref().AddCategory(MESSAGING_CODE_DEBUG, "Debug(s)", MESSAGING_LABEL_DEBUG, ImVec4(0.8f, 0.8f, 0.0f, 1.0f));
    Messaging::ref().SetLayoutManager(&LayoutManager::ref());
    ez::Log::ref().setStandardLogMessageFunctor([](const int& vType, const std::string& vMessage) {
        MessageData msg_datas;
        const auto& type = vType;
        Messaging::ref().AddMessage(vMessage, type, false, msg_datas, {});
    });
}