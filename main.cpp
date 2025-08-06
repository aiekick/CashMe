// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
Copyright 2022-2023 Stephane Cuillerdier (aka aiekick)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include <App.h>
#include <string>
#include <iostream>

#include <ezlibs/ezLog.hpp>
#include <ezlibs/ezTools.hpp>

//#define ENABLE_MEM_CHECK

// The following macros set and clear, respectively, given bits
// of the C runtime library debug flag, as specified by a bitmask.
#ifdef _DEBUG
#ifdef ENABLE_MEM_CHECK
#define SET_CRT_DEBUG_FIELD(a) _CrtSetDbgFlag((a) | _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG))
#define CLEAR_CRT_DEBUG_FIELD(a) _CrtSetDbgFlag(~(a) & _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG))
#endif 
#endif

int main(int argc, char** argv) {
    int res = EXIT_SUCCESS;

#ifdef _MSC_VER
#ifdef _DEBUG
#ifdef ENABLE_MEM_CHECK
    // active memory leak detector
    // https://stackoverflow.com/questions/4790564/finding-memory-leaks-in-a-c-application-with-visual-studio
    // Send all reports to STDOUT
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDOUT);
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDOUT);
    // Set the debug heap to report memory leaks when the process terminates,
    // and to keep freed blocks in the linked list.
    SET_CRT_DEBUG_FIELD(_CRTDBG_LEAK_CHECK_DF | _CRTDBG_DELAY_FREE_MEM_DF);
#endif
#endif
#endif

    {
        ez::Log::initSingleton();

        try {
            App app;
            if (app.init()) {
                app.run(argc, argv);
                app.unit();
            }
        } catch (const std::exception& e) {
            LogVarLightInfo("Exception %s", e.what());
            res = EXIT_FAILURE;
            EZ_TOOLS_DEBUG_BREAK;
        }

        ez::Log::ref().close();
        ez::Log::unitSingleton();
    }

#ifdef _MSC_VER
#ifdef _DEBUG
#ifdef ENABLE_MEM_CHECK
    _CrtCheckMemory();
    if (_CrtDumpMemoryLeaks() == TRUE) {
        EZ_TOOLS_DEBUG_BREAK;
    }
#endif
#endif
#endif

    return res;
}
