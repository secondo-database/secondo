/*

1 Dynamic Library Management

January 2002 Ulrich Telle

*/

using namespace std;

#include "SecondoConfig.h"
#include "DynamicLibrary.h"

#ifndef SECONDO_WIN32
#include <dlfcn.h>
#endif

DynamicLibrary::DynamicLibrary()
  : libraryHandle( 0 ), libName( "" ), errorMessage( "" )
{
}

DynamicLibrary::~DynamicLibrary()
{
  assert( this );
  Unload();
}

bool
DynamicLibrary::Load( const string& libraryName )
{
  assert( &libraryName );
  bool ok = false;
  if ( libraryName.length() > 0  && !IsLoaded() )
  {
#ifdef SECONDO_WIN32
    libraryHandle = ::LoadLibrary( libraryName.c_str() );
#else
    string name = libraryName + ".so";
    libraryHandle = ::dlopen( name.c_str(), RTLD_LAZY | RTLD_GLOBAL );
#endif
    libName = libraryName;
    ok = (libraryHandle != 0);
    if ( !ok )
    {
      SetErrorMessage();
    }
  }
  else
  {
    errorMessage = (IsLoaded()) ?
                   "Another library ("+libName+") is already loaded." :
                   "No library name specified.";
  }
  return ok;
}

bool
DynamicLibrary::Unload()
{
  bool ok = false;
  if ( IsLoaded() )
  {
#ifdef SECONDO_WIN32
    ok = ::FreeLibrary( static_cast<HINSTANCE>(libraryHandle) );
    libraryHandle = 0;
#else
    ok = ::dlclose( libraryHandle ) == 0;
    libraryHandle = 0;
#endif
    if ( !ok )
    {
      SetErrorMessage();
    }
  }
  else
  {
    errorMessage = "No library is loaded.";
  }
  return (ok);
}

bool
DynamicLibrary::IsLoaded() const
{
  return (libraryHandle != 0);
}

string
DynamicLibrary::GetLibraryName() const
{
  if ( IsLoaded() )
  {
    return (libName);
  }
  else
  {
    return ("");
  }
}

void*
DynamicLibrary::GetFunctionAddress( const string& functionName )
{
  assert( &functionName );
  void* functionAddr = 0;
  if ( IsLoaded() )
  {
#ifdef SECONDO_WIN32
    functionAddr = ::GetProcAddress( static_cast<HINSTANCE>(libraryHandle),
                                     functionName.c_str());
#else
    functionAddr = ::dlsym( libraryHandle, functionName.c_str() );
#endif
    if ( functionAddr == 0 )
    {
      SetErrorMessage();
    }
  }
  else
  {
    errorMessage = "No library is loaded.";
  }
  return (functionAddr);
}

string 
DynamicLibrary::GetLastErrorMessage()
{
  string errmsg = errorMessage;
  errorMessage = "";
  return (errmsg);
}

void
DynamicLibrary::SetErrorMessage()
{
  char* msgBuffer;
#ifdef SECONDO_WIN32
  ::FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                   NULL, GetLastError(),
                   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                   (LPTSTR) &msgBuffer, 0, NULL );
  errorMessage = msgBuffer;
  LocalFree( msgBuffer );
#else
  msgBuffer = ::dlerror();
  errorMessage = (msgBuffer != 0) ? msgBuffer : "";
#endif
}

