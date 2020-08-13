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

#include "PackagerExUtils.h"
#include "PackagerExImplementation.h"


#define MB_in_BYTES  1000000

const int64_t WPEFramework::Plugin::PackagerExImplementation::STORE_BYTES_QUOTA = 10 * MB_in_BYTES;
const char*   WPEFramework::Plugin::PackagerExImplementation::STORE_NAME        = "DACstorage";
const char*   WPEFramework::Plugin::PackagerExImplementation::STORE_KEY         = "4d4680a1-b3b0-471c-968b-39495d2b1cc3";

using namespace std;

namespace WPEFramework {
namespace Plugin {

// Events
#define DAC_EVT_INSTALL_ACK "DAC_InstallAck"

  PackagerExImplementation::PackagerExImplementation()
                                : mTaskNumber(0)
  {
    DDD();

   // RegisterAll();  

    auto path = g_build_filename("/opt", "persistent", nullptr);

    if (!PackagerExUtils::fileExists(path))
    {
        g_mkdir_with_parents(path, 0745);
    }

    auto file = g_build_filename(path, STORE_NAME, nullptr);

    bool success = PackagerExUtils::init(file, STORE_KEY);
    LOGERR("\n %s() ... SQLite >> Init()  %s ", __PRETTY_FUNCTION__, (success ? " OK" : " FAILED !"));

    if(success)
    {


//JUNK JUNK JUNK JUNK JUNK JUNK JUNK JUNK JUNK JUNK JUNK JUNK 
//JUNK JUNK JUNK JUNK JUNK JUNK JUNK JUNK JUNK JUNK JUNK JUNK 
//
//  TEST CODE...
//
#if 0  
      PackageInfoEx *pkg = Core::Service<PackageInfoEx>::Create<PackageInfoEx>("myName", "myVersion", "myID");

      pkg->setName("Test Name");
      pkg->setBundlePath("/opt/foo/bar/myAppFolder");
      pkg->setVersion("1.2.3");
      pkg->setPkgId("TestApp0123456");
      pkg->setInstalled("Fri Jul 31 16:54:41 UTC 2020");
      pkg->setSizeInBytes(123456);
      pkg->setType("DAC");



LOGERR("########## addPkgRow \n");
  PackagerExUtils::addPkgRow(pkg);  

    pkg->Release();

LOGERR("########## hasPkgRow('TestApp0123456') == %s\n", 
      (PackagerExUtils::hasPkgRow( "TestApp0123456" ) ? "TRUE" : "FALSE") );



LOGERR("########## hasPkgRow('foo') == %s\n\n", 
      (PackagerExUtils::hasPkgRow( "foo" ) ? "TRUE" : "FALSE") );


LOGERR("########## showTable \n\n");
PackagerExUtils::showTable();


LOGERR("\n########## delPkgRow\n");
  PackagerExUtils::delPkgRow("TestApp0123456");


LOGERR("########## NOW ? hasPkgRow('TestApp0123456') == %s\n", 
      (PackagerExUtils::hasPkgRow( "TestApp0123456" ) ? "TRUE" : "FALSE") );

#endif //00
//JUNK JUNK JUNK JUNK JUNK JUNK JUNK JUNK JUNK JUNK JUNK JUNK 
//JUNK JUNK JUNK JUNK JUNK JUNK JUNK JUNK JUNK JUNK JUNK JUNK 


// fprintf(stderr, "\n########## getPkgRow\n");
// PackageInfoEx* pp = PackagerExUtils::getPkgRow("TestApp0123456");

// PackageInfoEx::printPkg(pp);

     // PackagerExUtils::setupThreadQ(); // start thread Q 

      // PackagerExUtils::installURL("http://10.0.2.15/test.tgz");
    }

    g_free(path);
    g_free(file);
  }

  PackagerExImplementation::~PackagerExImplementation()
  {
    DDD();

   // UnregisterAll();

    PackagerExUtils::term();
  }


  // DAC Installer API
  uint32_t PackagerExImplementation::Install_imp(const string& pkgId, const string& type, const string& url,
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

  uint32_t PackagerExImplementation::doInstall(const string& pkgId, const string& type, const string& url,
                                                 const string& token, const string& listener)
  {
    std::string install_name( "(empty))" );
    std::string install_url( url );
    std::string install_ver( "1.2.3" );

    // Parse the URL...
    //
    if(PackagerExUtils::fileEndsWith(url.c_str(), "json"))
    {
        LOGINFO(" %s() ... DOWNLOAD >>>  %s\n", __PRETTY_FUNCTION__, install_url.c_str());

        // Download JSON manifest...
        //
        if(PackagerExUtils::downloadJSON(url.c_str()) != PackagerExUtils::DACrc_t::dac_OK)
        {
            LOGERR(" %s() ... ERROR:  Failed to download JSON >> %s \n", __PRETTY_FUNCTION__, url.c_str());
            return -1; // FAIL  //PackagerExUtils::DACrc_t::dac_FAIL;
        }

        // Parse JSON for meta...
        install_url  = PackagerExUtils::mPackageCfg["install"].String(); // update install from URL
        install_name = PackagerExUtils::mPackageCfg["name"].String();
        install_ver  = PackagerExUtils::mPackageCfg["version"].String();
    }
  
    // Download TGZ package...
    //
    LOGINFO(" %s() ... DOWNLOAD >>>  %s\n", __PRETTY_FUNCTION__, install_url.c_str());

    if(PackagerExUtils::downloadURL(install_url.c_str()) != PackagerExUtils::DACrc_t::dac_OK)
    {
        LOGERR(" %s() ... DOWNLOAD (%s)>>>  FAILED\n", __PRETTY_FUNCTION__, install_url.c_str());
        return -1; // FAIL
    }
    else
    {
        char uuid_path[PATH_MAX];

        // Get UUID ...
        std::string uuid_str = PackagerExUtils::getGUID();

        // Create path ... APPS_ROOT / {UUID} / {app}
        snprintf(uuid_path, PATH_MAX, "%s/%s/", APPS_ROOT, uuid_str.c_str());
        
        if(PackagerExUtils::extract(TMP_FILENAME, uuid_path) != PackagerExUtils::DACrc_t::dac_OK)
        {
            // Clean up failed extraction
            //
            LOGERR(" %s() ... EXTRACT >>>  FAILED\n", __PRETTY_FUNCTION__);

            PackagerExUtils::removeFolder(uuid_path); // remove debris

            return -1; // FAIL
        }

        // TODO: look for JSON meta in app bundle...
        //
        LOGINFO(" %s() ... INSTALLED >>> [ %s ]\n", __PRETTY_FUNCTION__, install_name.c_str());

        // Always cleanup
        PackagerExUtils::fileRemove(TMP_FILENAME);

        PackageInfoEx* pkg = Core::Service<PackageInfoEx>::Create<PackageInfoEx>();

        time_t rawtime;
        time (&rawtime);

        int32_t bytes = PackagerExUtils::folderSize(uuid_path);

        pkg->setPkgId(pkgId);
        pkg->setName(install_name);
        pkg->setBundlePath(uuid_path);
        pkg->setVersion(install_ver);
        pkg->setInstalled(ctime (&rawtime));
        pkg->setSizeInBytes(bytes);
        pkg->setType(type);
        
        PackagerExUtils::addPkgRow(pkg); // add to SQL 

        pkg->Release();
    }

    return 0;
  }

  uint32_t PackagerExImplementation::Remove_imp( const string& pkgId, const string& listener)
  {
    LOGINFO("... Remove_imp(%s, %s) - ENTER ", pkgId.c_str(), listener.c_str());

    PackageInfoEx* pkg = PackagerExUtils::getPkgRow(pkgId);

    if(pkg)
    {
      LOGERR(" removeFolder( %s ) ... NOT found", pkg->BundlePath().c_str());

      PackagerExUtils::removeFolder(pkg->BundlePath());

      bool rc = PackagerExUtils::delPkgRow(pkgId);

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

  uint32_t PackagerExImplementation::Cancel_imp( const string& task, const string& listener)
  {
    DDD();

// fprintf(stderr, "\nHUGH >>>>> Fill LIST >>> mPPPlist.size()  = %ld ", mPPPlist.size() ); 

    return 0;
  }

  uint32_t PackagerExImplementation::IsInstalled_imp(const string& pkgId)//, JsonObject &response)
  {
    fprintf(stderr, "\n\nPackagerExImplementation::IsInstalled_imp() ... pkgId: [%s]\n\n", pkgId.c_str() ); 
    LOGERR("DAC::IsInstalled_imp(%s) ... ENTER", pkgId.c_str() ); 

    return PackagerExUtils::hasPkgRow( pkgId );
  }

  uint32_t PackagerExImplementation::GetInstallProgress_imp(const string& task)
  {
    DDD();
    return 31;
  }

  PackageInfoEx::IIterator* PackagerExImplementation::GetInstalled_imp()
  {
    DDD();
    return nullptr;
  }

  PackageInfoEx* PackagerExImplementation::GetPackageInfo_imp(const string& pkgId)
  {
    PackageInfoEx* pkg = PackagerExUtils::getPkgRow(pkgId);

    PackagerExUtils::showTable();
    // sendNotify("MyDummy Event", JsonObject());

    return pkg;
  }

    void PackagerExImplementation::SendN()
    {
        // Core::Time now(Core::Time::Now());
        // Core::JSON::String currentTime;

        // currentTime = now.ToRFC1123();

// JsonObject params;
// params["Time"] = "dummy";
// params["name"] = "Testing";


        // PluginHost::JSONRPC method to send out a JSONRPC message to all subscribers to the event "clock".
//        Notify(_T("onInstallComplete"), params);

// LOGERR("PackagerExImplementation::SendN()  .... sent ..." );


		// We are currently supporting more release, the old interface is expecting a bit a different response:
   //    GetHandler(1)->Notify(_T("clock"), "Groundhog Day");// Data::Time(now.Hours(), now.Minutes(), now.Seconds()));
    }

  int64_t PackagerExImplementation::GetAvailableSpace_imp()
  {    
// JUNK
// JUNK
// JUNK
LOGERR("PackagerImplementation::GetAvailableSpace()  .... Sending ..." );


// JsonObject params;
// params["descriptor"] = "dummy";
// params["name"] = "Testing";
// sendNotify("onInstallComplete", params);

// Notify(_T("onInstallComplete"), params);

// SendN();

// SendN();

// SendN();

// SendN();

// 

LOGERR("PackagerImplementation::GetAvailableSpace()  .... SENT ..." );
// JUNK
// JUNK
// JUNK

    int64_t used_bytes = PackagerExUtils::sumSizeInBytes();

    LOGERR("PackagerImplementation::GetAvailableSpace()  .... %jd ", used_bytes);

    return ((STORE_BYTES_QUOTA - used_bytes)/1000); // in KB
  }
        
  }  // namespace Plugin
}  // namespace WPEFramework