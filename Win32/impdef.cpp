//==================================
// This is a modification of:
//   PEDUMP - Matt Pietrek 1995
// done by Ismael Jurado 1997
//
// This program only outputs to stdout a DEF file from a DLL given in the
// command line.
// I don´t know how to contact Matt, so if this program is not legal, please
// let me know.
//    ismaelj@hotmail.com
//
// A small change to the output format made by Colin Peters
// (colin@bird.fu.is.aga-u.ac.jp) to work with the GNU-Win32 dlltool utility.
//==================================

#include <windows.h>
#include <stdio.h>

// Here starts Matt's code
//
// MakePtr is a macro that allows you to easily add to values (including
// pointers) together without dealing with C's pointer arithmetic.  It
// essentially treats the last two parameters as DWORDs.  The first
// parameter is used to typecast the result to the appropriate pointer type.
#define MakePtr( cast, ptr, addValue ) (cast)( (DWORD)(ptr) + (DWORD)(addValue))
PIMAGE_SECTION_HEADER GetEnclosingSectionHeader(DWORD rva,
                                                PIMAGE_NT_HEADERS pNTHeader)
{
    PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(pNTHeader);
    unsigned i;

    for ( i=0; i < pNTHeader->FileHeader.NumberOfSections; i++, section++ ) {
        // Is the RVA within this section?
        if ( (rva >= section->VirtualAddress) &&
             (rva < (section->VirtualAddress + section->Misc.VirtualSize)))
            return section;
    }
    return 0;
}

void DumpExportsSection(DWORD base, PIMAGE_NT_HEADERS pNTHeader) {
    PIMAGE_EXPORT_DIRECTORY exportDir;
    PIMAGE_SECTION_HEADER header;
    INT delta;
    DWORD i;
    PDWORD functions;
    PWORD ordinals;
    PSTR *name;
    DWORD exportsStartRVA, exportsEndRVA;

    exportsStartRVA = pNTHeader->OptionalHeader.DataDirectory
                            [IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
    exportsEndRVA = exportsStartRVA + pNTHeader->OptionalHeader.DataDirectory
                            [IMAGE_DIRECTORY_ENTRY_EXPORT].Size;

    // Get the IMAGE_SECTION_HEADER that contains the exports.  This is
    // usually the .edata section, but doesn't have to be.
    header = (PIMAGE_SECTION_HEADER)GetEnclosingSectionHeader( exportsStartRVA, pNTHeader );
    if ( !header ) return;
    delta = (INT)(header->VirtualAddress - header->PointerToRawData);
    exportDir = MakePtr(PIMAGE_EXPORT_DIRECTORY, base, exportsStartRVA - delta);
    printf("EXPORTS\n");
    functions = (PDWORD)((DWORD)exportDir->AddressOfFunctions - delta + base);
    ordinals = (PWORD)((DWORD)exportDir->AddressOfNameOrdinals - delta + base);
    name = (PSTR *)((DWORD)exportDir->AddressOfNames - delta + base);
    for ( i=0; i < exportDir->NumberOfFunctions; i++ ) {
        DWORD entryPointRVA = functions[i];
        DWORD j;
        int flag=0;

        if ( entryPointRVA == 0 )   // Skip over gaps in exported function
            continue;               // ordinals (the entrypoint is 0 for
                                    // these functions).
        // See if this function has an associated name exported for it.
        for ( j=0; j < exportDir->NumberOfNames; j++ )
            if ( ordinals[j] == i ) {
                printf("%s", name[j] - delta + base);
                flag=1;
            }
        // Is it a forwarder?  If so, the entry point RVA is inside the
        // .edata section, and is an RVA to the DllName.EntryPointName
        if ( (entryPointRVA >= exportsStartRVA)&&(entryPointRVA <= exportsEndRVA) )
        {
            printf("%s", entryPointRVA - delta + base );
            flag=1;
        }

        if (flag) printf("\n");
    }
}

void DumpExeFile( PIMAGE_DOS_HEADER dosHeader )
{
    PIMAGE_NT_HEADERS pNTHeader;
    DWORD base = (DWORD)dosHeader;

    pNTHeader = MakePtr( PIMAGE_NT_HEADERS, dosHeader,
                                dosHeader->e_lfanew );

    // First, verify that the e_lfanew field gave us a reasonable
    // pointer, then verify the PE signature.
    // IJ: gcc doesn't support C exeptions??
//    __try
//    {
        if ( pNTHeader->Signature != IMAGE_NT_SIGNATURE )
        {
            printf("Not a Portable Executable (PE) EXE\n");
            return;
        }
//    }
/*
    __except( TRUE )    // Should only get here if pNTHeader (above) is bogus
    {
        printf( "invalid .EXE\n");
        return;
    }
*/
    DumpExportsSection(base, pNTHeader);
    printf("\n");
}

void DumpFile(LPSTR filename)
{
    HANDLE hFile;
    HANDLE hFileMapping;
    LPVOID lpFileBase;
    PIMAGE_DOS_HEADER dosHeader;
    
    hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL,
                        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    if ( hFile == INVALID_HANDLE_VALUE )
    {
        printf("Couldn't open file with CreateFile()\n");
        return;
    }

    hFileMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if ( hFileMapping == 0 )
    {
        CloseHandle(hFile);
        printf("Couldn't open file mapping with CreateFileMapping()\n");
        return;
    }

    lpFileBase = MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, 0);
    if ( lpFileBase == 0 )
    {
        CloseHandle(hFileMapping);
        CloseHandle(hFile);
        printf("Couldn't map view of file with MapViewOfFile()\n");
        return;
    }

//  printf(";Exports of file %s\n\n", filename);
    
    dosHeader = (PIMAGE_DOS_HEADER)lpFileBase;
    if ( dosHeader->e_magic == IMAGE_DOS_SIGNATURE )
    {
        DumpExeFile( dosHeader );
    }
    else printf("Format Unknown!\n");
}

char HelpText[] =
"IMPDEF"
"Syntax: IMPDEF DLLname\n\n"
"Modification of:\n"
"\tPEDUMP - Win32/COFF EXE/OBJ/LIB file dumper - 1995 Matt Pietrek\n"
"by Ismael Jurado 1997. Only produces a DEF file from a DLL\n"
"Let me know if this modification is not legal: ismaelj@hotmail.com";

int main(int argc, char *argv[]) {
  if ( argc == 1 ) {
    printf( HelpText );
    return 1;
  }
  DumpFile(argv[1]);
  return 0;
}

