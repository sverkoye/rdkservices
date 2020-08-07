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

#include "time.h"
#include <locale>

#include "utils.h"

#include "DACutils.h"
#include "DACinstallerImplementation.h"


#define MB_in_BYTES  1000000

const int64_t WPEFramework::Plugin::DACinstallerImplementation::STORE_BYTES_QUOTA = 10 * MB_in_BYTES;
const char*   WPEFramework::Plugin::DACinstallerImplementation::STORE_NAME        = "DACstorage";
const char*   WPEFramework::Plugin::DACinstallerImplementation::STORE_KEY         = "4d4680a1-b3b0-471c-968b-39495d2b1cc3";

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

#if 0  
      mInfo = Core::Service<PackageInfoEx>::Create<PackageInfoEx>("myName", "myVersion", "myID");

      mInfo->setName("Test Name");
      mInfo->setBundlePath("/opt/foo/bar/myAppFolder");
      mInfo->setVersion("1.2.3");
      mInfo->setPkgId("TestApp0123456");
      mInfo->setInstalled("Fri Jul 31 16:54:41 UTC 2020");
      mInfo->setSizeInBytes(123456);
      mInfo->setType("DAC");



fprintf(stderr, "########## addPkgRow \n");
  DACutils::addPkgRow(mInfo);  


fprintf(stderr, "########## hasPkgRow('TestApp0123456') == %s\n", 
      (DACutils::hasPkgRow( "TestApp0123456" ) ? "TRUE" : "FALSE") );



fprintf(stderr, "########## hasPkgRow('foo') == %s\n\n", 
      (DACutils::hasPkgRow( "foo" ) ? "TRUE" : "FALSE") );


fprintf(stderr, "########## showTable \n\n");
DACutils::showTable();


fprintf(stderr, "\n########## delPkgRow\n");
  DACutils::delPkgRow("TestApp0123456");


fprintf(stderr, "########## NOW ? hasPkgRow('TestApp0123456') == %s\n", 
      (DACutils::hasPkgRow( "TestApp0123456" ) ? "TRUE" : "FALSE") );

#endif //00


// fprintf(stderr, "\n########## getPkgRow\n");
// PackageInfoEx* pp = DACutils::getPkgRow("TestApp0123456");

// PackageInfoEx::printPkg(pp);

     // DACutils::setupThreadQ(); // start thread Q 

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
    std::string install_name( "(empty))" );
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

        install_url  = DACutils::mPackageCfg["install"].String();
        install_name = DACutils::mPackageCfg["name"].String();
    }
  
    // Download TGZ package...
    //
    LOGINFO(" %s() ... DOWNLOAD >>>  %s\n", __PRETTY_FUNCTION__, install_url.c_str());

    if(DACutils::downloadURL(install_url.c_str()) != DACutils::DACrc_t::dac_OK)
    {
        LOGERR(" %s() ... DOWNLOAD (%s)>>>  FAILED\n", __PRETTY_FUNCTION__, install_url.c_str());
        return -1; // FAIL
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

            return -1; // FAIL
        }

        LOGERR(" %s() ... INSTALLED >>> [ %s ]\n", __PRETTY_FUNCTION__, install_name.c_str());

        // Always cleanup
        DACutils::fileRemove(TMP_FILENAME);

        mInfo = Core::Service<PackageInfoEx>::Create<PackageInfoEx>();

        time_t rawtime;
        time (&rawtime);

        int32_t bytes = DACutils::folderSize(uuid_path);

        mInfo->setPkgId(pkgId);
        mInfo->setName(install_name);
        mInfo->setBundlePath(uuid_path);
        mInfo->setVersion("1.2.3");
        mInfo->setInstalled(ctime (&rawtime));
        mInfo->setSizeInBytes(bytes);
        mInfo->setType(type);
        
        DACutils::addPkgRow(mInfo);
    }

    return 0;
  }

  uint32_t DACinstallerImplementation::Remove_imp( const string& pkgId, const string& listener)
  {
    LOGINFO("... Remove_imp(%s, %s) - ENTER ", pkgId.c_str(), listener.c_str());

    PackageInfoEx* pkg = DACutils::getPkgRow(pkgId);

    if(pkg)
    {
      LOGERR(" removeFolder( %s ) ... NOT found", pkg->BundlePath().c_str());

      DACutils::removeFolder(pkg->BundlePath());

      bool rc = DACutils::delPkgRow(pkgId);

      if(rc == false)
      {
        LOGINFO("... Remove_imp(%s, %s) - FAILED... not found ? ", pkgId.c_str(), listener.c_str());
        return -1; // FAILED
      }
    }
    else
    {
      LOGERR(" .Remove_imp( %s ) ... NOT found", pkgId.c_str());
      return -1; // FAILED
    }

    return 0; // SUCCESS
  }

  uint32_t DACinstallerImplementation::Cancel_imp( const string& task, const string& listener)
  {
    DDD();

    // for(int i = 0; i < 50000; i++)
    // {
    //   PackageInfoEx *pp = Core::Service<PackageInfoEx>::Create<PackageInfoEx>("myName", "myVersion", "myID");
    //   mPPPlist.push_back(pp);
    // }

// fprintf(stderr, "\nHUGH >>>>> Fill LIST >>> mPPPlist.size()  = %ld ", mPPPlist.size() ); 

    return 0;
  }

  uint32_t DACinstallerImplementation::IsInstalled_imp(const string& pkgId)//, JsonObject &response)
  {
    fprintf(stderr, "\n\nDACinstallerImplementation::IsInstalled_imp() ... pkgId: [%s]\n\n", pkgId.c_str() ); 
    LOGERR("DAC::IsInstalled_imp(%s) ... ENTER", pkgId.c_str() ); 

    return DACutils::hasPkgRow( pkgId );
  }

  uint32_t DACinstallerImplementation::GetInstallProgress_imp(const string& task)
  {
    DDD();
    return 31;
  }

  PackageInfoEx::IIterator* DACinstallerImplementation::GetInstalled_imp()
  {
    DDD();
    return nullptr;
  }

  PackageInfoEx* DACinstallerImplementation::GetPackageInfo_imp(const string& pkgId)
  {
    PackageInfoEx* pkg = DACutils::getPkgRow(pkgId);

    int64_t foo = DACutils::sumSizeInBytes();
    LOGERR(" #########   foo = %ld bytes", foo);

    DACutils::showTable();
    // sendNotify("MyDummy Event", JsonObject());

    return pkg;
  }

  int64_t DACinstallerImplementation::GetAvailableSpace_imp()
  {    
    int64_t used_bytes = DACutils::sumSizeInBytes();

    //LOGERR("PackagerImplementation::GetAvailableSpace()  ... %jd   BBB", used_bytes);

    return ((STORE_BYTES_QUOTA - used_bytes)/1000); // in KB
  }
        
  }  // namespace Plugin
}  // namespace WPEFramework