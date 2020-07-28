
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

#include "utils.h"

#define DDD()   fprintf(stderr, "\nHUGH >>>>> DAC Impl ... Call ... %s()", __FUNCTION__); 

namespace WPEFramework {
namespace Plugin {

    class DACutils
    {
        public:
            static bool init(const char* filename, const char* key);
            static void term();
            static void vacuum();

            static bool fileRemove(const char* f);
            static bool fileExists(const char* f);
            static bool fileEncrypted(const char* f);

        private:
            static void* mData;
    };

  } // namespace Plugin
}  // namespace WPEFramework
