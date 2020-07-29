
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

namespace WPEFramework {
namespace Plugin {

    class JobPool; //fwd

    class DACutils
    {
        public:
            DACutils(const DACutils&) = delete;
            DACutils& operator=(const DACutils&) = delete;

            // DACutils();
            // ~DACutils();

            static bool init(const char* filename, const char* key);
            static void term();
            static void vacuum();

            static bool fileRemove(const char* f);
            static bool fileExists(const char* f);
            static bool fileEncrypted(const char* f);

            static bool setValue(const string& ns, const string& key, const string& value);
            static bool getValue(const string& ns, const string& key, string& value);

            static bool deleteKey(const string& ns, const string& key);
            static bool deleteNamespace(const string& ns);

            // App installation helpers
            static int extract(const char *filename);

            static void setupThreadQ();
            static void killThreadQ();

            static void addJob();

            static int installURL(const char *url);

        private:
            static void*   mData;

            static JobPool jobPool;

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
