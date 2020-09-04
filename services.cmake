# If not stated otherwise in this file or this component's Licenses.txt file the
# following copyright and licenses apply:
#
# Copyright 2016 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

#
# features
#

option(PLUGIN_DISPLAYSETTINGS "Include DisplaySettings plugin" ON)
option(PLUGIN_LOGGING_PREFERENCES "Include LoggingPreferences plugin" ON)
option(PLUGIN_USER_PREFERENCES "Include UserPreferences plugin" ON)
option(PLUGIN_SYSTEMSERVICES "Include SystemServices plugin" ON)
option(PLUGIN_CONTROLSERVICE "Include ControlService plugin" ON)

#add_definitions (-DENABLE_MEMORYINFO_SERVICE)

add_definitions (-DWAREHOUSE_API)
option(WAREHOUSE_API "WAREHOUSE_API" ON)

#add_definitions (-DBROWSER_SETTINGS)
#add_definitions (-DSCREEN_CAPTURE)

add_definitions (-DSLEEP_TIMER)
option(SLEEP_TIMER "SLEEP_TIMER" OFF)

add_definitions (-DHAS_AUTHSERVICE)
option(HAS_AUTHSERVICE "HAS_AUTHSERVICE" OFF)

add_definitions (-DHAS_TIMER)
option(HAS_TIMER "HAS_TIMER" ON)

#add_definitions (-DENABLE_COOKIEJAR)
#add_definitions (-DENABLE_HEADERS2)
#add_definitions (-DENABLE_FRAME_RATE_SERVICE)
#add_definitions (-DUSE_USER_PREFERENCES_SERVICE)

add_definitions (-DHAS_API_HDMI_INPUT)
option(HAS_API_HDMI_INPUT "HAS_API_HDMI_INPUT" ON)

#add_definitions (-DHAS_RECEIVER_DIAGNOSTICS_SERVICE)
#add_definitions (-DHAS_LOGGING_PREFERENCES)

add_definitions (-DHAS_API_APPLICATION)
option(HAS_API_APPLICATION "HAS_API_APPLICATION" ON)

#add_definitions (-DHAS_API_PING)
#add_definitions (-DHAS_API_HDCP_COMPLIANCE)
#add_definitions (-DHAS_API_DATA_CAPTURE)

add_definitions (-DHAS_API_COPILOT)
option(HAS_API_COPILOT "HAS_API_COPILOT" OFF)

add_definitions (-DHAS_API_FRAME_RATE)
option(HAS_API_FRAME_RATE "HAS_API_FRAME_RATE" ON)

add_definitions (-DHAS_API_WEBSOCKET_PROXY)
option(HAS_API_WEBSOCKET_PROXY "HAS_API_WEBSOCKET_PROXY" OFF)

add_definitions (-DHAS_API_SCREEN_CAPTURE)

add_definitions (-DHAS_API_STORAGE_MANAGER)
option(HAS_API_STORAGE_MANAGER "HAS_API_STORAGE_MANAGER" OFF)

#add_definitions (-DHAS_API_INTRUSION_DETECTION)
#add_definitions (-DHAS_API_LINEAR_SEGMENTED_ADVERTISING)
#add_definitions (-DHAS_API_VIDEO_APPLICATION_EVENTS)
#add_definitions (-DENABLE_GET_PROGRESS_BYPASS)
#add_definitions (-DENABLE_RDK_BROWSER2)

add_definitions (-DHAS_API_DEVICEDIAGNOSTICS)
option(HAS_API_DEVICEDIAGNOSTICS "HAS_API_DEVICEDIAGNOSTICS" ON)

#add_definitions (-DHAS_API_DEVICEPROVISIONING)

add_definitions (-DHAS_API_SOUND)
option(HAS_API_SOUND "HAS_API_SOUND" OFF)

add_definitions (-DENABLE_HDCP_PROFILE)
option(ENABLE_HDCP_PROFILE "ENABLE_HDCP_PROFILE" ON)

add_definitions (-DHAS_ACTIVITY_MONITOR)
option(HAS_ACTIVITY_MONITOR "HAS_ACTIVITY_MONITOR" ON)

#add_definitions (-DCLIENT_VERSION_STRING)=\\\"$(VERSION_FULL_VALUE)\\\"
#add_definitions (-DSTB_VERSION_STRING)=\\\"$(FULL_VERSION_NAME_VALUE)\\\"
#add_definitions (-DSTB_TIMESTAMP_STRING)=\\\"$(STB_TIMESTAMP_VALUE)\\\"

#add_definitions (-DHAS_API_TTSSETTINGSSERVICE)
#add_definitions (-DHAS_API_TTSSESSIONSERVICE)
#add_definitions (-DHAS_API_TTSRESOURCESERVICE)
add_definitions (-DHAS_API_CONTINUE_WATCHING)
option(HAS_API_CONTINUE_WATCHING "HAS_API_CONTINUE_WATCHING" ON)

if(HAS_API_CONTINUE_WATCHING)
    if(CONTINUEWATCHING_DISABLE_SECAPI)
        add_definitions (-DDISABLE_SECAPI)
    endif()
endif()

#add_definitions (-DHAS_API_PROXIES)
add_definitions (-DENABLE_SYSTEM_5)
add_definitions (-DENABLE_SYSTEM_6)
add_definitions (-DENABLE_SYSTEM_7)
add_definitions (-DENABLE_SYSTEM_8)
add_definitions (-DENABLE_SYSTEM_9)
add_definitions (-DENABLE_SYSTEM_10)
add_definitions (-DENABLE_SYSTEM_11)
add_definitions (-DENABLE_SYSTEM_12)
add_definitions (-DENABLE_SYSTEM_13)
add_definitions (-DENABLE_SYSTEM_14)
add_definitions (-DENABLE_SYSTEM_15)
add_definitions (-DENABLE_SYSTEM_16)
add_definitions (-DENABLE_SYSTEM_17)

add_definitions (-DHAS_STATE_OBSERVER)
option(HAS_STATE_OBSERVER "HAS_STATE_OBSERVER" ON)

if (ENABLE_XCAST_PLUGIN)
    message("Enabling Xcast plugin")
    add_definitions (-DHAS_API_XCASTSERVICE)
    option(HAS_API_XCASTSERVICE "HAS_API_XCASTSERVICE" OFF)
endif()

add_definitions (-DDISABLE_WEBKIT_WIDGETS)

add_definitions (-DENABLE_DEVICE_PROVISIONING)
option(DEVICE_PROVISIONING "DEVICE_PROVISIONING" OFF)

if (ENABLE_BLUETOOTH_CONTROL)
    message("Building with Bluetooth support")
    add_definitions (-DENABLE_BLUETOOTH_CONTROL)
    add_definitions (-DHAS_API_BLUETOOTH)
    option(PLUGIN_BLUETOOTH "PLUGIN_BLUETOOTH" ON)
endif()

message("Building with DataCapture support")
option(PLUGIN_DATA_CAPTURE "Include DataCapture plugin" OFF)

if(BUILD_ENABLE_TSB)
    message("Building with TSB")
    add_definitions (-DBUILD_ENABLE_TSB)
    add_definitions (-DUSE_TSB_SETTINGS)
    add_definitions (-DENABLE_TSB_SETTING)
endif()

if (BUILD_ENABLE_CLOCK)
    message("Building with clock support")
    add_definitions (-DBUILD_ENABLE_CLOCK)
    add_definitions (-DCLOCK_BRIGHTNESS_ENABLED)
endif()

if (BUILD_ENABLE_MSO_PAIRING)
    message("Building with MSO pairing")
    add_definitions (-DBUILD_ENABLE_MSO_PAIRING)
    add_definitions (-DMSO_PAIRING)
endif()

if (BUILD_ENABLE_MSO_PAIRING_EXTENSIONS)
    message("Building with MSO pairing extensions")
    add_definitions (-DBUILD_ENABLE_MSO_PAIRING_EXTENSIONS)
    add_definitions (-DMSO_PAIRING_ENABLE_LOS)
    add_definitions (-DENABLE_MSOPAIRING_4)
    add_definitions (-DENABLE_MSOPAIRING_5)
endif()

if (BUILD_ENABLE_LATEST_FRONT_PANEL_API)
    message("Building with the latest front panel api")
    add_definitions (-DBUILD_ENABLE_LATEST_FRONT_PANEL_API)
    add_definitions (-DENABLE_FRONT_PANEL_API_V4)
endif()

if (BUILD_ENABLE_EXTENDED_ALL_SEGMENTS_TEXT_PATTERN)
    add_definitions (-DUSE_EXTENDED_ALL_SEGMENTS_TEXT_PATTERN)
endif()

if (BUILD_ENABLE_AV_INPUT)
    message("Building with AV input support")
    add_definitions (-DBUILD_ENABLE_AV_INPUT)
    add_definitions (-DHAS_API_AVINPUT)
    add_definitions (-DUSE_AVINPUT)
endif()

if (BUILD_ENABLE_TR69)
    message("Building with TR-69")
    add_definitions (-DBUILD_ENABLE_TR69)
    add_definitions (-DUSE_TR_69)
endif()

if(BUILD_ENABLE_STORAGE_MANAGER)
    message("Building with storage manager")
    add_definitions (-DBUILD_ENABLE_STORAGE_MANAGER)
    add_definitions (-DUSE_STORAGE_MANAGER_API)
endif()

if(BUILD_REFACTORED_STORAGEMANAGER)
    message("Building refactored storage manager - v2")
    add_definitions (-DBUILD_REFACTORED_STORAGEMANAGER)
    add_definitions (-DUSE_RDK_STORAGE_MANAGER_V2)
#    LIBS += -L"${FSROOT}/usr/lib" -lrdkstmgr
endif()

if(BUILD_ENABLE_VREX)
    message("Building with VREX")
    add_definitions (-DBUILD_ENABLE_VREX)
    add_definitions (-DENABLE_VREX_SERVICE)
endif()

if(BUILD_ENABLE_REMOTEACTIONMAPPING)
    message("Building with RemoteActionMappingService")
    add_definitions (-DBUILD_ENABLE_REMOTEACTIONMAPPING)
    add_definitions (-DENABLE_REMOTE_ACTION_MAPPING_SERVICE)
endif()

if(BUILD_ENABLE_CONTROL)
    message("Building with ControlService")
    add_definitions (-DBUILD_ENABLE_CONTROL)
    add_definitions (-DENABLE_CONTROL_SERVICE)
endif()

if(BUILD_DISABLE_WEBKIT_WIDGETS)
    message("Building with webkit widgets disabled")
    add_definitions (-DBUILD_DISABLE_WEBKIT_WIDGETS)
    add_definitions (-DDISABLE_WEBKIT_WIDGETS)
endif()

if(BUILD_ENABLE_WIFI_MANAGER)
    message("Building with wifi manager")
    option(PLUGIN_WIFIMANAGER "Include Wifi plugin" ON)
    option(PLUGIN_LOSTANDFOUND "Include LostAndFound plugin" OFF)
    add_definitions (-DBUILD_ENABLE_WIFI_MANAGER)
    add_definitions (-DHAS_WIFI_MANAGER)
endif()

if(BUILD_ENABLE_NETWORK_MANAGER)
    message("Building with network manager")
    option(PLUGIN_NETWORK "Include Network plugin" OFF)
    add_definitions (-DBUILD_ENABLE_NETWORK_MANAGER)
    add_definitions (-DHAS_NETWORK_MANAGER)
endif()

if(BUILD_ENABLE_CEF)
    message("Building with CEF")
    add_definitions (-DBUILD_ENABLE_CEF)
    add_definitions (-DUSE_CEF)
endif()

if(BUILD_BROADCOM)
    include(broadcom.cmake)
elseif(BUILD_RASPBERRYPI)
    include(raspberrypi.cmake)
elseif(BUILD_REALTEK)
    include(realtek.cmake)
elseif(BUILD_AMLOGIC)
    include(amlogic.cmake)
endif()






