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

#include <interfaces/IPackager.h>
#include <interfaces/json/JsonData_StateControl.h>

#include "utils.h"

namespace WPEFramework {
namespace Plugin {
namespace {
    constexpr auto* kInstallMethodName = _T("install");
    constexpr auto* kSynchronizeMethodName = _T("synchronize");


//    constexpr auto* kDAC_InstallMethodName            = _T("install");    NOT USED 
    constexpr auto* kDAC_RemoveMethodName             = _T("remove");
    constexpr auto* kDAC_CancelMethodName             = _T("cancel");
    constexpr auto* kDAC_IsInstalledMethodName        = _T("isInstalled");
    constexpr auto* kDAC_GetInstallProgressMethodName = _T("getInstallProgress");
    constexpr auto* kDAC_GetInstalledMethodName       = _T("getInstalled");
    constexpr auto* kDAC_GetPackageInfoMethodName     = _T("getPackageInfo");
    constexpr auto* kDAC_GetAvailableSpaceMethodName  = _T("getAvailableSpace");
}

    class Packager : public PluginHost::IPlugin, 
                     public PluginHost::IWeb, 
                     public PluginHost::JSONRPC
    {
    public:
        struct Params : public Core::JSON::Container
        {
            Params& operator=(const Params& other) = delete;
            Params() {
                Add(_T("package"), &Package);
                Add(_T("architecture"), &Architecture);
                Add(_T("version"), &Version);

                Add(_T("task"), &Task);
                Add(_T("pkgId"), &PkgId);
                Add(_T("type"), &Type);
                Add(_T("url"), &Url);
                Add(_T("token"), &Token);
                Add(_T("listener"), &Listener);
            }
            Params(const Params& other)
                : Package(other.Package)
                , Architecture(other.Architecture)
                , Version(other.Version)

                , Task(other.Task)

                , PkgId(other.PkgId)
                , Type(other.Type)
                , Url(other.Url)
                , Token(other.Token)
                , Listener(other.Listener)
            {
                Add(_T("package"), &Package);
                Add(_T("architecture"), &Architecture);
                Add(_T("version"), &Version);

                Add(_T("task"), &Task);

                Add(_T("pkgId"), &PkgId);
                Add(_T("type"), &Type);
                Add(_T("url"), &Url);
                Add(_T("token"), &Token);
                Add(_T("listener"), &Listener);
            }
            Core::JSON::String Package;
            Core::JSON::String Architecture;
            Core::JSON::String Version;

            Core::JSON::String Task;

            Core::JSON::String PkgId;
            Core::JSON::String Type;
            Core::JSON::String Url;
            Core::JSON::String Token;
            Core::JSON::String Listener;
        }; // STRUCT     

        // We do not allow this plugin to be copied !!
        Packager(const Packager&) = delete;
        Packager& operator=(const Packager&) = delete;

        Packager()
            : _skipURL(0)
            , _connectionId(0)
            , _service(nullptr)
            , _implementation(nullptr)
            , _notification(this)
        {
            // Packager API
            Register<Params, JsonObject>(kInstallMethodName, [this](const Params& params, JsonObject response) -> uint32_t
            {
                if( ( params.PkgId.IsSet() && params.PkgId.Value().empty() == false ) &&
                    ( params.Type.IsSet()  && params.Type.Value().empty()  == false ) &&
                    ( params.Url.IsSet()   && params.Url.Value().empty()   == false ) )
                {
                    // ---- DAC Installer API ----
                    //
                    // DAC::Install()
                    //
                    this->_implementation->Install(params.PkgId.Value(), params.Type.Value(), 
                                                   params.Url.Value(),   params.Token.Value(),
                                                   params.Listener.Value()); 

                    // char taskId[255];

                    // sprintf(taskId, 255, "%d", this->_implementation->getNextTaskID() );

                    // string foo(this->_implementation->getNextTaskID());

                    // response["task"]   = string( taskId );
                    // response["result"] = true;

                    return 0;
                }
                else // default to Packager API
                {
                    // Packager
                    return this->_implementation->Install(params.Package.Value(), params.Version.Value(),
                                                           params.Architecture.Value());
                }
            });

            Register<void, void>(kSynchronizeMethodName, [this]() -> uint32_t
            {
                return this->_implementation->SynchronizeRepository();
            });

            // ---- DAC Installer API ----
            //
            // DAC::Remove()
            //
            Register<Params, void>(kDAC_RemoveMethodName, [this](const Params& params) -> uint32_t
            {                 
                return this->_implementation->Remove(params.PkgId.Value(), params.Listener.Value());
            });
            //
            // DAC::Cancel()
            //
            Register<Params, void>(kDAC_CancelMethodName, [this](const Params& params) -> uint32_t
            {
                //sendNotify("MyDummy Event", JsonObject());

                return this->_implementation->Cancel(params.Task.Value(), params.Listener.Value());
            });
            //
            // DAC::IsInstalled()
            //            
            Register<Params, JsonObject>(kDAC_IsInstalledMethodName, [this](const Params& params, JsonObject& response) -> uint32_t
            {
                uint32_t result = Core::ERROR_NONE;

fprintf(stderr, "\n\npackager.h >>> IsInstalled_imp() ... pkgId: [%s]\n\n", params.PkgId.Value().c_str() ); 

                bool isInstalled = this->_implementation->IsInstalled(params.PkgId.Value());

                response["available"] = (isInstalled ? true : false);

                return result;
            });
            //
            // DAC::GetInstallProgress()
            //
            Register<Params, JsonObject>(kDAC_GetInstallProgressMethodName, [this](const Params& params, JsonObject& response) -> uint32_t
            {
                uint32_t result = Core::ERROR_NONE;

                uint32_t pc = this->_implementation->GetInstallProgress(params.Task.Value());
                
                fprintf(stderr, "\nHUGH >>>>> Call ... DAC::GetInstallProgress()   pc: [%d]", pc); 

                char str[255];
                snprintf(str, 255, "%d%%", pc);

                response["percentage"] =  string(str);

                return result;
            });

            auto PkgInfo2json = [](Exchange::IPackager::IPackageInfoEx *pkg, JsonObject& response)
            {
                if(pkg == nullptr)
                {
                    LOGERR("Packager::PkgInfo2json() - Bad args !!! ... pkg == NULL");
                    return -1; // ERROR
                }

                // TODO:  Use instead ??
                //
                // JsonObject json = PackageInfoEx::pkg2json( (PackageInfoEx*) pkg);

                char str[255];
                snprintf(str, 255, "%jd", pkg->SizeInBytes());

                response["name"]       = pkg->Name();
                response["bundlePath"] = pkg->BundlePath();
                response["version"]    = pkg->Version();
                response["id"]         = pkg->PkgId();
                response["installed"]  = pkg->Installed();
                response["size"]       = string(str);
                response["type"]       = pkg->Type();

                return 0;
            };

            //
            // DAC::GetInstalled()
            //
            Register<void, JsonObject>(kDAC_GetInstalledMethodName,  [this, PkgInfo2json](JsonObject& response) -> uint32_t
            {
                JsonArray list; // installed packages

                Exchange::IPackager::IPackageInfoEx::IIterator *iter = this->_implementation->GetInstalled();

                while (iter->Next() == true)
                {
                    Exchange::IPackager::IPackageInfoEx *pkg = (Exchange::IPackager::IPackageInfoEx *) iter;

                    //Exchange::IPackager::IPackageInfoEx *pkg = this->_implementation->GetPackageInfo("foo");

                    LOGERR("Packager::GetInstalled() - App: %s", pkg->Name().c_str());

                    if(pkg != nullptr)
                    {
                        JsonObject pkgJson;
                        PkgInfo2json(pkg, pkgJson);

                        list.Add( pkgJson );
                    }

                    if(pkg)
                    {
                        pkg->Release();
                    }
                }//WHILE
               
                response["applications"] = list;
                
                // sendNotify("MyDummy Event", JsonObject());

                return 0;
            });

            //
            // DAC::GetPackageInfo()
            //
            Register<Params, JsonObject>(kDAC_GetPackageInfoMethodName, [this, PkgInfo2json](const Params& params, JsonObject& response) -> uint32_t
            {
                Exchange::IPackager::IPackageInfoEx *pkg = this->_implementation->GetPackageInfo(params.PkgId.Value());

                LOGERR("Packager::GetPackageInfo() - App: %s", pkg->Name().c_str());

                JsonObject pkgJson;
                PkgInfo2json(pkg, response);

                if(pkg)
                {
                    pkg->Release();
                }

                return 0; 
            });
            //
            // DAC::GetAvailableSpace()
            //
            Register<void, JsonObject>(kDAC_GetAvailableSpaceMethodName,  [this](JsonObject& response) -> uint32_t
            {
                int64_t bytes = this->_implementation->GetAvailableSpace();

                char str[255];
                snprintf(str, 255, "%jd", bytes);

                response["availableSpaceInKB"] = string(str);

                return 0;
            });
        }

        ~Packager() override
        {
            Unregister(kInstallMethodName);
            Unregister(kSynchronizeMethodName);

            Unregister(kDAC_RemoveMethodName);
            Unregister(kDAC_CancelMethodName);
            Unregister(kDAC_IsInstalledMethodName);
            Unregister(kDAC_GetInstallProgressMethodName);
            Unregister(kDAC_GetInstalledMethodName);
            Unregister(kDAC_GetPackageInfoMethodName);
            Unregister(kDAC_GetAvailableSpaceMethodName);
        }

        BEGIN_INTERFACE_MAP(Packager)
            INTERFACE_ENTRY(PluginHost::IPlugin)
            INTERFACE_ENTRY(PluginHost::IWeb)
            INTERFACE_ENTRY(PluginHost::IDispatcher)
            INTERFACE_AGGREGATE(Exchange::IPackager, _implementation)
        END_INTERFACE_MAP

        //   IPlugin methods
        const string Initialize(PluginHost::IShell* service) override;
        void Deinitialize(PluginHost::IShell* service) override;
        string Information() const override;

        //	IWeb methods
        void Inbound(Web::Request& request) override;
        Core::ProxyType<Web::Response> Process(const Web::Request& request) override;

    private:
        class Notification : public RPC::IRemoteConnection::INotification,
                             public PluginHost::IStateControl::INotification,
                             public Exchange::IPackager::INotification
        {
        private:
            Notification() = delete;
            Notification(const Notification&) = delete;
            Notification& operator=(const Notification&) = delete;

        public:
            explicit Notification(Packager* parent)
                : _parent(*parent)
            {
                ASSERT(parent != nullptr);
            }

            ~Notification() override
            {
            }

        public:
            virtual void Activated(RPC::IRemoteConnection*) //override
            {
            }

            virtual void Deactivated(RPC::IRemoteConnection* connection) //override
            {
                _parent.Deactivated(connection);
            }

            virtual void StateChange(Exchange::IPackager::IPackageInfo* package, 
                                     Exchange::IPackager::IInstallationInfo* install) //override
            {
                fprintf(stderr, "\n ########\n ########  StateChange() !!! \n ########");
               // _parent.IntallStep(status);
            }

            virtual void RepositorySynchronize(uint32_t status) //override
            {
                fprintf(stderr, "\n ########\n ########  RepositorySynchronize() !!! status: %ul\n ########", status);
               // _parent.IntallStep(status);
            }

            virtual void IntallStep(uint32_t status) //override
            {
                fprintf(stderr, "\n ########\n ########  IntallStep() !!! status: %ul\n ########", status);
                _parent.IntallStep(status);
            }

            virtual void StateChange(const PluginHost::IStateControl::state state) //override
            {
                fprintf(stderr, "\n ########\n ########  StateChange() !!! state: %ul\n ########", state);
                // _parent.StateChange(state);
            }

            BEGIN_INTERFACE_MAP(Notification)
                INTERFACE_ENTRY(Exchange::IPackager::INotification)
                INTERFACE_ENTRY(PluginHost::IStateControl::INotification)
                INTERFACE_ENTRY(RPC::IRemoteConnection::INotification)                
            END_INTERFACE_MAP

        private:
            Packager& _parent;
        }; // CLASS - Notification

        void Deactivated(RPC::IRemoteConnection* connection);

        void IntallStep(uint32_t status);

        // JSONRPC
        void event_relayevent(std::string event);
        // void event_installstep(uint32_t status);


        uint8_t _skipURL;
        uint32_t _connectionId;
        PluginHost::IShell* _service;
        Exchange::IPackager* _implementation;
        Core::Sink<Notification> _notification;
    };

}  // namespace Plugin
}  // namespace WPEFramework
