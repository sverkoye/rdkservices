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

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        class PackageInfoEx : public Exchange::IPackager::IPackageInfoEx
        {
          public:
            PackageInfoEx(const PackageInfoEx&) = delete;
            PackageInfoEx& operator=(const PackageInfoEx&) = delete;

            ~PackageInfoEx() override
            {
            }

            PackageInfoEx(const std::string& name,
                          const std::string& version,
                          const std::string& pkgId)
                            : _name(name)
                            , _version(version)
                            , _pkgId(pkgId)
            {
            }

            PackageInfoEx() = default;

            BEGIN_INTERFACE_MAP(PackageInfoEx)
                INTERFACE_ENTRY(Exchange::IPackager::IPackageInfoEx)
            END_INTERFACE_MAP

            string  Name()                const override { return _name;       };
            void setName( string v)             { _name = v;          };

            string  BundlePath()          const override { return _bundlePath; };
            void setBundlePath( string v)       { _bundlePath = v;    };

            string  Version()             const override { return _version;    };
            void setVersion( string v)          { _version = v;       };

            string  PkgId()               const override { return _pkgId;      };
            void setPkgId( string v)            { _pkgId = v;         };

            string  Installed()           const override { return _installed;  };
            void setInstalled( string v)        { _installed = v;     };

            uint32_t  SizeInBytes()         const override { return _sizeInBytes; };
            void setSizeInBytes( uint32_t v)      { _sizeInBytes = v;   };

            string  Type()                const override { return _type;       };
            void setType( string v)             { _type = v;          };
            
          private:
            string   _name;
            string   _bundlePath;
            string   _version;
            string   _pkgId;
            string   _installed;    // timestamp
            uint32_t _sizeInBytes;  // bytes
            string   _type;
        };// CLASS - PackageInfoEx
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

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
 
        //using PackageInfoEx = DACinstallerImplementation::PackageInfoEx; 

        using IPackageInfoEx = Exchange::IPackager::IPackageInfoEx;
        
        IPackageInfoEx::IIterator* GetInstalled_imp();
        
        PackageInfoEx* GetPackageInfo_imp(const string& pkgId);

        // uint32_t GetInstalled_imp();
        // uint32_t GetPackageInfo_imp(const string& pkgId);
        uint32_t GetAvailableSpace_imp();

        virtual JsonObject getInfo() { LOGERR(" getInfo GOOD"); return JsonObject(); };

        static const char* STORE_NAME;
        static const char* STORE_KEY;

    private:
        uint32_t doInstall(const string& pkgId, const string& type, const string& url,const string& token, const string& listener);
  
        uint32_t mTaskNumber;

        PackageInfoEx *mInfo;

    };//CLASS - DACinstallerImplementation

  } // namespace Plugin
}  // namespace WPEFramework
