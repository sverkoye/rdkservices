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
 
#include <glib.h>

#include "DACutils.h"
#include "DACinstallerImplementation.h"


const char* WPEFramework::Plugin::DACinstallerImplementation::STORE_NAME = "DACstorage";
const char* WPEFramework::Plugin::DACinstallerImplementation::STORE_KEY  = "4d4680a1-b3b0-471c-968b-39495d2b1cc3";

using namespace std;

namespace WPEFramework {
namespace Plugin {

// Events
#define DAC_EVT_INSTALL_ACK "DAC_InstallAck"

  DACinstallerImplementation::DACinstallerImplementation()
                                : mTaskNumber(0)
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
  uint32_t DACinstallerImplementation::Install_imp(const string& pkgId, const string& type, const string& url,
                                                   const string& token, const string& listener)
  { 

    // JsonObject params;
    // JsonObject result;
    // // params["jsonrpc"] = "2.0";
    // params["id"]        = "5";
    // params["task"]      = atoi(mTaskNumber++);
    // params["result"]    = result;
    // result["success"]   = true;

    // sendNotify(DAC_EVT_INSTALL_ACK, params);

    /*return*/ doInstall(pkgId, type,  url, token, listener); // THREAD IT

    // response["error"] = "params missing";

    // response["value"] = "test123";

    // returnResponse(true);

    return 0;
  }

  uint32_t DACinstallerImplementation::doInstall(const string& pkgId, const string& type, const string& url,
                                                 const string& token, const string& listener)
  {
    std::string install_app( "(empty))" );
    std::string install_url( url );

    // Parse the URL...
    //
    if(DACutils::fileEndsWith(url.c_str(), "json"))
    {
        LOGINFO(" %s() ... DOWNLOAD >>>  %s\n", __PRETTY_FUNCTION__, install_url.c_str());

        // Download JSON manifest...
        //
        if(DACutils::downloadJSON(url.c_str()) != DACutils::DACrc_t::dac_OK)
        {
            LOGERR(" %s() ... ERROR:  Failed to download JSON >> %s \n", __PRETTY_FUNCTION__, url.c_str());
            return -1; // FAIL  //DACutils::DACrc_t::dac_FAIL;
        }

        install_url = DACutils::mPackageCfg["install"].String();
        install_app = DACutils::mPackageCfg["name"].String();
    }
  
    // Download TGZ package...
    //
    LOGINFO(" %s() ... DOWNLOAD >>>  %s\n", __PRETTY_FUNCTION__, install_url.c_str());

    if(DACutils::downloadURL(install_url.c_str()) != DACutils::DACrc_t::dac_OK)
    {
        LOGERR(" %s() ... DOWNLOAD >>>  FAILED\n", __PRETTY_FUNCTION__);
        return -1; // FAIL  //DACutils::DACrc_t::dac_FAIL;
    }
    else
    {
        char uuid_path[PATH_MAX];

        // Get UUID
        std::string uuid_str = DACutils::getGUID();

        // Create path ... APPS_ROOT / {UUID} / {app}
        snprintf(uuid_path, PATH_MAX, "%s/%s/", APPS_ROOT, uuid_str.c_str());
        
        if(DACutils::extract(TMP_FILENAME, uuid_path) != DACutils::DACrc_t::dac_OK)
        {
            // Clean up failed extraction
            //
            LOGERR(" %s() ... EXTRACT >>>  FAILED\n", __PRETTY_FUNCTION__);

            DACutils::removeFolder(uuid_path); // remove debris

            return -1; // FAIL  //DACutils::DACrc_t::dac_FAIL;
        }

        LOGERR(" %s() ... INSTALLED >>> [ %s ]\n", __PRETTY_FUNCTION__, install_app.c_str());

        // Always cleanup
        DACutils::fileRemove(TMP_FILENAME);
    }

    return 0;
  }

  uint32_t DACinstallerImplementation::Remove_imp( const string& pkgId, const string& listener)
  {
    // fprintf(stderr, "\nHUGH >>>>> Call ... DAC::Remove_imp() ... pkgId: '%s'  listener: '%s' ", pkgId.c_str(), listener.c_str() ); 

    return 0;
  }

  uint32_t DACinstallerImplementation::Cancel_imp( const string& task, const string& listener)
  {
    DDD();
    return 0;
  }

  uint32_t DACinstallerImplementation::IsInstalled_imp(const string& pkgId)//, JsonObject &response)
  {
    DDD();

    fprintf(stderr, "\nHUGH >>>>> Call ... DAC::IsInstalled_imp() ... %s", pkgId.c_str() ); 

    return 0;
  }

  uint32_t DACinstallerImplementation::GetInstallProgress_imp(const string& task)
  {
    DDD();
    return 31;
  }

  using PackageInfoEx = DACinstallerImplementation::PackageInfoEx;

  PackageInfoEx::IIterator* DACinstallerImplementation::GetInstalled_imp()
  {
    DDD();
    return nullptr;
  }

  PackageInfoEx*  DACinstallerImplementation::GetPackageInfo_imp(const string& pkgId)
  {
    DDD();

    mInfo = Core::Service<PackageInfoEx>::Create<PackageInfoEx>("myName", "myVersion", "myID");

    fprintf(stderr, "########## info->name = %s", mInfo->Name().c_str());

    mInfo->setName("Test Name");
    mInfo->setBundlePath("/opt/foo/bar/myAppFolder");
    mInfo->setVersion("1.2.3");
    mInfo->setInstalled("Fri Jul 31 16:54:41 UTC 2020");
    mInfo->setSizeInBytes(123456);
    mInfo->setType("DAC");

    return mInfo;
  }

  uint32_t DACinstallerImplementation::GetAvailableSpace_imp()
  {
    DDD();
    return 0;
  }

  }  // namespace Plugin
}  // namespace WPEFramework