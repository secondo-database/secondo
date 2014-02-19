/*
----
This file is part of SECONDO.

Copyright (C) 2007,
Faculty of Mathematics and Computer Science,
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


1 CollectFlobServer

A socket server accepting all collectFlob queries from the cluster on the current computer.

Requests are collected into a FIFO queue, being processed sequentially with
limited number of threads.

1.1 Includes

*/
#include <iostream>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <netinet/in.h>
#include <resolv.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <algorithm>

#include "Profiles.h"
#include <map>
#include <queue>
#include <set>

using namespace std;

int string2int(const string& str){
  istringstream s(str);
  int res;
  s >> res;
  return res;
}

string int2string(const int& number)
{
  ostringstream oss;
  oss << number;
  return oss.str();
}

template<typename T1, typename T2>
struct classCompPair
{
  bool operator() (const pair<T1,T2>& pair_1,
      const pair<T1,T2>& pair_2) const
  {
    return (pair_1.first < pair_2.first)
        || (!(pair_1.first > pair_2.first && pair_1.first < pair_2.first)
          && (pair_1.second < pair_2.second));
  }
};

void* ReceiveRequest(void*);
void* ProcessRequest(void*);
void* TraverseRequest(void*);
bool collectFlob(string request);


const int MAX_COPYTIMES = 20;
queue<pair<string, int*> > requests;
pthread_mutex_t CFBS_mutex;
const size_t PipeWidth = 1;
bool tokenPass[PipeWidth];
pthread_t threadId[PipeWidth];

class Prcv_Thread{
public:
  Prcv_Thread(pair<string, int*> r, int t):
    token(t){
    request = r.first;
    csock = r.second;
  }

  string request;
  int* csock;
  int token;

};

int main(){
  pthread_mutex_init(&CFBS_mutex, NULL);

  //Read the port for collectFlobServer
  string config = string(getenv("SECONDO_CONFIG"));
  int host_port = string2int(SmiProfile::GetParameter(
      "ParallelSecondo","CFBServPort", "", config));

  struct sockaddr_in my_addr;
  int hsock;
  int* p_int;

  socklen_t addr_size = 0;
  int* csock;
  sockaddr_in sadr;

  pthread_t recvTID = 0, trvsTID = 0;

  memset(threadId, 0, sizeof(pthread_t)*PipeWidth);

  hsock = socket(AF_INET, SOCK_STREAM, 0);
  if (hsock == -1){
    cerr << "Error!! Initializing socket fails: " << errno << endl;
    return -1;
  }

  p_int = (int*)malloc(sizeof(int));
  *p_int = 1;
  if ( (setsockopt(hsock, SOL_SOCKET, SO_REUSEADDR,
          (char*)p_int, sizeof(int)) == -1)
      || (setsockopt(hsock, SOL_SOCKET, SO_KEEPALIVE,
          (char*)p_int, sizeof(int)) == -1)){
    cerr << "Error!! Setting socket options fails: " << errno << endl;
    free(p_int);
    return -1;
  }
  free(p_int);

  my_addr.sin_family = AF_INET;
  my_addr.sin_port = htons(host_port);
  memset(&(my_addr.sin_zero), 0, 8);
  my_addr.sin_addr.s_addr = INADDR_ANY;

  if (bind(hsock, (sockaddr*)&my_addr, sizeof(my_addr)) == -1){
    cerr << "Error!! Binding socket on port " << host_port
        << " fails: " << errno << endl;
    cerr << "Make sure nothing else is listening on this port! " << endl;
    return -1;
  }
  if (listen(hsock, 10) == -1){
    cerr << "Error!! Listening fails: " << errno << endl;
    return -1;
  }

  addr_size = sizeof(sockaddr_in);

  pthread_create(&trvsTID, 0, &TraverseRequest, 0);
  pthread_detach(trvsTID);

  while(true){
    //Process all received requests first
//    if (!requests.empty()){
//      for (size_t t = 0; t < PipeWidth; t++){
//        if (tokenPass[t] || pthread_kill(threadId[t], 0))
//        {
//          pair<string, int*> r = requests.front();
//          tokenPass[t] = true;
//          Prcv_Thread* pt = new Prcv_Thread(r, t);
//          pthread_create(&threadId[t], 0, &ProcessRequest, (void*)pt);
//          requests.pop();
//        }
//
//        if (requests.empty()){
//          break;
//        }
//      }
//    }

    csock = (int*)malloc(sizeof(int));
    if((*csock = accept(hsock, (sockaddr*)&sadr, &addr_size))!= -1){
      //Put it into the queue first
      pthread_create(&recvTID, 0, &ReceiveRequest, (void*)csock);
      pthread_detach(recvTID);
    }
    else {
      cerr << "Warning!! Accepting fails: " << errno << endl;
    }
  }
}


void* ReceiveRequest(void* lp)
{
  //Get one request and put it into the queue

  int* csock = (int*)lp;

  size_t buffer_len = 1024;
  char buffer[buffer_len];
  int byteCount;

  memset(buffer, 0, buffer_len);
  if ((byteCount = recv(*csock, buffer, buffer_len, 0)) == -1){
    cerr << "Error!! Receiving data fails: " << errno << endl;
    free(csock);
    return NULL;
  }
  string args(buffer, find(buffer, buffer + buffer_len, '\0'));

  pthread_mutex_lock(&CFBS_mutex);
  cerr << "Received: " << args << endl;
  requests.push(make_pair(args, csock));
  pthread_mutex_unlock(&CFBS_mutex);

  return NULL;
}

void* TraverseRequest(void* ptr)
{
  while(true)
  {
    if (!requests.empty()){
      for (size_t t = 0; t < PipeWidth; t++){
        if (tokenPass[t] || pthread_kill(threadId[t], 0))
        {
          pair<string, int*> r = requests.front();
          tokenPass[t] = true;
          Prcv_Thread* pt = new Prcv_Thread(r, t);
          pthread_create(&threadId[t], 0, &ProcessRequest, (void*)pt);

          pthread_mutex_lock(&CFBS_mutex);
          requests.pop();
          pthread_mutex_unlock(&CFBS_mutex);
        }

        if (requests.empty()){
          break;
        }
      }
    }
  }
}

void* ProcessRequest(void* ptr)
{
  //Process one request
  Prcv_Thread* pt = (Prcv_Thread*)ptr;
  int token = pt->token;
  int* csock = pt->csock;

  bool ok = collectFlob(pt->request);

  int bytecount = 0;
  if ((bytecount = send(*csock, (char*)&ok, sizeof(bool), 0)) == -1){
    cerr << "Error!! Sending data fails: " << errno << endl;
    free(csock);
    return NULL;
  }
  free(csock);

  pthread_mutex_lock(&CFBS_mutex);
  tokenPass[token] = false;
  pthread_mutex_unlock(&CFBS_mutex);

  return NULL;
}

bool collectFlob(string request)
{
  istringstream ss(request);
  //The destPSFS contains the remote IP
  string sheetName, resultName, localPSFS, destPSFS;
  ss >> sheetName >> resultName >> localPSFS >> destPSFS;

  pthread_mutex_lock(&CFBS_mutex);
  cerr << "sheetName: " << sheetName << endl;
  cerr << "resultName: " << resultName << endl;
  cerr << "localPSFS: " << localPSFS << endl;
  cerr << "destPSFS: " << destPSFS << endl;
  pthread_mutex_unlock(&CFBS_mutex);

  //1. Fetch the sheet
  string command = "scp -q" + destPSFS + "/" + sheetName
      + " " + localPSFS;
  int atimes = MAX_COPYTIMES;
  while( atimes-- > 0 ){
    if (0 == system(command.c_str())){
      break;
    }
    else{
      sleep(1);
    }
  }
  if (atimes <= 0){
    pthread_mutex_lock(&CFBS_mutex);
    cerr << "Error!! Fetching the sheet file "
        << destPSFS << "/" << sheetName << " fails" << endl;
    pthread_mutex_unlock(&CFBS_mutex);
    return false;
  }

  //2. Prepare the result
  map< u_int32_t, ifstream* > flobFiles;
  map< u_int32_t, ifstream* >::iterator it;
  ifstream* flobFile = 0;

  string sheetFilePath = localPSFS + "/" + sheetName;
  string resultFilePath = localPSFS + "/tmp_" + resultName;
  ifstream sheetFile(sheetFilePath.c_str());
  ofstream resultFile(resultFilePath.c_str(), ios::binary);

  //Cached Flob markers, avoid extract the same Flob
  set<pair<u_int32_t, size_t>, classCompPair<u_int32_t, size_t> > lobMarkers;
  //Empty lobs are prepared for Flobs that have been created in another sheet
  set<pair<u_int32_t, size_t>, classCompPair<u_int32_t, size_t> > emptyLobs;

  string flobOrder;
  u_int32_t lastFileId = 0;
  while (getline(sheetFile, flobOrder))
  {
    stringstream ss(flobOrder);
    u_int32_t fileId;
    int sourceDS, mode;
    size_t offset, size;
    ss >> fileId >> sourceDS >> offset >> mode >> size;

    if ( mode != 3) {
      //This Flob may already have been fetched by another thread.
      size_t recId = sourceDS;
      pair<u_int32_t, size_t> mlob(fileId, recId);
      if (emptyLobs.find(mlob) != emptyLobs.end()){
        continue;
      } else {
        emptyLobs.insert(mlob);
      }

      char block[size];
      memset(block, 0, size);
      resultFile.write(block, size);
      continue;
    }

    //The flob is never mentioned in the current sheet
    pair<u_int32_t, size_t> mlob(fileId, offset);
    if (lobMarkers.find(mlob) != lobMarkers.end()){
      continue;
    } else {
      lobMarkers.insert(mlob);
    }

    if (lastFileId != fileId)
    {
      it = flobFiles.find(fileId);
      if ( it == flobFiles.end()){
        string flobFileName = localPSFS + "/flobFile_" + int2string(fileId);
        flobFile = new ifstream(flobFileName.c_str(), ios::binary);
        flobFiles[fileId] = flobFile;
        it = flobFiles.find(fileId);
      }
      lastFileId = fileId;
    }

    char block[size];
    flobFile = it->second;
    flobFile->seekg(offset, ios_base::beg);
    flobFile->read(block, size);
    resultFile.write(block, size);
  }

  for (it = flobFiles.begin(); it != flobFiles.end(); it++)
  {
    flobFile = it->second;
    flobFile->close();
    delete flobFile;
    flobFile = 0;
  }
  flobFiles.clear();
  resultFile.close();
  sheetFile.close();

  //3. Send the result
  command = "scp -q " + resultFilePath + " " + destPSFS + "/" + resultName;
  atimes = MAX_COPYTIMES;
  while ( atimes-- > 0){
    if ( 0 == system(command.c_str())){
      if (::unlink(resultFilePath.c_str()) != 0){
        cerr << "Warning! Deleting the result file "
            << resultFilePath << " fails. " << endl;
      }

      if (::unlink(sheetFilePath.c_str()) != 0){
        cerr << "Warning! Deleting the sheet file "
            << sheetFilePath << " fails. " << endl;
      }

      break;
    }
    else {
      sleep(1);
    }
  }

  return true;
}


