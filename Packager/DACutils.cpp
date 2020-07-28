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
 
#include <sqlite3.h>
#include <glib.h>

#include "DACutils.h"

#ifndef SQLITE_FILE_HEADER
#define SQLITE_FILE_HEADER "SQLite format 3"
#endif

#define SQLITE *(sqlite3**) &mData


namespace WPEFramework {
namespace Plugin {


#if defined(SQLITE_HAS_CODEC)

    bool DACutils::fileEncrypted(const char* f)
    {
        FILE* fd = fopen(f, "rb");
        if (!fd)
        {
            return false;
        }

        int    magicSize = strlen(SQLITE_FILE_HEADER);
        char* fileHeader = (char*)malloc(magicSize + 1);
        int     readSize = (int)fread(fileHeader, 1, magicSize, fd);

        fclose(fd);

        bool eq = magicSize == readSize && ::memcmp(fileHeader, SQLITE_FILE_HEADER, magicSize) == 0;
        free(fileHeader);

        return !eq;
    }
#endif

    bool DACutils::fileRemove(const char* f)
    {
        return ( remove (f) == 0);
    }

    bool DACutils::fileExists(const char* f)
    {
        return g_file_test(f, G_FILE_TEST_EXISTS);
    }

    bool DACutils::init(const char* filename, const char* key)
    {
        // LOGINFO();
        fprintf(stderr, "\n %s() ... SQLite >>  filename: %s    key: %s", __PRETTY_FUNCTION__, filename, key);

        sqlite3* &db = SQLITE;

        term();

        bool shouldEncrypt = key && *key;
    #if defined(SQLITE_HAS_CODEC)
        bool shouldReKey = shouldEncrypt && fileExists(filename) && !fileEncrypted(filename);
    #endif
        int rc = sqlite3_open(filename, &db);
        if (rc)
        {
            fprintf(stderr, "\n %s() ... SQLite >>  %d : %s", __PRETTY_FUNCTION__, rc, sqlite3_errmsg(db));

            // LOGERR("%d : %s", rc, sqlite3_errmsg(db));
            term();
            return false;
        }

        /* Based on pxCore, Copyright 2015-2018 John Robinson */
        /* Licensed under the Apache License, Version 2.0 */
        if (shouldEncrypt)
        {
    #if defined(SQLITE_HAS_CODEC)
            std::vector<uint8_t> pKey;
    #if defined(USE_PLABELS)

            // NOTE: pbnj_utils stores the nonce under XDG_DATA_HOME/data.
            // If the dir doesn't exist it will fail
            auto path = g_build_filename(g_get_user_data_dir(), "data", nullptr);
            if (!fileExists(path))
                g_mkdir_with_parents(path, 0755);
            g_free(path);

            bool result = pbnj_utils::prepareBufferForOrigin(key, [&pKey](const std::vector<uint8_t>& buffer) {
                pKey = buffer;
            });
            if (!result)
            {
                // LOGERR("pbnj_utils fail");
                term();
                return false;
            }
    #else
            // LOGWARN("SQLite encryption key is not secure, path=%s", filename);
            pKey = std::vector<uint8_t>(key, key + strlen(key));
    #endif
            if (!shouldReKey)
                rc = sqlite3_key_v2(db, nullptr, pKey.data(), pKey.size());
            else
            {
                rc = sqlite3_rekey_v2(db, nullptr, pKey.data(), pKey.size());
                if (rc == SQLITE_OK)
                    vacuum();
            }

            if (rc != SQLITE_OK)
            {
              fprintf(stderr, "\n %s() ... Failed to attach encryption key to SQLite", __PRETTY_FUNCTION__);
                // LOGERR("Failed to attach encryption key to SQLite database %s\nCause - %s", filename, sqlite3_errmsg(db));
                term();
                return false;
            }

            if (shouldReKey && !fileEncrypted(filename))
            {
                fprintf(stderr, "\n %s() ... SQLite database file is clear after re-key", __PRETTY_FUNCTION__);

                // LOGERR("SQLite database file is clear after re-key, path=%s", filename);
            }
    #endif
        }

        char *errmsg;
        rc = sqlite3_exec(db, "CREATE TABLE if not exists namespace ("
                              "id INTEGER PRIMARY KEY,"
                              "name TEXT UNIQUE"
                              ");", 0, 0, &errmsg);
        if (rc != SQLITE_OK || errmsg)
        {
            if (errmsg)
            {
                fprintf(stderr, "\n %s() ... %d : %s", __PRETTY_FUNCTION__, rc, errmsg);

                // LOGERR("%d : %s", rc, errmsg);
                sqlite3_free(errmsg);
            }
            else
            {
                fprintf(stderr, "\n %s() ... %d : %s", __PRETTY_FUNCTION__, rc, "(none)");

                // LOGERR("%d", rc);
            }
        }

        if (rc == SQLITE_NOTADB
            && shouldEncrypt
    #if defined(SQLITE_HAS_CODEC)
            && !shouldReKey // re-key should never fail
    #endif
                )
        {
            fprintf(stderr, "\n %s() ... SQLite database is encrypted, but the key doesn't work", __PRETTY_FUNCTION__);

            // LOGWARN("SQLite database is encrypted, but the key doesn't work");
            term();
            if (!fileRemove(filename) || fileExists(filename))
            {
                // LOGERR("Can't remove file");
                return false;
            }
            rc = sqlite3_open(filename, &db);
            term();
            if (rc || !DACutils::fileExists(filename))
            {
              fprintf(stderr, "\n %s() ... SQLite >> Can't create file", __PRETTY_FUNCTION__);
                // LOGERR("Can't create file");
                return false;
            }
            // LOGWARN("SQLite database has been reset, trying re-key");
            return init(filename, key);
        }

        rc = sqlite3_exec(db, "CREATE TABLE if not exists item ("
                              "ns INTEGER,"
                              "key TEXT,"
                              "value TEXT,"
                              "FOREIGN KEY(ns) REFERENCES namespace(id) ON DELETE CASCADE ON UPDATE NO ACTION,"
                              "UNIQUE(ns,key) ON CONFLICT REPLACE"
                              ");", 0, 0, &errmsg);
        if (rc != SQLITE_OK || errmsg)
        {
            if (errmsg)
            {
                fprintf(stderr, "\n %s() ... SQLite >> %d : %s", __PRETTY_FUNCTION__, rc, errmsg);

                // LOGERR("%d : %s", rc, errmsg);
                sqlite3_free(errmsg);
            }
            else
            {
                // LOGERR("%d", rc);
            }
        }

        rc = sqlite3_exec(db, "PRAGMA foreign_keys = ON;", 0, 0, &errmsg);
        if (rc != SQLITE_OK || errmsg)
        {
            if (errmsg)
            {
              fprintf(stderr, "\n %s() ... SQLite >> %d : %s", __PRETTY_FUNCTION__, rc, errmsg);
              // LOGERR("%d : %s", rc, errmsg);
              sqlite3_free(errmsg);
            }
            else
            {
              fprintf(stderr, "\n %s() ... SQLite >> %d : %s", __PRETTY_FUNCTION__, rc, "(none1)");
              // LOGERR("%d", rc);
            }
        }

        return true;
    }


    void DACutils::term()
    {
        // LOGINFO();

        sqlite3* &db = SQLITE;

        if (db)
        {
            sqlite3_close(db);
        }

        db = NULL;
    }

    void DACutils::vacuum()
    {
        // LOGINFO();
        fprintf(stderr, "\n %s() ... ENTER", __PRETTY_FUNCTION__);

        sqlite3* &db = SQLITE;

        if (db)
        {
            char *errmsg;
            int rc = sqlite3_exec(db, "VACUUM", 0, 0, &errmsg);
            if (rc != SQLITE_OK || errmsg)
            {
                if (errmsg)
                {
                    fprintf(stderr, "\n %s() ... SQLite >> %d : %s", __PRETTY_FUNCTION__, rc, errmsg);

                    // LOGERR("%s", errmsg);
                    sqlite3_free(errmsg);
                }
                else
                {
                    fprintf(stderr, "\n %s() ... SQLite >> %d : %s", __PRETTY_FUNCTION__, rc, "(none2)");

                    // LOGERR("%d", rc);
                }
            }
        }
    }

  }  // namespace Plugin
}  // namespace WPEFramework