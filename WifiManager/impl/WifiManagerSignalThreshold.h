/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
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
**/

#pragma once

#include "../Module.h"
#include "../WifiManagerDefines.h"
#include "../WifiManagerInterface.h"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace WPEFramework {
    namespace Plugin {
        class WifiManagerSignalThreshold {
            // This class realizes the following methods:
            // - setSignalThresholdChangeEnabled
            // - isSignalThresholdChangeEnabled
            // And the following event:
            // - onWifiSignalTresholdChanged
            // From WifiManager module.
            //
            // As the onWifiSignalTresholdChanged event is signalled periodically,
            // it has to be handled with an additional thread.
        public:
            WifiManagerSignalThreshold(WifiManagerInterface &wifiManager);
            virtual ~WifiManagerSignalThreshold();
            WifiManagerSignalThreshold(const WifiManagerSignalThreshold&) = delete;
            WifiManagerSignalThreshold& operator=(const WifiManagerSignalThreshold&) = delete;

            uint32_t setSignalThresholdChangeEnabled(const JsonObject& parameters, JsonObject& response);
            void setWifiStateConnected(bool connected);
            uint32_t isSignalThresholdChangeEnabled(const JsonObject& parameters, JsonObject& response) const;

        private:
            void setSignalThresholdChangeEnabled(bool enabled, int interval);
            bool isSignalThresholdChangeEnabled() const;

            void loop(int interval);
            void stopThread();
            void startThread(int interval);
            void getWifiState(WifiState &state);

        private:
            std::thread thread;
            std::atomic<bool> changeEnabled;
            std::mutex cv_mutex;
            std::condition_variable cv;
            WifiManagerInterface &wifiManager;
            bool running;
        };
    }
}
