/**
 * If not stated otherwise in this file or this component's LICENSE
 * file the following copyright and licenses apply:
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
 **/

#pragma once

#include "tptimer.h"
#include "Module.h"
#include "utils.h"
#include "AbstractPlugin.h"
#include "RtNotifier.h"
#include "libIBus.h"
#include "libIBusDaemon.h"
#include "pwrMgr.h"

namespace WPEFramework {

namespace Plugin {
// This is a server for a JSONRPC communication channel.
// For a plugin to be capable to handle JSONRPC, inherit from PluginHost::JSONRPC.
// By inheriting from this class, the plugin realizes the interface PluginHost::IDispatcher.
// This realization of this interface implements, by default, the following methods on this plugin
// - exists
// - register
// - unregister
// Any other methood to be handled by this plugin  can be added can be added by using the
// templated methods Register on the PluginHost::JSONRPC class.
// As the registration/unregistration of notifications is realized by the class PluginHost::JSONRPC,
// this class exposes a public method called, Notify(), using this methods, all subscribed clients
// will receive a JSONRPC message as a notification, in case this method is called.
class XCast : public AbstractPlugin, public RtNotifier {
private:
    
    // We do not allow this plugin to be copied !!
    XCast(const XCast&) = delete;
    XCast& operator=(const XCast&) = delete;
    
    //Begin methods
    uint32_t getApiVersionNumber(const JsonObject& parameters, JsonObject& response);
    uint32_t applicationStateChanged(const JsonObject& parameters, JsonObject& response);
    uint32_t setEnabled(const JsonObject& parameters, JsonObject& response);
    uint32_t getEnabled(const JsonObject& parameters, JsonObject& response);
    //End methods
    
    //Begin events
    
    //End events
public:
    XCast();
    virtual ~XCast();
    //Build QueryInterface implementation, specifying all possible interfaces to be returned.
    BEGIN_INTERFACE_MAP(XCast)
    INTERFACE_ENTRY(PluginHost::IPlugin)
    INTERFACE_ENTRY(PluginHost::IDispatcher)
    END_INTERFACE_MAP
    //IPlugin methods
    virtual const string Initialize(PluginHost::IShell* service) override;
    virtual void Deinitialize(PluginHost::IShell* service) override;
    virtual string Information() const override;
    
    virtual void onRtServiceDisconnected(void) override;
    virtual void onXcastApplicationLaunchRequest(string appName, string parameter) override;
    virtual void onXcastApplicationStopRequest(string appName, string appID) override;
    virtual void onXcastApplicationHideRequest(string appName, string appID) override;
    virtual void onXcastApplicationResumeRequest(string appName, string appID) override;
    virtual void onXcastApplicationStateRequest(string appName, string appID) override;
private:
    /**
     * Whether Cast service is enabled by RFC
     */
    static bool isCastEnabled;
    static bool m_xcastEnableSettings;
    static IARM_Bus_PWRMgr_PowerState_t m_powerState;
    uint32_t m_apiVersionNumber;
    //Timer related variables and functions
    TpTimer m_locateCastTimer;
    const void InitializeIARM();
    void DeinitializeIARM();
    void persistEnabledSettings(bool enableStatus);
    //Internal methods
    void onLocateCastTimer();
    
    /**
     * Check whether the xdial service is allowed in this device.
     */
    static bool checkRFCServiceStatus();
    static bool checkXcastSettingsStatus();
    static void powerModeChange(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
};
} // namespace Plugin
} // namespace WPEFramework
