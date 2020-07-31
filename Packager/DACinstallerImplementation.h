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
#include "utils.h"

#include <stdio.h>
#include <stdint.h>
#include <string>

using namespace std;

//#define DACLOG()   fprintf(stderr, "\n %s()", __PRETTY_FUNCTION__); 
// #define DACLOG()   printf

#define DDD()   fprintf(stderr, "\nHUGH >>>>> DAC Impl ... Call ... %s()", __FUNCTION__); 

namespace WPEFramework {
namespace Plugin {

    class DACinstallerImplementation
    {
    public:
        DACinstallerImplementation(const DACinstallerImplementation&) = delete;
        DACinstallerImplementation& operator=(const DACinstallerImplementation&) = delete;

        DACinstallerImplementation();
        ~DACinstallerImplementation();

        // DAC Installer API
        uint32_t Install_imp(const string& pkgId, const string& type, const string& url, const string& token, const string& listener);
        uint32_t Remove_imp( const string& pkgId, const string& listener);
        uint32_t Cancel_imp( const string& task,  const string& listener);

        uint32_t IsInstalled_imp(const string& pkgId);
        uint32_t GetInstallProgress_imp(const string& task);
        uint32_t GetInstalled_imp();
        uint32_t GetPackageInfo_imp(const string& pkgId);
        uint32_t GetAvailableSpace_imp();

        static const char* STORE_NAME;
        static const char* STORE_KEY;

    private:
        uint32_t doInstall(const string& pkgId, const string& type, const string& url,const string& token, const string& listener);
  
    };

  } // namespace Plugin
}  // namespace WPEFramework
