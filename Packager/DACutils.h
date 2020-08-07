
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

#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>

#include "utils.h"

//TODO: make configurable and scoped
//
#define TMP_FILENAME  "/opt/tmpApp.tgz"
#define APPS_ROOT     "/opt/dac_apps"


namespace WPEFramework {
namespace Plugin {

//class DACinstallerImplementation; //fwd
class PackageInfoEx; //fwd

    class JobPool; //fwd

    class DACutils
    {
        public:
            enum class DACrc_t { dac_OK, dac_WARN, dac_FAIL };

            DACutils(const DACutils&) = delete;
            DACutils& operator=(const DACutils&) = delete;

            // DACutils();
            // ~DACutils();

            static bool init(const char* filename, const char* key);
            static bool createTable();

            // Clean up
            static void term();
            static void vacuum();

            // File hepers
            static bool    fileRemove(const char* f);
            static bool    fileExists(const char* f);
            static bool    fileEncrypted(const char* f);
            static bool    fileEndsWith(const string& f, const string& ext);

            static bool    removeFolder(const string& dirname);
            static bool    removeFolder(const char *dirname);

            static int64_t folderSize(const char *d);

            static std::string getGUID();

            static bool           hasPkgRow(const string& pkgId);
            static bool           hasPkgRow(const char* pkgId);
            static bool           addPkgRow(const PackageInfoEx* pkg);
            static PackageInfoEx* getPkgRow(const string& pkgId);
            static bool           delPkgRow(const string& pkgId);

            static int64_t        sumSizeInBytes();

            static PackageInfoEx* mThisPkg;

            static void showTable();

            // SQL helpers
            // static bool setValue(const string& ns, const string& key, const string& value);
            // static bool getValue(const string& ns, const string& key, string& value);

            // static bool deleteKey(const string& ns, const string& key);
            // static bool deleteNamespace(const string& ns);

            // House-keeping
            static void setupThreadQ();
            static void killThreadQ();

           // static DACrc_t installURL(const char *url);

            // Install helpers
            static DACrc_t downloadJSON(const char *url);
            static DACrc_t downloadURL(const char *url);
            
            static DACrc_t extract(const char *filename, const char *to_path = nullptr);

            static void addJob();

            // private data
            static void*        mData;
            static JsonObject   mPackageCfg;

        private:

            static JobPool                     jobPool;
            static std::vector<std::thread> threadPool; // thread pool

            static const int64_t  MAX_SIZE_BYTES;
            static const int64_t  MAX_VALUE_SIZE_BYTES;
    };

    class JobPool
    {
       public:
            JobPool(const JobPool&) = delete;
            JobPool& operator=(const JobPool&) = delete;

            // DACpool();
            // ~DACpool();

        JobPool();
        ~JobPool();

        void push(std::function<void()> func);
        void done();
        void worker_func();

      private:
          std::queue<std::function<void()>> m_JobQ;
          std::mutex                        m_lock;
          std::condition_variable           m_data_condition;
          std::atomic<bool>                 m_accept_functions;
    };
  } // namespace Plugin
}  // namespace WPEFramework
