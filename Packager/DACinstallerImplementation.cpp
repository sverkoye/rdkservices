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
 
// #include <sqlite3.h>
#include <glib.h>

#include "DACutils.h"
#include "DACinstallerImplementation.h"

// #ifndef SQLITE_FILE_HEADER
// #define SQLITE_FILE_HEADER "SQLite format 3"
// #endif

// #define SQLITE *(sqlite3**) &mData

const char* WPEFramework::Plugin::DACinstallerImplementation::STORE_NAME = "DACstorage";
const char* WPEFramework::Plugin::DACinstallerImplementation::STORE_KEY  = "4d4680a1-b3b0-471c-968b-39495d2b1cc3";

// using namespace Utils;
using namespace std;

// namespace {
// #if defined(SQLITE_HAS_CODEC)
//     bool fileEncrypted(const char* f)
//     {
//         FILE* fd = fopen(f, "rb");
//         if (!fd)
//         {
//             return false;
//         }

//         int    magicSize = strlen(SQLITE_FILE_HEADER);
//         char* fileHeader = (char*)malloc(magicSize + 1);
//         int     readSize = (int)fread(fileHeader, 1, magicSize, fd);

//         fclose(fd);

//         bool eq = magicSize == readSize && ::memcmp(fileHeader, SQLITE_FILE_HEADER, magicSize) == 0;
//         free(fileHeader);

//         return !eq;
//     }
// #endif

//     bool fileRemove(const char* f)
//     {
//         return (remove (f) == 0);
//     }

//     bool fileExists(const char* f)
//     {
//         return g_file_test(f, G_FILE_TEST_EXISTS);
//     }
// }

namespace WPEFramework {
namespace Plugin {


  DACinstallerImplementation::DACinstallerImplementation()
  {
    DDD();

    auto path = g_build_filename("opt", "persistent", nullptr);

    if (!DACutils::fileExists(path))
    {
        g_mkdir_with_parents(path, 0745);
    }

    auto file = g_build_filename(path, STORE_NAME, nullptr);

    bool success = DACutils::init(file, STORE_KEY);

    fprintf(stderr, "\n %s() ... SQLite >> Init()  %s ", __PRETTY_FUNCTION__, (success ? " OK" : " FAILED !"));

    g_free(path);
    g_free(file);
  }

  DACinstallerImplementation::~DACinstallerImplementation()
  {
    DDD();
  }


  // DAC Installer API
  uint32_t DACinstallerImplementation::Install_imp(const string& pkgId, const string& type, const string& url,const string& token, const string& listener)
  { 
    DDD(); 

    fprintf(stderr, "\nHUGH >>>>> Call ... DAC::Install_imp()"); 

    return 0;
  }

  uint32_t DACinstallerImplementation::Remove_imp( const string& pkgId, const string& listener)
  {
    DDD();

    fprintf(stderr, "\nHUGH >>>>> Call ... DAC::Remove_imp() ... pkgId: '%s'  listener: '%s' ", pkgId.c_str(), listener.c_str() ); 

    return 0;
  }

  uint32_t DACinstallerImplementation::Cancel_imp( const string& task, const string& listener)
  {
    DDD();
    return 0;
  }

  uint32_t DACinstallerImplementation::IsInstalled_imp(const string& pkgId)
  {
    DDD();

    fprintf(stderr, "\nHUGH >>>>> Call ... DAC::IsInstalled_imp() ... %s", pkgId.c_str() ); 

    return 0;
  }

  uint32_t DACinstallerImplementation::GetInstallProgress_imp(const string& task)
  {
    DDD();
    return 0;
  }

  uint32_t DACinstallerImplementation::GetInstalled_imp()
  {
    DDD();
    return 0;
  }

  uint32_t DACinstallerImplementation::GetPackageInfo_imp(const string& pkgId)
  {
    DDD();
    return 0;
  }

  uint32_t DACinstallerImplementation::GetAvailableSpace_imp()
  {
    DDD();
    return 0;
  }

  }  // namespace Plugin
}  // namespace WPEFramework