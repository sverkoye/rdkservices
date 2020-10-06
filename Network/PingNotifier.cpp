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

#include "Network.h"

namespace WPEFramework
{
    namespace Plugin
    {
        /**
         * @ingroup SERVMGR_PING_API
         */
        JsonObject Network::_doPing(std::string endPoint, int packets)
        {
            LOGINFO("PingService calling ping");
            JsonObject pingResult;
            std::string interface = "";
            std::string gateway;
            bool result = false;
            std::string outputFile;
            FILE *fp = NULL;

            pingResult["target"] = endPoint;

            if(NetUtils::isIPV6(endPoint))
            {
                LOGINFO("%s: Endpoint '%s' is ipv6", __FUNCTION__,endPoint.c_str());
            }
            else if(NetUtils::isIPV4(endPoint))
            {
                LOGINFO("%s: Endpoint '%s' is ipv4", __FUNCTION__,endPoint.c_str());
            }
            else if(NetUtils::isValidEndpointURL(endPoint))
            {
                LOGINFO("%s: Endpoint '%s' is url", __FUNCTION__,endPoint.c_str());
            }
            else
            {
                LOGERR("%s: Endpoint '%s' is not valid", __FUNCTION__,endPoint.c_str());
                pingResult["success"] = false;
                pingResult["error"] = "invalid input for endpoint: " + endPoint;
                return pingResult;
            }

            if (!_getDefaultInterface(interface, gateway))
            {
                LOGERR("%s: Could not get default interface", __FUNCTION__);
                pingResult["success"] = false;
                pingResult["error"] = "Could not get default interface";
                return pingResult;
            }

            char cmd [1000] = {0x0};
            if (NetUtils::isIPV6(endPoint))
            {
                snprintf(cmd, sizeof(cmd), "ping6 -I %s -c %d -W 5 '%s' 2>&1",
                        interface.c_str(), packets, endPoint.c_str());
            }
            else
            {
                snprintf(cmd, sizeof(cmd), "ping -c %d -W 5 '%s' 2>&1",
                        packets, endPoint.c_str());
            }

            LOGWARN("ping command: %s", cmd);

            // Run the command and dump the output to /tmp/pingoutput
            if (NetUtils::execCmd(cmd, outputFile, &result, "pingoutput") < 0)
            {
                LOGERR("%s: SERVICEMANAGER_FILE_ERROR: Can't open pipe for command '%s' for read mode: %s"
                        , __FUNCTION__, cmd, strerror(errno));

                pingResult["success"] = false;
                pingResult["error"] = "Could not run command";
            }
            else if (!result) // check the command return status
            {
                pingResult["success"] = false;
                pingResult["error"] = "Could not ping endpoint";
            }
            else if ((fp = fopen(outputFile.c_str(), "r")) == NULL)
            {
                pingResult["success"] = false;
                pingResult["error"] = "Could not read ping result";
            }
            else
            {
                pingResult["success"] = true;
                pingResult["error"] = "";

                char linearray[1000]={0x0};
                while(fgets(linearray, sizeof(linearray), fp) != NULL)
                {
                    std::string line(linearray);
                    LOGINFO("ping result: %s", line.c_str());

                    if( line.find( "packet" ) != std::string::npos ) {
                        //Example: 10 packets transmitted, 10 packets received, 0% packet loss

                        std::stringstream ss( line );
                        int transCount;
                        ss >> transCount;
                        pingResult["packetsTransmitted"] = transCount;

                        std::string token;
                        getline( ss, token, ',' );
                        getline( ss, token, ',' );
                        std::stringstream ss2( token );
                        int rxCount;
                        ss2 >> rxCount;
                        pingResult["packetsReceived"] = rxCount;

                        getline( ss, token, ',' );
                        std::string prefix = token.substr(0, token.find("%"));
                        //double lossFloat = ::atof(prefix.c_str());
                        //pingResult["packetLoss"] = lossFloat;
                        pingResult["packetLoss"] = prefix.c_str();

                    }else if( line.find( "min/avg/max" ) != std::string::npos ) {
                        //Example: round-trip min/avg/max = 17.038/18.310/20.197 ms

                        std::stringstream ss( line );
                        std::string fullpath;
                        getline( ss, fullpath, '=' );
                        getline( ss, fullpath, '=' );

                        std::string prefix;
                        int index = fullpath.find("/");
                        if (index >= 0)
                        {
                            prefix = fullpath.substr(0, fullpath.find("/"));
                            pingResult["tripMin"] = prefix.c_str();
                        }

                        index = fullpath.find("/");
                        if (index >= 0)
                        {
                            fullpath = fullpath.substr(index + 1, fullpath.length());
                            prefix = fullpath.substr(0, fullpath.find("/"));
                            pingResult["tripAvg"] = prefix.c_str();
                        }

                        index = fullpath.find("/");
                        if (index >= 0)
                        {
                            fullpath = fullpath.substr(index + 1, fullpath.length());
                            prefix = fullpath.substr(0, fullpath.find("/"));
                            pingResult["tripMax"] = prefix.c_str();
                        }

                        index = fullpath.find("/");
                        if (index >= 0)
                        {
                            fullpath = fullpath.substr(index + 1, fullpath.length());
                            pingResult["tripStdDev"] = prefix.c_str();
                        }
                    }else if( line.find( "bad" ) != std::string::npos ) {
                        pingResult["success"] = false;
                        pingResult["error"] = "Bad Address";
                    }
                }
                fclose(fp);

                // clear up
                std::remove(outputFile.c_str());
            }

            return pingResult;
        }

        /**
         * @ingroup SERVMGR_PING_API
         */
        JsonObject Network::_doPingNamedEndpoint(std::string endpointName, int packets)
        {
            LOGINFO("PingService calling pingNamedEndpoint for %s", endpointName.c_str());
            std::string error = "";
            JsonObject returnResult;

            if (endpointName == "CMTS")
            {
                std::string gateway = "";
                string interface;
                if (_getDefaultInterface(interface, gateway))
                {
                    returnResult = _doPing(gateway, packets);
                }
                else
                {
                    LOGERR("%s: Can't get gateway address for interface CMTS", __FUNCTION__);
                    error = "Could not find interface";
                }
            }
            else
            {
                error = "Invalid endpoint name";
            }


            if (error != "")
            {
                returnResult["target"] = endpointName;
                returnResult["success"] = false;
                returnResult["error"] = error;
            }

            return returnResult;
        }
    }
}
