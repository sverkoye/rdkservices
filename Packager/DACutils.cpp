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

#include <archive.h>
#include <archive_entry.h>

#include <curl/curl.h>
#include <curl/easy.h>

#include <uuid/uuid.h>

#include <limits.h>
#include <sys/types.h>
#include <unistd.h>

#include <string>
#include <fstream>
#include <streambuf>

#include <inttypes.h>
#include <dirent.h>
#include <sys/stat.h>

#include "DACutils.h"
#include "DACinstallerImplementation.h"


#ifndef SQLITE_FILE_HEADER
#define SQLITE_FILE_HEADER "SQLite format 3"
#endif

#define SQLITE *(sqlite3**) &mData

const int64_t WPEFramework::Plugin::DACutils::MAX_SIZE_BYTES = 1000000;
const int64_t WPEFramework::Plugin::DACutils::MAX_VALUE_SIZE_BYTES = 1000;

JsonObject   WPEFramework::Plugin::DACutils::mPackageCfg;

namespace WPEFramework {
namespace Plugin {


PackageInfoEx* DACutils::mThisPkg = nullptr;

std::vector<std::thread>       DACutils::threadPool; // thread pool
WPEFramework::Plugin::JobPool  DACutils::jobPool;


void* WPEFramework::Plugin::DACutils::mData = nullptr;

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

    int64_t DACutils::folderSize(const char *d)
    {
        DIR *dd;
        struct dirent *de;
        struct stat buf;

        int exists;
        int64_t total_size = 0;

        dd = opendir(".");
        if (dd == NULL)
        {
            LOGERR("DACutils::folderSize( %s ) - FAILED", d);
            return -1;
        }

        total_size = 0;

        // Iterate folders and files
        for (de = readdir(dd); de != NULL; de = readdir(dd))
        {
            exists = stat(de->d_name, &buf);
            if (exists < 0)
            {
                fprintf(stderr, "Couldn't stat %s\n", de->d_name);
            }
            else
            {
                total_size += buf.st_size;
            }
        }//FOR

        closedir(dd);

        // LOGERR("DACutils::folderSize( %s )  = %jd", d, total_size);

        return total_size;
    }


    bool iequals(const string& a, const string& b)
    {
        unsigned int sz = a.size();
        if (b.size() != sz)
            return false;
        for (unsigned int i = 0; i < sz; ++i)
            if (tolower(a[i]) != tolower(b[i]))
                return false;
        return true;
    }

    bool DACutils::fileEndsWith(const std::string& f, const std::string& ext)
    {
        std::string::size_type idx = f.rfind('.');

        if(idx != std::string::npos)
        {
            std::string extension = f.substr(idx+1);

            if(iequals(extension, ext) ) //extension.compare(ext))
            {
                return true;
            }
        }

        return false;
    }

    bool DACutils::removeFolder(const string& dirname)
    {
        return DACutils::removeFolder( (const char *) dirname.c_str() );
    }

    bool DACutils::removeFolder(const char *dirname)
    {
        const char *topdir = dirname;

        DIR *dir;
        struct dirent *entry;
        char path[PATH_MAX];

        if (path == nullptr)
        {
            fprintf(stderr, "Out of memory error\n");
            return 0;
        }

        dir = opendir(dirname);
        if (dir == nullptr)
        {
            perror("Error opendir()");
            return 0;
        }

        while ((entry = readdir(dir)) != NULL)
        {
            if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, ".."))
            {
                snprintf(path, (size_t) PATH_MAX, "%s/%s", dirname, entry->d_name);
                if (entry->d_type == DT_DIR)
                {
                    removeFolder(path);
                }

                DACutils::fileRemove(path);
            }
        }//WHILE

        closedir(dir);

        // finally... remove the top folder
        DACutils::fileRemove(topdir);

        return 1;
    }

    std::string DACutils::getGUID()
    {
        uuid_t uuid;
        char uuid_str[37];

        // Generate GUID
        uuid_generate_time_safe(uuid);
        uuid_unparse_lower(uuid, uuid_str);

        return std::string( uuid_str );
    }

    bool DACutils::init(const char* filename, const char* key)
    {
        mThisPkg = Core::Service<PackageInfoEx>::Create<PackageInfoEx>();

        LOGINFO(" %s() ... SQLite >>  filename: %s    key: %s", __PRETTY_FUNCTION__, filename, key);

        sqlite3* &db = SQLITE;

        term(); // ensure closed 

        bool shouldEncrypt = key && *key;
    #if defined(SQLITE_HAS_CODEC)
        bool shouldReKey = shouldEncrypt && 
                           DACutils::fileExists(filename) && 
                          !DACutils::fileEncrypted(filename);
    #endif
        int rc = sqlite3_open(filename, &db);
        if (rc)
        {
            fprintf(stderr, " %s() ... SQLite >>  %d : %s", __PRETTY_FUNCTION__, rc, sqlite3_errmsg(db));

            // LOGERR("%d : %s", rc, sqlite3_errmsg(db));
            term();
            return false;
        }

        // Based on pxCore, Copyright 2015-2018 John Robinson
        // Licensed under the Apache License, Version 2.0 
        if (shouldEncrypt)
        {
    #if defined(SQLITE_HAS_CODEC)
            std::vector<uint8_t> pKey;
    #if defined(USE_PLABELS)

            // NOTE: pbnj_utils stores the nonce under XDG_DATA_HOME/data.
            // If the dir doesn't exist it will fail
            auto path = g_build_filename(g_get_user_data_dir(), "data", nullptr);

            if (!DACutils::fileExists(path))
            {
                g_mkdir_with_parents(path, 0755);
            }
            g_free(path);

            bool result = pbnj_utils::prepareBufferForOrigin(key, [&pKey](const std::vector<uint8_t>& buffer)
            {
                pKey = buffer;
            });

            if (!result)
            {
                // LOGERR("pbnj_utils fail");
                DACutils::term();
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
              fprintf(stderr, " %s() ... Failed to attach encryption key to SQLite", __PRETTY_FUNCTION__);
                // LOGERR("Failed to attach encryption key to SQLite database %s\nCause - %s", filename, sqlite3_errmsg(db));
                DACutils::term();
                return false;
            }

            if (shouldReKey && !fileEncrypted(filename))
            {
                fprintf(stderr, " %s() ... SQLite database file is clear after re-key", __PRETTY_FUNCTION__);

                // LOGERR("SQLite database file is clear after re-key, path=%s", filename);
            }
    #endif
        }

        char *errmsg = nullptr;

        // rc = sqlite3_exec(db, "CREATE TABLE if not exists namespace ("
        //                       "id INTEGER PRIMARY KEY,"
        //                       "name TEXT UNIQUE"
        //                       ");", 0, 0, &errmsg);

        // if (rc != SQLITE_OK || errmsg)
        // {
        //     if (errmsg)
        //     {
        //         fprintf(stderr, " %s() ... %d : %s", __PRETTY_FUNCTION__, rc, errmsg);

        //         // LOGERR("%d : %s", rc, errmsg);
        //         sqlite3_free(errmsg);
        //     }
        //     else
        //     {
        //         fprintf(stderr, " %s() ... %d : %s", __PRETTY_FUNCTION__, rc, "(none)");

        //         // LOGERR("%d", rc);
        //     }
        // }

    //     if (rc == SQLITE_NOTADB
    //         && shouldEncrypt
    // #if defined(SQLITE_HAS_CODEC)
    //         && !shouldReKey // re-key should never fail
    // #endif
    //             )
    //     {
    //         fprintf(stderr, " %s() ... SQLite database is encrypted, but the key doesn't work", __PRETTY_FUNCTION__);

    //         // LOGWARN("SQLite database is encrypted, but the key doesn't work");
    //         DACutils::term();

    //         if (!fileRemove(filename) || fileExists(filename))
    //         {
    //             // LOGERR("Can't remove file");
    //             return false;
    //         }

    //         rc = sqlite3_open(filename, &db);
    //         DACutils::term();

    //         if (rc || !DACutils::fileExists(filename))
    //         {
    //           fprintf(stderr, " %s() ... SQLite >> Can't create file", __PRETTY_FUNCTION__);
    //             // LOGERR("Can't create file");
    //             return false;
    //         }
    //         // LOGWARN("SQLite database has been reset, trying re-key");
    //         return DACutils::init(filename, key);
    //     }

        if ( createTable() == false)
        {
            LOGERR(" %s() ... SQLite >> createTable() .... FAILED ! \n", __PRETTY_FUNCTION__);
            return false;
        }

        rc = sqlite3_exec(db, "PRAGMA foreign_keys = ON;", 0, 0, &errmsg);

        if (rc != SQLITE_OK || errmsg)
        {
            if (errmsg)
            {
              LOGERR(" %s() ... SQLite >> %d : %s \n", __PRETTY_FUNCTION__, rc, errmsg);
              // LOGERR("%d : %s", rc, errmsg);
              sqlite3_free(errmsg);
            }
            else
            {
              LOGERR(" %s() ... SQLite >> %d : %s \n", __PRETTY_FUNCTION__, rc, "(none1)");
              // LOGERR("%d", rc);
            }
        }

        return true;
    }

    bool DACutils::createTable()
    {
        char   *errmsg;
        sqlite3* &db = SQLITE;

        int rc = sqlite3_exec(db, "CREATE TABLE if not exists dacTable ("
                                    "name TEXT NOT NULL,"
                                    "bundlePath TEXT NOT NULL,"
                                    "version TEXT NOT NULL,"
                                    "id TEXT PRIMARY KEY NOT NULL,"
                                    "installed TEXT NOT NULL,"
                                    "size INT NOT NULL,"
                                    "type TEXT NOT NULL"
                                    // "FOREIGN KEY(ns) REFERENCES namespace(id) ON DELETE CASCADE ON UPDATE NO ACTION,"
                                    // "UNIQUE(ns,key) ON CONFLICT REPLACE"
                                    ");", 0, 0, &errmsg);

        if (rc != SQLITE_OK || errmsg)
        {
            if (errmsg)
            {
                LOGERR(" %s() ... SQLite >> %d : %s", __PRETTY_FUNCTION__, rc, errmsg);

                // LOGERR("%d : %s", rc, errmsg);
                sqlite3_free(errmsg);
                return false;
            }
        }
        else
        {
             fprintf(stderr, "\n TABLE READY \n");

            //  DACutils::showTable();
        }

       // DACutils::term(); // ensure closed 
    
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
        fprintf(stderr, " %s() ... ENTER", __PRETTY_FUNCTION__);

        sqlite3* &db = SQLITE;

        if (db)
        {
            char *errmsg;
            int rc = sqlite3_exec(db, "VACUUM", 0, 0, &errmsg);
            if (rc != SQLITE_OK || errmsg)
            {
                if (errmsg)
                {
                    fprintf(stderr, " %s() ... SQLite >> %d : %s", __PRETTY_FUNCTION__, rc, errmsg);

                    // LOGERR("%s", errmsg);
                    sqlite3_free(errmsg);
                }
                else
                {
                    fprintf(stderr, " %s() ... SQLite >> %d : %s", __PRETTY_FUNCTION__, rc, "(none2)");
                    // LOGERR("%d", rc);
                }
            }
        }
    }

    bool DACutils::hasPkgRow(const string& pkgId)
    {
        bool success = false;

        sqlite3* &db = SQLITE;

        if (db)
        {
            sqlite3_stmt *stmt;
            sqlite3_prepare_v2(db, "SELECT id"
                                    " FROM dacTable"
                                    " where id = ?"
                                    ";", -1, &stmt, nullptr);

            // SELECT this 'pkgId'
            //
            int rc = sqlite3_bind_text(stmt, 1, pkgId.c_str(), -1, SQLITE_TRANSIENT);

            // excute SQL
            rc = sqlite3_step(stmt);
            if (rc == SQLITE_ROW) // FOUND !!
            {
                success = true;
            }
            else
            {
                LOGWARN("ERROR:  hasPkgRow() ...  %s not found ", pkgId.c_str());
            }

            sqlite3_finalize(stmt);
        }

        return success;
    }

    bool DACutils::hasPkgRow(const char* pkgId)
    {
        return DACutils::hasPkgRow(string(pkgId));
    }

    bool DACutils::addPkgRow(const PackageInfoEx* pkg)
    {
        if(pkg == nullptr)
        {
            LOGERR(" %s() ...  Bad Args...  NULL", __PRETTY_FUNCTION__);
            return false;
        }
    
LOGINFO(" %s() ... Adding row for '%s'... ", __PRETTY_FUNCTION__, pkg->Name().c_str());

        bool success = false;

        sqlite3* &db = SQLITE;

        // Check size of addition
        //
        // if (db)
        // {
        //     sqlite3_stmt *stmt;
        //     sqlite3_prepare_v2(db, "SELECT sum(s) FROM ("
        //                            " SELECT sum(length(key)+length(value)) s FROM item"
        //                            " UNION ALL"
        //                            " SELECT sum(length(name)) s FROM namespace"
        //                            ");", -1, &stmt, nullptr);

        //     if (sqlite3_step(stmt) == SQLITE_ROW)
        //     {
        //         int64_t size = sqlite3_column_int64(stmt, 0);
        //         if (size > MAX_SIZE_BYTES)
        //         {
        //             // LOGWARN("max size exceeded: %lld", size);
        //         }
        //         else
        //         {
        //             success = true;
        //         }
        //     }
        //     else
        //     {
        //     // LOGERR("ERROR getting size: %s", sqlite3_errmsg(db));
        //     }

        //     sqlite3_finalize(stmt);
        // }

        // if (success)
        // {
        //     success = false;

        //     sqlite3_stmt *stmt;
        //     sqlite3_prepare_v2(db, "INSERT OR IGNORE INTO dacTable (id) VALUES (?);", -1, &stmt, nullptr);

        //     sqlite3_bind_text(stmt, 1, ns.c_str(), -1, SQLITE_TRANSIENT);


        //     int rc = sqlite3_step(stmt);
        //     if (rc != SQLITE_DONE)
        //     {
        //      // LOGERR("ERROR inserting data: %s", sqlite3_errmsg(db));
        //     }
        //     else
        //     {
        //         success = true;
        //     }

        //     sqlite3_finalize(stmt);
        // }

success = true;

        if (success)
        {
            LOGINFO("INFO inserting data ...");

            success = false;

            sqlite3_stmt *stmt;
            int result = sqlite3_prepare_v2(db, "INSERT INTO dacTable (name, bundlePath, version, id, installed, size, type)  VALUES (?,?,?,?,?,?,?)", -1, &stmt, nullptr);

            if (result != SQLITE_OK)
            {
                LOGERR("ERROR  >>> sqlite3_prepare_v2() ... %s", sqlite3_errmsg(db));
                sqlite3_finalize(stmt);
                return result;
            }

            if ((result = sqlite3_bind_text(stmt, 1, pkg->Name().c_str(), -1, SQLITE_TRANSIENT) ) != SQLITE_OK)
            {
                LOGERR("ERROR inserting data ... Name: '%s' ... #1 ... %s", pkg->Name().c_str(), sqlite3_errmsg(db));
                sqlite3_finalize(stmt);
                return result;
            }

            if ((result = sqlite3_bind_text(stmt, 2, pkg->BundlePath().c_str(), -1, SQLITE_TRANSIENT) ) != SQLITE_OK)
            {
                LOGERR("ERROR inserting data ... BundlePath ... #2 ... %s", sqlite3_errmsg(db));
                sqlite3_finalize(stmt);
                return result;
            }

            if ((result = sqlite3_bind_text(stmt, 3, pkg->Version().c_str(), -1, SQLITE_TRANSIENT) ) != SQLITE_OK)
            {
                LOGERR("ERROR inserting data ... Version ... #3 ... %s", sqlite3_errmsg(db));
                sqlite3_finalize(stmt);
                return result;
            }

            if ((result = sqlite3_bind_text(stmt, 4, pkg->PkgId().c_str(), -1, SQLITE_TRANSIENT) ) != SQLITE_OK)
            {
                LOGERR("ERROR inserting data ... PkgId ... #4 ... %s", sqlite3_errmsg(db));
                sqlite3_finalize(stmt);
                return result;
            }
                        
            if ((result = sqlite3_bind_text(stmt, 5, pkg->Installed().c_str(), -1, SQLITE_TRANSIENT) ) != SQLITE_OK)
            {
                LOGERR("ERROR inserting data ... Installed ... #5 ... %s", sqlite3_errmsg(db));
                sqlite3_finalize(stmt);
                return result;
            }

            if ((result = sqlite3_bind_int( stmt, 6, pkg->SizeInBytes()) ) != SQLITE_OK)
            {
                LOGERR("ERROR inserting data ... SizeInBytes ... #6 ... %s", sqlite3_errmsg(db));
                sqlite3_finalize(stmt);
                return result;
            }

            if ((result = sqlite3_bind_text(stmt, 7, pkg->Type().c_str(), -1, SQLITE_TRANSIENT) ) != SQLITE_OK)
            {
                LOGERR("ERROR inserting data ... Type ... #7 ... %s  val: %s", sqlite3_errmsg(db), pkg->Type().c_str() );
                sqlite3_finalize(stmt);
                return result;
            }

            int rc = sqlite3_step(stmt);
            if (rc != SQLITE_DONE)
            {
                LOGERR("ERROR inserting data: %s", sqlite3_errmsg(db));
            }
            else
            {
                success = true;
            }

            sqlite3_finalize(stmt);
        }

//         if (success)
//         {
//             success = false;

//             sqlite3_stmt *stmt;
//             sqlite3_prepare_v2(db, "SELECT sum(s) FROM ("
//                                     " SELECT sum(length(key)+length(value)) s FROM item"
//                                     " UNION ALL"
//                                     " SELECT sum(length(name)) s FROM namespace"
//                                     ");", -1, &stmt, nullptr);

//             if (sqlite3_step(stmt) == SQLITE_ROW)
//             {
//                 int64_t size = sqlite3_column_int64(stmt, 0);
//                 if (size > MAX_SIZE_BYTES)
//                 {
//                     // LOGWARN("max size exceeded: %lld", size);

// // TODO: Fixme
// // TODO: Fixme

//                     // JsonObject params;
//                     // sendNotify(C_STR(EVT_ON_STORAGE_EXCEEDED), params);
//                 }
//                 else
//                 {
//                     success = true;
//                 }
//             }
//             else
//             {
//                 // LOGERR("ERROR getting size: %s", sqlite3_errmsg(db));
//             }

//             sqlite3_finalize(stmt);
//         }

        return success;
    }

    PackageInfoEx* DACutils::getPkgRow(const string& pkgId)
    {
        sqlite3* &db = SQLITE;

        if (db)
        {
            sqlite3_stmt *stmt;
            sqlite3_prepare_v2(db, "SELECT name, bundlePath, version, id, installed, size, type"
                                    " FROM dacTable"
                                    " where id = ?"
                                    ";", -1, &stmt, nullptr);

            // SELECT this 'pkgId'
            //
            int rc = sqlite3_bind_text(stmt, 1, pkgId.c_str(), -1, SQLITE_TRANSIENT);

            // excute SQL
            rc = sqlite3_step(stmt);
            if (rc == SQLITE_ROW) // FOUND !!
            {
                mThisPkg->setName(        sqlite3_column_text(stmt, 0) ); // name
                mThisPkg->setBundlePath(  sqlite3_column_text(stmt, 1) ); // path
                mThisPkg->setVersion(     sqlite3_column_text(stmt, 2) ); // version
                mThisPkg->setPkgId(       sqlite3_column_text(stmt, 3) ); // id
                mThisPkg->setInstalled(   sqlite3_column_text(stmt, 4) ); // installed
                mThisPkg->setSizeInBytes( sqlite3_column_int( stmt, 5) ); // size (bytes)
                mThisPkg->setType(        sqlite3_column_text(stmt, 6) ); // type

                PackageInfoEx::printPkg(mThisPkg);
#if 0

        LOGWARN(">>>>>>>  Name()       ... %s", mThisPkg->Name()       .c_str() );
        LOGWARN(">>>>>>>  BundlePath() ... %s", mThisPkg->BundlePath() .c_str() );
        LOGWARN(">>>>>>>  Version()    ... %s", mThisPkg->Version()    .c_str() );
        LOGWARN(">>>>>>>  PkgId()      ... %s", mThisPkg->PkgId()      .c_str() );
        LOGWARN(">>>>>>>  Installed()  ... %s", mThisPkg->Installed()  .c_str() );
        LOGWARN(">>>>>>>  Size()       ... %d", mThisPkg->SizeInBytes()         );
        LOGWARN(">>>>>>>  Type()       ... %s", mThisPkg->Type()       .c_str() );

#endif //0
            }
            else
            {
                LOGWARN("ERROR:  getPkgRow() ...  %s not found: %d", pkgId.c_str(), rc);
            }

            sqlite3_finalize(stmt);
        }

        return mThisPkg;
    }


    bool DACutils::delPkgRow(const string& pkgId)
    {
        if(pkgId.empty()) // passed empty string ?
        {
            LOGERR("... delPkgRow() - passed empty ID ");
            return false;
        }

        bool success = false;

        sqlite3* &db = SQLITE;

        if (db)
        {
            sqlite3_stmt *stmt;
            sqlite3_prepare_v2(db, "DELETE FROM dacTable"
                                    " where id = (?)"
                                    ";", -1, &stmt, NULL);

            sqlite3_bind_text(stmt, 1, pkgId.c_str(), -1, SQLITE_TRANSIENT);

            int rc = sqlite3_step(stmt);
            if (rc != SQLITE_DONE)
            {
                LOGERR("ERROR removing data: %s", sqlite3_errmsg(db));
            }
            else
            {
                LOGINFO("INFO removed data: %s", pkgId.c_str());
                success = true;
            }

            sqlite3_finalize(stmt);
        }
        return success;
    }

    static int showCallback(void *NotUsed, int argc, char **argv, char **azColName)
    {
        fprintf(stderr, "\n showTable() - showCallback \n");

        // int argc: holds the number of results
        // (array) azColName: holds each column returned
        // (array) argv: holds each value
        for(int i = 0; i < argc; i++)
        {
            // Show column name, value, and newline
            cout << azColName[i] << ": " << argv[i] << endl;
        }

        // Insert a newline
        cout << endl;

        // Return successful
        return 0;
    }

    void DACutils::showTable()
    {
        fprintf(stderr, "\n showTable() - ENTER \n");

        char *zErrMsg;
        sqlite3* &db = SQLITE;

        // Save SQL insert data
        char sql[] = "SELECT * FROM 'dacTable';";

        // Run the SQL
        int rc = sqlite3_exec(db, &sql[0], showCallback, 0, &zErrMsg);

        if (rc != SQLITE_OK)
        {
            LOGERR("ERROR showing table ... %s", sqlite3_errmsg(db));
        }
    }

    int64_t DACutils::sumSizeInBytes()
    {
        sqlite3* &db = SQLITE;
        int64_t total = 0;

        if (db)
        {
            sqlite3_stmt *stmt;
            sqlite3_prepare_v2(db, "SELECT SUM(size) FROM dacTable;", -1, &stmt, nullptr);

            // excute SQL
            int rc = sqlite3_step(stmt);
            if (rc == SQLITE_ROW) // FOUND !!
            {
                total = sqlite3_column_int(stmt, 0);
//                LOGWARN("ERROR:  sumSizeInBytes() ...  %ld bytes   rc: %d", total, rc);
            }
            else
            {
                LOGWARN("ERROR:  sumSizeInBytes() ...   sqlite3_step   bad: %d", rc);
            }

            sqlite3_finalize(stmt);
        }

        return total;
    }


#if 0 // JUNK
    bool DACutils::setValue(const string& ns, const string& key, const string& value)
    {
        // LOGINFO("%s %s %s", ns.c_str(), key.c_str(), value.c_str());

        bool success = false;

        sqlite3* &db = SQLITE;

        if (db)
        {
            sqlite3_stmt *stmt;
            sqlite3_prepare_v2(db, "SELECT sum(s) FROM ("
                                   " SELECT sum(length(key)+length(value)) s FROM item"
                                   " UNION ALL"
                                   " SELECT sum(length(name)) s FROM namespace"
                                   ");", -1, &stmt, nullptr);

            if (sqlite3_step(stmt) == SQLITE_ROW)
            {
                int64_t size = sqlite3_column_int64(stmt, 0);
                if (size > MAX_SIZE_BYTES)
                {
                    // LOGWARN("max size exceeded: %lld", size);
                }
                else
                {
                    success = true;
                }
            }
            else
            {
            // LOGERR("ERROR getting size: %s", sqlite3_errmsg(db));
            }

            sqlite3_finalize(stmt);
        }

        if (success)
        {
            success = false;

            sqlite3_stmt *stmt;
            sqlite3_prepare_v2(db, "INSERT OR IGNORE INTO namespace (name) values (?);", -1, &stmt, nullptr);

            sqlite3_bind_text(stmt, 1, ns.c_str(), -1, SQLITE_TRANSIENT);

            int rc = sqlite3_step(stmt);
            if (rc != SQLITE_DONE)
            {
             // LOGERR("ERROR inserting data: %s", sqlite3_errmsg(db));
            }
            else
            {
                success = true;
            }

            sqlite3_finalize(stmt);
        }

        if (success)
        {
            success = false;

            sqlite3_stmt *stmt;
            sqlite3_prepare_v2(db, "INSERT INTO item (ns,key,value)"
                                   " SELECT id, ?, ?"
                                   " FROM namespace"
                                   " WHERE name = ?"
                                   ";", -1, &stmt, nullptr);

            sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, value.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 3, ns.c_str(), -1, SQLITE_TRANSIENT);

            int rc = sqlite3_step(stmt);
            if (rc != SQLITE_DONE)
            {
                // LOGERR("ERROR inserting data: %s", sqlite3_errmsg(db));
            }
            else
            {
                success = true;
            }

            sqlite3_finalize(stmt);
        }

        if (success)
        {
            success = false;

            sqlite3_stmt *stmt;
            sqlite3_prepare_v2(db, "SELECT sum(s) FROM ("
                                    " SELECT sum(length(key)+length(value)) s FROM item"
                                    " UNION ALL"
                                    " SELECT sum(length(name)) s FROM namespace"
                                    ");", -1, &stmt, nullptr);

            if (sqlite3_step(stmt) == SQLITE_ROW)
            {
                int64_t size = sqlite3_column_int64(stmt, 0);
                if (size > MAX_SIZE_BYTES)
                {
                    // LOGWARN("max size exceeded: %lld", size);

                    // JsonObject params;
                    // sendNotify(C_STR(EVT_ON_STORAGE_EXCEEDED), params);
                }
                else
                {
                    success = true;
                }
            }
            else
            {
                // LOGERR("ERROR getting size: %s", sqlite3_errmsg(db));
            }

            sqlite3_finalize(stmt);
        }

        return success;
    }

    bool DACutils::getValue(const string& ns, const string& key, string& value)
    {
        // LOGINFO("%s %s", ns.c_str(), key.c_str());

        bool success = false;

        sqlite3* &db = SQLITE;

        if (db)
        {
            sqlite3_stmt *stmt;
            sqlite3_prepare_v2(db, "SELECT value"
                                    " FROM item"
                                    " INNER JOIN namespace ON namespace.id = item.ns"
                                    " where name = ? and key = ?"
                                    ";", -1, &stmt, nullptr);

            sqlite3_bind_text(stmt, 1, ns.c_str(),  -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, key.c_str(), -1, SQLITE_TRANSIENT);

            int rc = sqlite3_step(stmt);
            if (rc == SQLITE_ROW)
            {
                value = (const char*)sqlite3_column_text(stmt, 0);
                success = true;
            }
            else
            {
              //  LOGWARN("not found: %d", rc);
            }
            sqlite3_finalize(stmt);
        }

        return success;
    }

    bool DACutils::deleteKey(const string& ns, const string& key)
    {
        // LOGINFO("%s %s", ns.c_str(), key.c_str());

        bool success = false;

        sqlite3* &db = SQLITE;

        if (db)
        {
            sqlite3_stmt *stmt;
            sqlite3_prepare_v2(db, "DELETE FROM item"
                                    " where ns in (select id from namespace where name = ?)"
                                    " and key = ?"
                                    ";", -1, &stmt, NULL);

            sqlite3_bind_text(stmt, 1, ns.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, key.c_str(), -1, SQLITE_TRANSIENT);

            int rc = sqlite3_step(stmt);
            if (rc != SQLITE_DONE)
            {
                // LOGERR("ERROR removing data: %s", sqlite3_errmsg(db));
            }
            else
            {
                success = true;
            }

            sqlite3_finalize(stmt);
        }

        return success;
    }

    bool DACutils::deleteNamespace(const string& ns)
    {
        // LOGINFO("%s", ns.c_str());

        bool success = false;

        sqlite3* &db = SQLITE;

        if (db)
        {
            sqlite3_stmt *stmt;
            sqlite3_prepare_v2(db, "DELETE FROM namespace where name = ?;", -1, &stmt, NULL);

            sqlite3_bind_text(stmt, 1, ns.c_str(), -1, SQLITE_TRANSIENT);

            int rc = sqlite3_step(stmt);
            if (rc != SQLITE_DONE)
            {
                // LOGERR("ERROR removing data: %s", sqlite3_errmsg(db));
            }
            else
            {
                success = true;
            }

            sqlite3_finalize(stmt);
        }

        return success;
    }
#endif // 0 JUNK

    // ARCHIVE CODE
    static int
    copy_data(struct archive *ar, struct archive *aw)
    {
      int r;
      const void *buff;
      size_t size;
    #if ARCHIVE_VERSION_NUMBER >= 3000000
      int64_t offset;
    #else
      off_t offset;
    #endif

      for (;;)
      {
        r = archive_read_data_block(ar, &buff, &size, &offset);
        if (r == ARCHIVE_EOF)
        {
          return (ARCHIVE_OK);
        }
        if (r != ARCHIVE_OK)
        {
          return (r);
        }
        r = archive_write_data_block(aw, buff, size, offset);
        if (r != ARCHIVE_OK)
        {
          // warn("archive_write_data_block()",
          //     archive_error_string(aw));
          return (r);
        }
      }
    }


    DACutils::DACrc_t DACutils::extract(const char *filename, const char *to_path /* = nullptr */)
    {
      struct archive *a;
      struct archive *ext;
      struct archive_entry *entry;
      int flags;
      int r;

fprintf(stderr, " %s() ... Extracting >>>  %s\n", __PRETTY_FUNCTION__, filename);

      // Select which attributes we want to restore. 
      flags =  ARCHIVE_EXTRACT_TIME;
      flags |= ARCHIVE_EXTRACT_PERM;
      flags |= ARCHIVE_EXTRACT_ACL;
      flags |= ARCHIVE_EXTRACT_FFLAGS;

      a = archive_read_new();
      archive_read_support_format_all(a);
    //   archive_read_support_compression_all(a); // DEPRECATED ?
      archive_read_support_filter_all(a);

      ext = archive_write_disk_new();

      archive_write_disk_set_options(ext, flags);
      archive_write_disk_set_standard_lookup(ext);

      if ((r = archive_read_open_filename(a, filename, 10240)))
      {
        fprintf(stderr, " EXTRACT >>>  FATAL - '%s' NOT found.", filename);
        return DACrc_t::dac_FAIL;
      }

      for (;;)
      {
        r = archive_read_next_header(a, &entry);
        if (r == ARCHIVE_EOF)
        {
          break; // complete
        }
        if (r < ARCHIVE_OK)
        {
          fprintf(stderr, "%s\n", archive_error_string(a));
        }

        if (r < ARCHIVE_WARN)
        {
            fprintf(stderr, " %s() ... ERROR:   Next Header ... Unexpected > ARCHIVE_WARN\n", __PRETTY_FUNCTION__);
            return DACrc_t::dac_FAIL;
        }

        if(to_path != nullptr)
        {
            std::string targetFilepath(to_path);// = "/opt/"; 
            targetFilepath += archive_entry_pathname(entry); 

            archive_entry_set_pathname(entry, targetFilepath.c_str()); 

//            fprintf(stderr, " EXTRACT >>>  entry: %s", targetFilepath.c_str());
        }

        r = archive_write_header(ext, entry);
        if (r < ARCHIVE_OK)
        {
          fprintf(stderr, "%s\n", archive_error_string(ext));
        }
        else if (archive_entry_size(entry) > 0)
        {
          r = copy_data(a, ext);
          if (r < ARCHIVE_OK)
          {
            fprintf(stderr, "%s\n", archive_error_string(ext));
          }

          if (r < ARCHIVE_WARN)
          {
            fprintf(stderr, " %s() ... ERROR:   Entry Size ... Unexpected > ARCHIVE_WARN\n", __PRETTY_FUNCTION__);              
            return DACrc_t::dac_FAIL;
          }
        }

        r = archive_write_finish_entry(ext);
        if (r < ARCHIVE_OK)
        {
          fprintf(stderr, "%s\n", archive_error_string(ext));
        }
        if (r < ARCHIVE_WARN)
        {
           fprintf(stderr, " %s() ... ERROR:   Write Finish ... Unexpected > ARCHIVE_WARN\n", __PRETTY_FUNCTION__);              
           return DACrc_t::dac_FAIL;
        }
      }

      archive_read_close(a);
      archive_read_free(a);
      archive_write_close(ext);
      archive_write_free(ext);

      return DACrc_t::dac_OK; // success
    }

    void example_function()
    {
        std::cout << "bla" << std::endl;
    }

    void DACutils::setupThreadQ()
    {
        int num_threads = std::thread::hardware_concurrency() / 2; // be nice

        fprintf(stderr, " %s() ... hardware_concurrency()  tt: %d \n", __PRETTY_FUNCTION__, num_threads);

        for (int i = 0; i < num_threads; i++)
        {
            fprintf(stderr, " %s() ... Starting Worker()  i: %d \n", __PRETTY_FUNCTION__, i);
            threadPool.push_back(std::thread(&JobPool::worker_func, &jobPool));
        }
    }

    void DACutils::killThreadQ()
    {
        DACutils::jobPool.done();

        // Kill workers
        for (unsigned int i = 0; i < DACutils::threadPool.size(); i++)
        {
            fprintf(stderr, " %s() ... Killing WORKER  i: %d\n", __PRETTY_FUNCTION__, i);
            DACutils::threadPool.at(i).join();
        }
    }

    void DACutils::addJob( )
    {
        // here we should send our jobs
        for (int i = 0; i < 10; i++)
        {
            fprintf(stderr, " %s() ... Adding JOB  i: %d \n", __PRETTY_FUNCTION__, i);
            jobPool.push(example_function);
        }
    }

    size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream)
    {
        size_t written;
        written = fwrite(ptr, size, nmemb, stream);
        return written;
    }

    DACutils::DACrc_t DACutils::downloadJSON(const char *url)
    {
        // Always cleanup
        DACutils::fileRemove(TMP_FILENAME);

        // Download JSON manifest...
        //
        if(downloadURL(url) == DACrc_t::dac_OK)
        {
            // Read entire JSON text file...
            std::ifstream t(TMP_FILENAME);
            std::string txt((std::istreambuf_iterator<char>(t)),
                             std::istreambuf_iterator<char>());

            // Parse the text to JSON ... and get install URL.
            mPackageCfg    = JsonObject(txt);

            return DACrc_t::dac_OK;
        }

         return DACrc_t::dac_FAIL;
    }

    DACutils::DACrc_t DACutils::downloadURL(const char *url)
    {
      CURL *curl;
      FILE *fp;
      CURLcode res;

      // Always cleanup
      DACutils::fileRemove(TMP_FILENAME);

      DACutils::DACrc_t rc = DACrc_t::dac_FAIL;

      fprintf(stderr, " %s() ... download: %s \n", __PRETTY_FUNCTION__, url);

      curl = curl_easy_init();
      if (curl)
      {
          fp = fopen(TMP_FILENAME,"wb");

          curl_easy_setopt(curl, CURLOPT_URL, url);
          curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
          curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
          
          res = curl_easy_perform(curl);
          if(res == CURLE_OK)
          {
              rc = DACrc_t::dac_OK;
              /// queue download for install
          }

          curl_easy_cleanup(curl);

          fclose(fp);
      }

      return rc;
    }

    //================================================================================================

    JobPool::JobPool() : 
        m_JobQ(), m_lock(), m_data_condition(), m_accept_functions(true)
    {
    }

    JobPool::~JobPool()
    {
    }

    void JobPool::push(std::function<void()> func)
    {
        std::unique_lock<std::mutex> lock(m_lock);
        m_JobQ.push(func);

        // when we send the notification immediately, 
        // the consumer will try to get the lock , so unlock asap

        lock.unlock();
        m_data_condition.notify_one();
    }

    void JobPool::done()
    {
        std::unique_lock<std::mutex> lock(m_lock);
        m_accept_functions = false;
        lock.unlock();

        // when we send the notification immediately, 
        // the consumer will try to get the lock , so unlock asap
        m_data_condition.notify_all();
      
        //notify all waiting threads.
    }

    void JobPool::worker_func()
    {
        std::function<void()> func;
        while (true)
        {
            {
                std::unique_lock<std::mutex> lock(m_lock);
                m_data_condition.wait(lock, [this]()
                {
                  return !m_JobQ.empty() || !m_accept_functions;
                });

                if (!m_accept_functions && m_JobQ.empty())
                {
                    //lock will be release automatically.
                    //finish the thread loop and let it join in the main thread.
                    return;
                }

                func = m_JobQ.front();
                m_JobQ.pop();
                //release the lock
            }
            func();
        }
    }

  }  // namespace Plugin
}  // namespace WPEFramework