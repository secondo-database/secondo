/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

//paragraph [1] Title: [{\Large \bf ] [}]

[1] Implementation of the Storage Management Interface for UrelAlgebra

December 2009, Jiamin Lu

1 Includes, Constants

*/


#include <string.h>
#include <string>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <sstream>
#include <iostream>
#include <fstream>
#include <cassert>
#include <limits.h>

//#define TRACE_ON
#include "LogMsg.h"

#include <db_cxx.h>
#include "SecondoSMI.h"
#include "SmiBDB.h"
#include "SmiCodes.h"
#include "Profiles.h"
#include "CharTransform.h"
#include "Counter.h"
#include "WinUnix.h"
#include "DbVersion.h"

using namespace std;

//Some default variables
const int defaultPageSize = 1024;
const string::size_type updatePathLength = 1024;
//the length of the prefix path of a file
const int defaultSysPageNum = 1;
//number of pages for maintain system information


//Copy from bdbFile.cpp
static void BdbInitCatalogEntry(SmiCatalogEntry& entry)
{
  memset(entry.fileName, 0, (2 * SMI_MAX_NAMELEN + 2));
}

namespace {

/*
~SysPageLock~ serializes the read-modify-write of the shared-process counter
stored on a memory pool file's system page across processes. In the multi-user
modes the Berkeley DB environment has its lock subsystem enabled
(DB\_INIT\_LOCK), so a short exclusive lock on an object derived from the file
name prevents concurrent ~RegisterInFile~ / ~UnRegisterInFile~ calls from
different processes from losing an update. In single-user (private)
environments there is a single process and no lock subsystem, so the guard is a
no-op. Failing to obtain the lock is reported but non-fatal: the operation then
proceeds unsynchronized, as it did before.

*/
class SysPageLock
{
  public:
    SysPageLock(DbEnv* dbenv, bool active, const std::string& resource)
      : env(active ? dbenv : 0), locker(0), haveLocker(false), held(false)
    {
      if (env == 0)
        return;
      int rc = env->lock_id(&locker);
      if (rc != 0)
        {
          SmiEnvironment::SetBDBError(rc);
          env = 0;
          return;
        }
      haveLocker = true;
      Dbt obj((void*) resource.data(), (u_int32_t) resource.size());
      rc = env->lock_get(locker, 0, &obj, DB_LOCK_WRITE, &lock);
      if (rc != 0)
        SmiEnvironment::SetBDBError(rc);
      else
        held = true;
    }

    ~SysPageLock()
    {
      if (held)
        {
          int rc = env->lock_put(&lock);
          SmiEnvironment::SetBDBError(rc);
        }
      if (haveLocker)
        {
          int rc = env->lock_id_free(locker);
          SmiEnvironment::SetBDBError(rc);
        }
    }

  private:
    SysPageLock(const SysPageLock&);
    SysPageLock& operator=(const SysPageLock&);

    DbEnv*    env;
    u_int32_t locker;
    bool      haveLocker;
    bool      held;
    DbLock    lock;
};

}  // anonymous namespace

/*

2 Implementation of class SmiUpdateFile

*/
SmiUpdateFile::SmiUpdateFile() :
    dbMpf(0), poolPageSize(defaultPageSize),
    sysPageNum(defaultSysPageNum),
    existPageNum(0), isInitialized(false)
{
  opened = false;
}

SmiUpdateFile::SmiUpdateFile(SmiSize _ps) :
  dbMpf(0), poolPageSize((_ps > 0) ? _ps : defaultPageSize),
  sysPageNum(defaultSysPageNum), existPageNum(0), isInitialized(false)
{
  opened = false;
}


SmiUpdateFile::~SmiUpdateFile()
{}

/*

2.1 Create a new Smi Update File.

*/
bool SmiUpdateFile::Create(const string& context, uint16_t pageSize)
{
  //assert(!opened);
  static long& ctr = Counter::getRef("SmiFile::Open");
  int rc = 0;

  //The context doesn't useful here,
  //just for keeping the compatibility.
  if (CheckName(context))
    {
      fileId = SmiEnvironment::Implementation::GetFileId();

      if (fileId != 0)
        {
          string bdbName = SmiEnvironment::Implementation::
              ConstructFileName(fileId);
          fileName = bdbName;

          DbEnv* dbenv = SmiEnvironment::instance.impl->bdbEnv;
          rc = dbenv->memp_fcreate(&dbMpf, 0);
          SmiEnvironment::SetBDBError(rc);

          u_int32_t flags = 0;
          int mode = 0;

          InitializePoolFile();
          rc = dbMpf->open(fileName.c_str(), flags,
              mode, poolPageSize);
          SmiEnvironment::SetBDBError(rc);
          existPageNum = GetFactPageNum();

          if (rc == 0)
            {
              RegisterInFile();
              opened = true;
              ctr++;
            }
          else
            {
              SmiEnvironment::SetBDBError(rc);
            }
        }
      else
        {
          rc = E_SMI_FILE_NOFILEID;
          SmiEnvironment::SetError( E_SMI_FILE_NOFILEID );
        }
    }
  else
    {
      rc = E_SMI_FILE_INVALIDNAME;
      SmiEnvironment::SetError( E_SMI_FILE_INVALIDNAME );
    }

  //TODO --T
  cout << "The file: " << fileName << "is created" << endl;
  return (rc == 0);
}

/*

2.2 Create and open an memory pool file.

This method is only used in test case. Open a page-oriented file
called ~name~ as a memory pool file. If the file doesn't exist,
create and initiate it.

*/
bool SmiUpdateFile::Open(const string& name)
{
  //assert(!opened);
  static long& ctr = Counter::getRef("SmiFile::Open");
  int rc = 0;
  bool existing = false;
/*

The isTemporaryFile in SmiFile::Implementation is false by default,
create an empty Db handle bdbFile with SmiFile::Implementation.

*/
  if (impl->isTemporaryFile)
    {
      rc = E_SMI_FILE_ISTEMP;
      SmiEnvironment::SetError(E_SMI_FILE_ISTEMP);
    }
  else if (CheckName(name))
    {
      //Get fileID
      SmiCatalogEntry entry;
      if (SmiEnvironment::Implementation::LookUpCatalog(name, entry))
        {
          // Find this page-oriented file in catalog database
          fileId = entry.fileId;
          existing = true;
        }
      else
        {
          //Check whether a file with the given name was created
          //earlier within the enclosing transaction
          map<string, SmiCatalogFilesEntry>::iterator it =
              SmiEnvironment::instance.impl->
              bdbFilesToCatalog.find(name);
          if (it != SmiEnvironment::instance.impl->
              bdbFilesToCatalog.end()
              && it->second.updateOnCommit)
            {
              //Find this file in ~bdbFilesToCatalog~ array
              fileId = it->second.entry.fileId;
              existing = true;
            }
          else
            {
              //Create a new fileId which start from 1.
              fileId = SmiEnvironment::Implementation::GetFileId();
            }
        }

      if (fileId != 0)
        {
          string bdbName = SmiEnvironment::Implementation::
              ConstructFileName(fileId);
          fileName = bdbName;

          u_int32_t flags = existing ? DB_CREATE : 0;
          int mode = 0;

          //Open the memory pool file
          DbEnv* dbenv = SmiEnvironment::instance.impl->bdbEnv;
          rc = dbenv->memp_fcreate(&dbMpf, 0);
          SmiEnvironment::SetBDBError(rc);

          rc = dbMpf->open(fileName.c_str(), flags,
              mode, poolPageSize);
          SmiEnvironment::SetBDBError(rc);

          if (!existing)
            InitializePoolFile();

          if (rc == 0)
            {
              //Register this process in the environment,
              //so that later processes can find how many
              //processes are using this file.
              RegisterInFile();

              //Register newly created file into the catalog
              //database directly, because other processes
              //can't access to the process-own catalog cache
              if (!existing)
                {
                  bool ok = true;
                  DbTxn* ptid = SmiEnvironment::
                      instance.impl->usrTxn;
                  DbTxn* tid = 0;
                  if (SmiEnvironment::useTransactions)
                    {
                      rc = SmiEnvironment::instance.impl->bdbEnv->
                          txn_begin(ptid, &tid, 0);
                      SmiEnvironment::SetBDBError(rc);
                    }

                  if (rc == 0)
                    {
                      SmiCatalogFilesEntry catalogEntry;
                      BdbInitCatalogEntry(catalogEntry.entry);
                      catalogEntry.entry.fileId = fileId;
                      name.copy(catalogEntry.entry.fileName, 2
                          * SMI_MAX_NAMELEN + 1);
                      catalogEntry.entry.isKeyed = false;
                      catalogEntry.entry.isFixed = false;

                      ok = SmiEnvironment::instance.impl->
                          InsertIntoCatalog(catalogEntry.entry, tid);
                        
                      if (tid != 0)
                        {
                          if (ok)
                            {
                              rc = tid->commit(0);
                              SmiEnvironment::SetBDBError(rc);
                            }
                          else
                            {
                              rc = tid->abort();
                              SmiEnvironment::SetBDBError(rc);
                            }
                        }
                    }
                  else
                    {
                      if (tid != 0)
                        {
                          rc = tid->abort();
                          SmiEnvironment::SetBDBError(rc);
                        }
                    }
                }

              opened = true;
              ctr++;
            }
          else
            {
              SmiEnvironment::SetBDBError(rc);
            }
        }
      else
        {
          rc = E_SMI_FILE_NOFILEID;
          SmiEnvironment::SetError(E_SMI_FILE_NOFILEID);
        }
    }
  else
    {
      rc = E_SMI_FILE_INVALIDNAME;
      SmiEnvironment::SetError(E_SMI_FILE_INVALIDNAME);
    }

  return (rc == 0);
}


/*

2.3 Open an existing update file.

*/
bool SmiUpdateFile::Open(const SmiFileId _fID,
    const SmiSize _pSize, bool _isIni)
{
  //assert(!opened);

  poolPageSize = _pSize;
  isInitialized = _isIni;

  static long& ctr = Counter::getRef("SmiFile::Open");
  int rc = 0;
  if (_fID != 0)
    {
      fileName = SmiEnvironment::Implementation::
          ConstructFileName(_fID);

      u_int32_t flags = 0;
      int mode = 0;

      DbEnv* dbenv = SmiEnvironment::instance.impl->bdbEnv;
      rc = dbenv->memp_fcreate(&dbMpf, 0);
      SmiEnvironment::SetBDBError(rc);

      if (!isInitialized)
        InitializePoolFile();

      rc = dbMpf->open(fileName.c_str(), flags, mode, poolPageSize);
      SmiEnvironment::SetBDBError(rc);
      existPageNum = GetFactPageNum();

      if (rc == 0)
        {
          RegisterInFile();
          opened = true;
          ctr++;
        }
      else
        {
          SmiEnvironment::SetBDBError(rc);
        }
    }
  else
    {
      rc = E_SMI_FILE_NOFILEID;
      SmiEnvironment::SetError(E_SMI_FILE_NOFILEID);
    }

  return (rc == 0);
}

/*

2.4 Close the handle to the update file.

If there is no more process sharing the file, flush the pages pinned
in the memory back to the disk file.

*/
bool SmiUpdateFile::Close()
{
  static long& ctr = Counter::getRef("SmiFile::Close");
  int rc = 0;

  if (opened)
    {
      ctr++;
      opened = false;

      UnRegisterInFile();
      int sharedBy = GetNumOfShareProcess();
      if (sharedBy == 0)
        {
          //If there is no processes are sharing the file,
          //synchronize the disk file with the pool.
          SyncFile();
        }
      rc = dbMpf->close(0); //release the handle
      dbMpf = 0;
      SmiEnvironment::SetBDBError(rc);
    }
  return (rc == 0);
}

int SmiUpdateFile::GetExistPageNum()
{
  if (isInitialized)
    return (GetFactPageNum() - sysPageNum);
  else
    return 0;
}

int SmiUpdateFile::GetSysPageNum()
{
  return sysPageNum;
}

bool SmiUpdateFile::GetIniStatus()
{
  return isInitialized;
}

/*

Initiate the disk memory pool file.
If exist ~sysPage~, insert the first page contains system information.
For some unknown reasons, if use DB\_CREATE flag to open the file,
then the first page, which page number is 0, can't be accessed.
Since then, before accessing the page-oriented memory pool file,
the file should be created first, and write the first page to the file.

*/
bool SmiUpdateFile::InitializePoolFile()
{
  //assert(dbMpf != 0);
  int rc = 0;

  if (!isInitialized)
    {
      //Create memory pool disk file
      const char *dbEnvPath = new char[updatePathLength];
      SmiEnvironment::instance.impl->bdbEnv->get_home(&dbEnvPath);
      ostringstream absoluteFilePath;
      absoluteFilePath << dbEnvPath << PATH_SLASH << fileName;
      fstream fp(absoluteFilePath.str().c_str(), ios::out | ios::binary);

      if (fp.fail())
        {
          cerr << "Initialize Smi update file " << fileName
              << " failed." << endl;
          return false;
        }

      if (sysPageNum > 0)
        {
          char *buf = new char[poolPageSize];
          memset(buf, 0, poolPageSize);
          SmiUpdateSysPage sysPage;
          memcpy(buf, &sysPage, sizeof(SmiUpdateSysPage));
          fp.write(buf, poolPageSize);

          isInitialized = true;
          delete[] buf;
        }

      fp.close();
    }
  return (rc == 0);
}

int SmiUpdateFile::GetNumOfShareProcess()
{
  //assert(dbMpf != 0);

  if (sysPageNum > 0)
    {
      db_pgno_t pageno = 0;
      SmiUpdateSysPage sysPage;
      void* sysPagePointer;
      int rc = 0;

      // Read the counter under the same lock used by Register/UnRegister so
      // the returned value is a consistent snapshot with respect to concurrent
      // updates from other processes.
      SysPageLock sysLock(SmiEnvironment::instance.impl->bdbEnv,
                          !SmiEnvironment::singleUserMode, fileName);

#if DB_VERSION_REQUIRED(4,6)
      DbTxn* tid = 0;
      rc = dbMpf->get(&pageno, tid, 0, &sysPagePointer);
#else
      rc = dbMpf->get(&pageno, 0, &sysPagePointer);
#endif
      if (rc != 0)
        {
          // The page could not be pinned; sysPagePointer is undefined, so we
          // must not dereference it. Report the error and give up.
          SmiEnvironment::SetBDBError(rc);
          return -1;
        }
      memcpy(&sysPage, sysPagePointer, sizeof(SmiUpdateSysPage));
#if DB_VERSION_REQUIRED(4,6)
      rc = dbMpf->put(sysPagePointer, DB_PRIORITY_DEFAULT, 0);
#else
      rc = dbMpf->put(sysPagePointer, 0);
#endif
      SmiEnvironment::SetBDBError(rc);

      return sysPage.shareByNum;
    }
  else
    return -1;
}

/*

When there is a new process want to open a memory pool file,
it will use register method to increase the counter in the
systematic page. Conversely, when closing the file, then use
unregister method to decrease the counter.

*/
bool SmiUpdateFile::RegisterInFile()
{
  //assert(dbMpf != 0);

  db_pgno_t pageno = 0;
  void* pagePointer;
  int rc = 0;

  if (sysPageNum > 0)
    {
      SmiUpdateSysPage sysPage;

      // Serialize the counter update below against other processes for the
      // whole get -> modify -> put critical section.
      SysPageLock sysLock(SmiEnvironment::instance.impl->bdbEnv,
                          !SmiEnvironment::singleUserMode, fileName);

      // The system page is modified below, so it must be fetched dirty: since
      // Berkeley DB 4.6 the dirty flag is requested at get time, not at put
      // time.
#if DB_VERSION_REQUIRED(4,6)
      DbTxn* tid = 0;
      rc = dbMpf->get(&pageno, tid, DB_MPOOL_DIRTY, &pagePointer);
#else
      rc = dbMpf->get(&pageno, 0, &pagePointer);
#endif

      if (rc != 0)
        {
          // The page could not be pinned; pagePointer is undefined. Report the
          // error and give up instead of dereferencing it.
          SmiEnvironment::SetBDBError(rc);
          return false;
        }

      memcpy(&sysPage, pagePointer, sizeof(SmiUpdateSysPage));
      sysPage.shareByNum++;
      memcpy(pagePointer, &sysPage, sizeof(SmiUpdateSysPage));
#if DB_VERSION_REQUIRED(4,6)
      rc = dbMpf->put(pagePointer, DB_PRIORITY_DEFAULT, 0);
#else
      rc = dbMpf->put(pagePointer, DB_MPOOL_DIRTY);
#endif
      SmiEnvironment::SetBDBError(rc);
    }

  //Get exist page numbers inside this file
  existPageNum = GetFactPageNum();

  return (rc == 0);
}

int SmiUpdateFile::GetFactPageNum()
{
  //assert(dbMpf != 0);

  u_int32_t factPageNum = 0;

  if (isInitialized)
    {
      db_pgno_t pageno = 0;
      void* pagePointer;
      int rc = 0;

      //Get exist page numbers inside this file
#if DB_VERSION_REQUIRED(4,6)
      DbTxn* tid = 0;
      rc = dbMpf->get(&pageno, tid, DB_MPOOL_LAST, &pagePointer);
#else
      rc = dbMpf->get(&pageno, DB_MPOOL_LAST, &pagePointer);
#endif
      if (rc != 0)
        {
          // The page could not be pinned; pageno/pagePointer are undefined.
          // Report the error and return 0 (no pages) rather than a bogus count.
          SmiEnvironment::SetBDBError(rc);
          return 0;
        }
      factPageNum = pageno + 1;
#if DB_VERSION_REQUIRED(4,6)
      rc = dbMpf->put(pagePointer, DB_PRIORITY_DEFAULT, 0);
#else
      rc = dbMpf->put(pagePointer, DB_MPOOL_CLEAN);
#endif
      SmiEnvironment::SetBDBError(rc);
    }
  return factPageNum;
}

bool SmiUpdateFile::UnRegisterInFile()
{
  //assert(dbMpf != 0);

  if (sysPageNum > 0)
    {
      db_pgno_t pageno = 0;
      SmiUpdateSysPage sysPage;
      void* sysPagePointer;
      int rc = 0;

      // Serialize the counter update below against other processes for the
      // whole get -> modify -> put critical section.
      SysPageLock sysLock(SmiEnvironment::instance.impl->bdbEnv,
                          !SmiEnvironment::singleUserMode, fileName);

      // The system page is modified below, so it must be fetched dirty (since
      // Berkeley DB 4.6 the dirty flag is requested at get time).
#if DB_VERSION_REQUIRED(4,6)
      DbTxn* tid = 0;
      rc = dbMpf->get(&pageno, tid, DB_MPOOL_DIRTY, &sysPagePointer);
#else
      rc = dbMpf->get(&pageno, 0, &sysPagePointer);
#endif
      if (rc != 0)
        {
          // The page could not be pinned; sysPagePointer is undefined. Report
          // the error and give up instead of dereferencing it.
          SmiEnvironment::SetBDBError(rc);
          return false;
        }
      memcpy(&sysPage, sysPagePointer, sizeof(SmiUpdateSysPage));
      sysPage.shareByNum--;
      memcpy(sysPagePointer, &sysPage, sizeof(SmiUpdateSysPage));
#if DB_VERSION_REQUIRED(4,6)
      rc = dbMpf->put(sysPagePointer, DB_PRIORITY_DEFAULT, 0);
#else
      rc = dbMpf->put(sysPagePointer, DB_MPOOL_DIRTY);
#endif
      SmiEnvironment::SetBDBError(rc);

      return (rc == 0);
    }
  else
    return true;
}

SmiSize SmiUpdateFile::GetPoolPageSize()
{
  return poolPageSize;
}

/*

2.5 AppendNewPage method in SmiUpdateFile

Use the ~get~ method and the DB\_MPOOL\_NEW flag in Berkeley DB
to add a new page to the end of the memory pool file.
Return the update page pointer and save it to the map structure ~gotPages~.

*/
bool SmiUpdateFile::AppendNewPage(SmiUpdatePage*& page)
{
  //assert(dbMpf != 0);
  int rc = 0;
  static long& ctr = Counter::getRef("SmiUpdateFile::AppPage");

    db_pgno_t realPageNo;
    void *pgPt;

#if DB_VERSION_REQUIRED(4,6)
      DbTxn* tid = 0;
      rc = dbMpf->get(&realPageNo,tid, DB_MPOOL_NEW, &pgPt);
#else
      rc = dbMpf->get(&realPageNo, DB_MPOOL_NEW, &pgPt);
#endif
    if (rc != 0)
      {
        // The new page could not be created; pgPt is undefined, so we must not
        // memset through it. Report the error and give up.
        SmiEnvironment::SetBDBError(rc);
        page = 0;
        return false;
      }
    ctr++;

    page = new SmiUpdatePage();
    page->pageNo = realPageNo - sysPageNum + 1;
    page->pageSize = poolPageSize;
    memset(pgPt, 0, poolPageSize);
    page->pagePt = pgPt;

    gotPages.insert(pair<db_pgno_t, SmiUpdatePage*>(page->pageNo, page));

    if (! isInitialized)
      isInitialized = true;

    existPageNum = GetFactPageNum();
    return (rc == 0);
}

/*

2.6 Get method in SmiUpdateFile

Get a page from the file. If the page is already got from the file before,
then return the page pointer from the ~gotPages~ directly.
Or else use Berkeley DB ~get~ operator to get it from the file,
and save the pointer into the ~gotPages~.

*/
bool SmiUpdateFile::GetPage(const db_pgno_t pageNo, SmiUpdatePage*& page)
{
    //assert(dbMpf != 0);
    map<db_pgno_t, SmiUpdatePage*>::iterator iter;
    int rc = 0;
    static long& ctr1 = Counter::getRef("SmiUpdateFile::GetPage::FromDisk");
    static long& ctr2 = Counter::getRef("SmiUpdateFile::GetPage::FromMem");


    iter = gotPages.find(pageNo);
    if ( iter == gotPages.end())
    {
      //Get the page from the disk file
      void *pagePointer;
      db_pgno_t realPageNo = pageNo + sysPageNum - 1;

      // Fetch the page dirty: the caller may modify it and later have it
      // written back via PutPage(isChanged=true). Since Berkeley DB 4.6 the
      // dirty flag is requested at get time, not at put time, and PutPage
      // can no longer mark the page dirty itself.
#if DB_VERSION_REQUIRED(4,6)
      DbTxn* tid = 0;
      rc = dbMpf->get(&realPageNo, tid, DB_MPOOL_DIRTY, &pagePointer);
#else
      rc = dbMpf->get(&realPageNo, 0, &pagePointer);
#endif
      if (rc != 0)
        {
          // The page could not be pinned; pagePointer is undefined. Report the
          // error and give up instead of handing out an invalid page.
          SmiEnvironment::SetBDBError(rc);
          page = 0;
          return false;
        }

      page = new SmiUpdatePage();
      page->pageNo = pageNo;
      page->pageSize = poolPageSize;
      page->pagePt = pagePointer;

      gotPages.insert(pair<db_pgno_t, SmiUpdatePage*>(pageNo, page));
      ctr1++;
      return true;
    }
    else
    {
      //Get the page from memory
      page = iter->second;
      ctr2++;
      return true;
    }
}

/*

2.7 Put method in SmiUpdateFile

Put a gotten but useless page back to the disk file. If ~isChanged~ is true,
then the modification on this page will be written to the source file before
being evicted from the pool.

*/
bool SmiUpdateFile::PutPage(const db_pgno_t pageNo,
    bool isChanged)
{
  //assert(dbMpf != 0);
  map<db_pgno_t, SmiUpdatePage*>::iterator iter;
  int rc = 0;
  static long& ctr = Counter::getRef("SmiUpdateFile::PutPage");


  iter = gotPages.find(pageNo);
  if (iter != gotPages.end())
    {
      SmiUpdatePage *page = iter->second;
      if (isChanged){
#if DB_VERSION_REQUIRED(4,6)
        rc = dbMpf->put(page->pagePt, DB_PRIORITY_DEFAULT,0 );
#else
        rc = dbMpf->put(page->pagePt, DB_MPOOL_DIRTY);
#endif
      } else {
#if DB_VERSION_REQUIRED(4,6)
        rc = dbMpf->put(page->pagePt, DB_PRIORITY_DEFAULT, 0);
#else
        rc = dbMpf->put(page->pagePt, DB_MPOOL_CLEAN);
#endif
      }
      SmiEnvironment::SetBDBError(rc);
      ctr++;

      delete iter->second;
      gotPages.erase(iter);
      return true;
    }
  else
    {
      cerr << "The page " << pageNo
          << " that is going to be put back doesn't exist"
          << endl;
      return false;
    }
}

/*

2.8 Synchronize method in SmiUpdateFile

Put all unput pages in the ~gotPages~ back to the source file.

*/
bool SmiUpdateFile::SyncFile()
{
  //assert(dbMpf != 0);
  int rc = 0;
  static long& ctr = Counter::getRef("SmiUpdateFile::SynPage");

  //put all got pages back, and write data to the disk file
  map<db_pgno_t, SmiUpdatePage*>::iterator iter;
  for (iter = gotPages.begin(); iter != gotPages.end(); /*iter++*/)
    {
      //Put all got pages back ...
      SmiUpdatePage *page = iter->second;
#if DB_VERSION_REQUIRED(4,6)
      rc = dbMpf->put(page->pagePt, DB_PRIORITY_DEFAULT, 0);
#else
      rc = dbMpf->put(page->pagePt, DB_MPOOL_DIRTY);
#endif
      SmiEnvironment::SetBDBError(rc);
      delete iter->second;
      gotPages.erase(iter++);
      ctr++;
    }

   //Synchronize the file
   rc = dbMpf->sync();
   SmiEnvironment::SetBDBError(rc);
   return (rc == 0);
}

/*

3 Implementation of class SmiUpdatePage

*/

SmiUpdatePage::SmiUpdatePage() :
  pageNo(0), pageSize(0)
{}

SmiUpdatePage::SmiUpdatePage(const SmiUpdatePage &rhs)
{
  pageNo = rhs.pageNo;
  pageSize = rhs.pageSize;
  pagePt = rhs.pagePt;
}

SmiUpdatePage::~SmiUpdatePage()
{
  pagePt = 0;
}


db_pgno_t SmiUpdatePage::GetPageNo()
{
  return pageNo;
}

void* SmiUpdatePage::GetPageAddr()
{
  return pagePt;
}

bool SmiUpdatePage::isAvailable()
{
  return (pagePt != 0);
}

SmiSize SmiUpdatePage::GetPageSize()
{
  return pageSize;
}

