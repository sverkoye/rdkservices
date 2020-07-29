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


const char* WPEFramework::Plugin::DACinstallerImplementation::STORE_NAME = "DACstorage";
const char* WPEFramework::Plugin::DACinstallerImplementation::STORE_KEY  = "4d4680a1-b3b0-471c-968b-39495d2b1cc3";

// using namespace Utils;
using namespace std;

namespace WPEFramework {
namespace Plugin {


  DACinstallerImplementation::DACinstallerImplementation()
  {
    DDD();

    auto path = g_build_filename("/opt", "persistent", nullptr);

    if (!DACutils::fileExists(path))
    {
        g_mkdir_with_parents(path, 0745);
    }

    auto file = g_build_filename(path, STORE_NAME, nullptr);


    bool success = DACutils::init(file, STORE_KEY);
    fprintf(stderr, "\n %s() ... SQLite >> Init()  %s ", __PRETTY_FUNCTION__, (success ? " OK" : " FAILED !"));


    if(success)
    {
      DACutils::setValue("MyNamespace", "DACroot", "/opt/persistent/DACroot");
      DACutils::setValue("MyNamespace", "MyKey", "123");

      string value;

      success = DACutils::getValue("MyNamespace", "MyKey", value);
      fprintf(stderr, "\n %s() ... getValue()  %s    ... value: %s", __PRETTY_FUNCTION__, (success ? " OK" : " FAILED !"), value.c_str() );

      //DACutils::extract("/opt/persistent/test.tgz");

     DACutils::setupThreadQ(); // start thread Q 

      // DACutils::installURL("http://10.0.2.15/test.tgz");
    }

    g_free(path);
    g_free(file);
  }

  DACinstallerImplementation::~DACinstallerImplementation()
  {
    DDD();

    DACutils::term();
  }


  // DAC Installer API
  uint32_t DACinstallerImplementation::Install_imp(const string& pkgId, const string& type, const string& url,const string& token, const string& listener)
  { 
    DDD(); 

    fprintf(stderr, "\nHUGH >>>>> Call ... DAC::Install_imp()"); 

    DACutils::installURL(url.c_str()); // "http://10.0.2.15/test.tgz");

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