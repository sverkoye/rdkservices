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

#pragma once

#include "Module.h"

namespace WPEFramework {
namespace Plugin {

class DACApplication : public PluginHost::IPlugin, PluginHost::JSONRPC {
public:
    DACApplication(const DACApplication&) = delete;
    DACApplication& operator=(const DACApplication&) = delete;

    DACApplication() : _notification(Core::Service<Notification>::Create<Notification>(this))
    {
        RegisterAll();
    }

    virtual ~DACApplication()
    {
        UnregisterAll();
    }

    BEGIN_INTERFACE_MAP(DACApplication)
       INTERFACE_ENTRY(PluginHost::IPlugin)
       INTERFACE_ENTRY(PluginHost::IDispatcher)
    END_INTERFACE_MAP

public:
    //   IPlugin methods
    // -------------------------------------------------------------------------------------------------------
    virtual const string Initialize(PluginHost::IShell* service) override;
    virtual void Deinitialize(PluginHost::IShell* service) override;
    virtual string Information() const override;

private:

    class DACConfig : public Core::JSON::Container {
    private:
        DACConfig(const DACConfig&) = delete;
        DACConfig& operator=(const DACConfig&) = delete;

    public:
        DACConfig()
            : Core::JSON::Container()
            , ClientIdentifier()
        {
            Add(_T("clientidentifier"), &ClientIdentifier);
        }
        ~DACConfig()
        {
        }

    public:
        Core::JSON::String ClientIdentifier;
    };

    class Notification : public PluginHost::IPlugin::INotification {
    public:
        Notification(DACApplication* parent)
            : _parent(*parent)
        {
            ASSERT(parent != nullptr);
        }
    private:
        Notification(const Notification&) = delete;
        Notification& operator=(const Notification&) = delete;

    public:
        virtual ~Notification() {}
        virtual void StateChange(PluginHost::IShell* service) final
        {
            PluginHost::IShell::state currentState(service->State());
            SYSLOG(Trace::Information, (_T("plugin STATE %s : %d"), service->Callsign().c_str(), currentState));
        }

        BEGIN_INTERFACE_MAP(Notification)
        INTERFACE_ENTRY(PluginHost::IPlugin::INotification)
        END_INTERFACE_MAP
    private:
        DACApplication& _parent;
    };

    string _dacappBundlePath;
    PluginHost::IShell* _service;
    Notification*  _notification;
    DACConfig _config;

    void StartContainer();
    void StopContainer();

    //   JSONRPC methods
    // -------------------------------------------------------------------------------------------------------
    void RegisterAll();
    void UnregisterAll();

    uint32_t get_url(Core::JSON::String& response) const;
    uint32_t set_url(const Core::JSON::String& param);
};

} // namespace Plugin
} // namespace WPEFramework
