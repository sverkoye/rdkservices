/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2020 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "DACApplication.h"
#include <core/core.h>
#include <websocket/websocket.h>

using namespace std;
using namespace WPEFramework;

namespace WPEFramework {
namespace Plugin {

    SERVICE_REGISTRATION(DACApplication, 1, 0);

    const string DACApplication::Initialize(PluginHost::IShell* service)
    {
        ASSERT (_service == nullptr);
        ASSERT (service != nullptr);
        _service = service;
        _service->Register(_notification);

        _config.FromString(service->ConfigLine());
        SYSLOG(Trace::Information, (_T("DISPLAY = %s"), _config.ClientIdentifier.Value().c_str()));
        return (string());
    }

    void DACApplication::StartContainer()
    {
        string display = _config.ClientIdentifier.Value();

        if (display.empty())
        {
		SYSLOG(Trace::Error, (_T("DISPLAY name is empty")));
		display = "wst-"+_service->Callsign();
		SYSLOG(Trace::Error, (_T("Guessing DISPLAY name is %s"), display.c_str()));
        }

        JsonObject result;
        JsonObject param;
        Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), (_T("127.0.0.1:9998")));
        auto remoteObject = new WPEFramework::JSONRPC::LinkType<Core::JSON::IElement>(_T("org.rdk.OCIContainer.1"), _T(""));
        param["containerId"] = _service->Callsign();
        param["bundlePath"] = _dacappBundlePath;
        param["westerosSocket"] = std::string("/run/") + display;
        string jsonstring;
        param.ToString(jsonstring);
        SYSLOG(Trace::Information, (_T("container START PARAMS = %s"), jsonstring.c_str()));
        remoteObject->Invoke<JsonObject, JsonObject>(1000, _T("startContainer"), param, result);
    }

    void DACApplication::StopContainer()
    {
        JsonObject result;
        JsonObject param;
        Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), (_T("127.0.0.1:9998")));
        auto remoteObject = new WPEFramework::JSONRPC::LinkType<Core::JSON::IElement>(_T("org.rdk.OCIContainer.1"), _T(""));
        param["containerId"] = _service->Callsign();
        string jsonstring;
        param.ToString(jsonstring);
        SYSLOG(Trace::Information, (_T("container STOP PARAMS = %s"), jsonstring.c_str()));
        remoteObject->Invoke<JsonObject, JsonObject>(1000, _T("stopContainer"), param, result);
    }

    void DACApplication::Deinitialize(PluginHost::IShell* service)
    {
        ASSERT(_service == service);
        StopContainer();
        _service->Unregister(_notification);
        _service = nullptr;
    }

    string DACApplication::Information() const
    {
        return (string());
    }
}
}
