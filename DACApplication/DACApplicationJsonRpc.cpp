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

#include "Module.h"
#include "DACApplication.h"

namespace WPEFramework {

namespace Plugin {

    // Registration
    //

    void DACApplication::RegisterAll()
    {
        Property<Core::JSON::String>(_T("url"), &DACApplication::get_url, &DACApplication::set_url, this);
    }

    void DACApplication::UnregisterAll()
    {
        Unregister(_T("url"));
    }

    // API implementation
    //

    // Property: url - URL loaded in the browser
    // Return codes:
    //  - ERROR_NONE: Success
    uint32_t DACApplication::get_url(Core::JSON::String& response) const
    {
        response = _dacappBundlePath;

        return Core::ERROR_NONE;
    }

    // Property: url - name of dacapp
    // Return codes:
    //  - ERROR_NONE: Success
    uint32_t DACApplication::set_url(const Core::JSON::String& param)
    {
        if (param.IsSet() && !param.Value().empty()) {
            _dacappBundlePath = param.Value();
            SYSLOG(Trace::Information, (_T("SET URL/DACAPP %s"), _dacappBundlePath.c_str()));
            StartContainer();
        }
        return Core::ERROR_NONE;
    }

} // namespace Plugin

}

