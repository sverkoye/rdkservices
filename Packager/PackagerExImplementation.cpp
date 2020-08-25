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
 


 // JUNK
 // JUNK
 // JUNK
 #define JUNK_MS   500
 // JUNK
 // JUNK
 // JUNK
 
#include <glib.h>

#include "time.h"
#include <locale>
#include <thread>         // std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds

#include "utils.h"

#include "PackagerExUtils.h"

#include "PackagerImplementation.h"
#include "PackagerExImplementation.h"


#define MB_in_BYTES  1000000

const int64_t WPEFramework::Plugin::PackagerImplementation::STORE_BYTES_QUOTA = 10 * MB_in_BYTES;
const char*   WPEFramework::Plugin::PackagerImplementation::STORE_NAME        = "DACstorage";
const char*   WPEFramework::Plugin::PackagerImplementation::STORE_KEY         = "4d4680a1-b3b0-471c-968b-39495d2b1cc3";

using namespace std;

namespace WPEFramework {
namespace Plugin {

// Events
#define DAC_EVT_INSTALL_ACK "DAC_InstallAck"

  // PackagerExImplementation::PackagerExImplementation()
  //                               : _taskNumber(0)
  void PackagerImplementation::InitPackageDB()
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

  // DAC Installer API
  uint32_t PackagerImplementation::Install(const string& pkgId, const string& type, const string& url,
                                                   const string& token, const string& listener)
  { 
      LOGERR("\n\n HUGH .... PackagerImplementation::Install() - ENTER");

      std::thread threadObj([pkgId, type,  url, token, listener, this]
      {
        this->doInstall(pkgId, type,  url, token, listener); // TODO: THREAD THIS
      });

     // threadObj.join();

    // response["error"] = "params missing";
    // response["value"] = "test123";
    // returnResponse(true);

    return 0;
  }

  uint32_t PackagerImplementation::doInstall(const string& pkgId, const string& type, const string& url,
                                             const string& token, const string& listener)
  {
    std::string install_name;
    std::string install_url;
    std::string install_ver;

//    NotifyIntallStep(INSTALL_START);

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
            return 11; // FAIL  //PackagerExUtils::DACrc_t::dac_FAIL;
        }

        // Parse JSON for meta...
        install_url  = PackagerExUtils::mPackageCfg["install"].String(); // update install from URL
        install_name = PackagerExUtils::mPackageCfg["name"].String();
        install_ver  = PackagerExUtils::mPackageCfg["version"].String();

        // Check NOT empty/exist
        if(install_url.empty()  || install_url  == "null" ||
           install_name.empty() || install_name == "null" ||
           install_ver.empty()  || install_ver  == "null" )
        {
            LOGERR( " %s() ... ERROR:  Missing keys in JSON >> %s \n", __PRETTY_FUNCTION__, url.c_str());
            LOGINFO(" %s() ... ERROR:  install_url: %s   install_name: %s  install_ver: %s\n", 
                __PRETTY_FUNCTION__, install_url.c_str(), install_name.c_str(), install_ver.c_str());

            return 22; // FAIL  //PackagerExUtils::DACrc_t::dac_FAIL;          
        }

        // Validate URL
        if(PackagerExUtils::validateURL(install_url.c_str()) != PackagerExUtils::DACrc_t::dac_OK )
        {
            LOGERR(" %s() ... ERROR:  Invlaid URL >> %s \n", __PRETTY_FUNCTION__, url.c_str());
            return 33; // FAIL  //PackagerExUtils::DACrc_t::dac_FAIL;          
        }
    }
    else
    {
      // No JSON manifest - just a .tgz

      LOGWARN(" %s() ... WARN:  No JSON manifest - just a .tgz - using dummy fields >> %s \n", __PRETTY_FUNCTION__, url.c_str());
      // TODO:  Find a JSON manifest withing the .tgz ?

      install_name = "(empty)";
      install_ver  = "1.2.3";
    }
  
    NotifyIntallStep(Exchange::IPackager::DOWNLOADING);
/* JUNK */ std::this_thread::sleep_for(std::chrono::milliseconds(JUNK_MS)); // JUNK

    // Download TGZ package...
    //
    LOGINFO(" %s() ... DOWNLOAD >>>  %s\n", __PRETTY_FUNCTION__, install_url.c_str());

    if(PackagerExUtils::downloadURL(install_url.c_str()) != PackagerExUtils::DACrc_t::dac_OK)
    {
        LOGERR(" %s() ... DOWNLOAD (%s)>>>  FAILED\n", __PRETTY_FUNCTION__, install_url.c_str());
        return 44; // FAIL
    }
    else
    {
        NotifyIntallStep(Exchange::IPackager::DOWNLOADED);
 /* JUNK */ std::this_thread::sleep_for(std::chrono::milliseconds(JUNK_MS)); // JUNK

        NotifyIntallStep(Exchange::IPackager::VERIFYING);
/* JUNK */ std::this_thread::sleep_for(std::chrono::milliseconds(JUNK_MS)); // JUNK

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

            return 55; // FAIL
        }

        NotifyIntallStep(Exchange::IPackager::VERIFIED);
/* JUNK */ std::this_thread::sleep_for(std::chrono::milliseconds(JUNK_MS)); // JUNK

        // TODO: look for JSON meta in app bundle...
        //
        LOGINFO(" %s() ... INSTALLED >>> [ %s ]\n", __PRETTY_FUNCTION__, install_name.c_str());

        // Always cleanup
        PackagerExUtils::fileRemove(TMP_FILENAME);

        NotifyIntallStep(Exchange::IPackager::INSTALLING);
/* JUNK */ std::this_thread::sleep_for(std::chrono::milliseconds(JUNK_MS)); // JUNK

        PackageInfoEx* pkg = Core::Service<PackageInfoEx>::Create<PackageInfoEx>();

        time_t rawtime;
        time (&rawtime);
        std::string strtime = ctime (&rawtime);

        // NOTE:  Remove trailing '\n' >> illegal in JSON  
        //
        // "Tue Aug 25 18:04:14 2020\n"  >>> "Tue Aug 25 18:04:14 2020"
        //
        strtime.pop_back(); // (C++11 code) 

        int32_t bytes = PackagerExUtils::folderSize(uuid_path);

        pkg->setPkgId(pkgId);
        pkg->setName(install_name);
        pkg->setBundlePath(uuid_path);
        pkg->setVersion(install_ver);
        pkg->setInstalled( strtime );
        pkg->setSizeInBytes(bytes);
        pkg->setType(type);
        
        PackagerExUtils::addPkgRow(pkg); // add to SQL 

/* JUNK */ std::this_thread::sleep_for(std::chrono::milliseconds(JUNK_MS)); // JUNK

        NotifyIntallStep(Exchange::IPackager::INSTALLED);

        pkg->Release();
    }

    LOGERR("\n\n HUGH .... PackagerImplementation::doInstall() - EXIT"); 

    return 0; // no error
  }

  uint32_t PackagerImplementation::Remove( const string& pkgId, const string& listener)
  {
    LOGINFO("... Remove(%s, %s) - ENTER ", pkgId.c_str(), listener.c_str());

    PackageInfoEx* pkg = PackagerExUtils::getPkgRow(pkgId);

    if(pkg)
    {
      LOGERR(" removeFolder( %s ) ... NOT found", pkg->BundlePath().c_str());

      PackagerExUtils::removeFolder(pkg->BundlePath());

      bool rc = PackagerExUtils::delPkgRow(pkgId);

      if(rc == false)
      {
        LOGINFO("... Remove(%s, %s) - FAILED... not found ? ", pkgId.c_str(), listener.c_str());
        return -1; // FAILED
      }
    }
    else
    {
      LOGERR(" .Remove( %s ) ... NOT found", pkgId.c_str());
      return -1; // FAILED
    }

    return 0; // SUCCESS
  }

  uint32_t PackagerImplementation::Cancel( const string& task, const string& listener)
  {
    DDD();

    // TODO: 
    // fprintf(stderr, "\nHUGH >>>>> Fill LIST >>> mPPPlist.size()  = %ld ", mPPPlist.size() ); 

    return 0;
  }

  uint32_t PackagerImplementation::IsInstalled(const string& pkgId)//, JsonObject &response)
  {
    LOGERR("\n\nPackagerExImplementation::IsInstalled() ... pkgId: [%s]\n\n", pkgId.c_str() ); 

    return PackagerExUtils::hasPkgRow( pkgId );
  }

  uint32_t PackagerImplementation::GetInstallProgress(const string& task)
  {
    DDD();

    NotifyIntallStep(22);

    // TODO: 
    return 42;
  }

  PackageInfoEx::IIterator* PackagerImplementation::GetInstalled()
  {
    DDD();

    // TODO: 
    return nullptr;
  }

  PackageInfoEx* PackagerImplementation::GetPackageInfo(const string& pkgId)
  {
    LOGERR("DEBUG:  GetPackageInfo() - ENTER" );

    PackageInfoEx* pkg = PackagerExUtils::getPkgRow(pkgId);

    // if(pkg)
    // {
    //   PackagerExUtils::showTable();
    // }

    return pkg;
  }

  int64_t PackagerImplementation::GetAvailableSpace()
  {
    NotifyIntallStep(0);

    int64_t used_bytes = PackagerExUtils::sumSizeInBytes();

    LOGERR("PackagerExImplementation::GetAvailableSpace()  ... used_bytes: %jd ", used_bytes);

    return ((STORE_BYTES_QUOTA - used_bytes)/1000); // in KB
  }
  }  // namespace Plugin
}  // namespace WPEFramework