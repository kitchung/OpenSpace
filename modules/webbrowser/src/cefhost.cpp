/*****************************************************************************************
 *                                                                                       *
 * OpenSpace                                                                             *
 *                                                                                       *
 * Copyright (c) 2014-2018                                                               *
 *                                                                                       *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this  *
 * software and associated documentation files (the "Software"), to deal in the Software *
 * without restriction, including without limitation the rights to use, copy, modify,    *
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to    *
 * permit persons to whom the Software is furnished to do so, subject to the following   *
 * conditions:                                                                           *
 *                                                                                       *
 * The above copyright notice and this permission notice shall be included in all copies *
 * or substantial portions of the Software.                                              *
 *                                                                                       *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,   *
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A         *
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT    *
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF  *
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE  *
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                         *
 ****************************************************************************************/

#include "include/cefhost.h"

namespace {
const char* _loggerCat = "CefHost";
}

namespace openspace {

CefHost::CefHost(std::string helperLocation) {
    LDEBUG("Initializing CEF...");
    CefMainArgs args;
    CefSettings settings;

    CefString(&settings.browser_subprocess_path).FromASCII(
        // This is bad as it casts away the const
        (char*) helperLocation.c_str()
    );
    attachDebugSettings(settings);

#ifdef WIN32
    // Enable High-DPI support on Windows 7 or newer.
    CefEnableHighDPISupport();
#endif
    CefRefPtr<WebBrowserApp> app(new WebBrowserApp);

    CefInitialize(args, settings, app.get(), NULL);
    initializeCallbacks();
    LDEBUG("Initializing CEF... done!");
}

CefHost::~CefHost() {
    CefShutdown();
}

void CefHost::attachDebugSettings(CefSettings &settings) {
    settings.remote_debugging_port = 8088;
    LDEBUG(fmt::format(
        "Remote WebBrowser debugging available on http://localhost:{}",
        settings.remote_debugging_port
    ));
//    settings.single_process = true;
}

void CefHost::initializeCallbacks() {
    OsEng.registerModuleCallback(
            OpenSpaceEngine::CallbackOption::Render,
            [this](){
                CefDoMessageLoopWork();
            }
    );
}

}

