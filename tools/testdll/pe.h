typedef char  BYTE;
typedef short WORD;
typedef long  LONG;

typedef unsigned char  UBYTE;
typedef unsigned short UWORD;
typedef unsigned long  ULONG;

//
// Image Format
//

#define IMAGE_DOS_SIGNATURE                 0x5A4D      // MZ
#define IMAGE_OS2_SIGNATURE                 0x454E      // NE
#define IMAGE_OS2_SIGNATURE_LE              0x454C      // LE
#define IMAGE_NT_SIGNATURE                  0x00004550  // PE00

#pragma pack( push, 1 )

typedef struct _IMAGE_DOS_HEADER {      // DOS .EXE header
	UWORD   e_magic;                     // Magic number
	UWORD   e_cblp;                      // Bytes on last page of file
	UWORD   e_cp;                        // Pages in file
	UWORD   e_crlc;                      // Relocations
	UWORD   e_cparhdr;                   // Size of header in paragraphs
	UWORD   e_minalloc;                  // Minimum extra paragraphs needed
	UWORD   e_maxalloc;                  // Maximum extra paragraphs needed
	UWORD   e_ss;                        // Initial (relative) SS value
	UWORD   e_sp;                        // Initial SP value
	UWORD   e_csum;                      // Checksum
	UWORD   e_ip;                        // Initial IP value
	UWORD   e_cs;                        // Initial (relative) CS value
	UWORD   e_lfarlc;                    // File address of relocation table
	UWORD   e_ovno;                      // Overlay number
	UWORD   e_res[4];                    // Reserved words
	UWORD   e_oemid;                     // OEM identifier (for e_oeminfo)
	UWORD   e_oeminfo;                   // OEM information; e_oemid specific
	UWORD   e_res2[10];                  // Reserved words
	LONG    e_lfanew;                    // File address of new exe header
} IMAGE_DOS_HEADER, *PIMAGE_DOS_HEADER;

//
// File header format.
//

typedef struct _IMAGE_FILE_HEADER {
	UWORD    Machine;
	UWORD    NumberOfSections;
	ULONG    TimeDateStamp;
	ULONG    PointerToSymbolTable;
	ULONG    NumberOfSymbols;
	UWORD    SizeOfOptionalHeader;
	UWORD    Characteristics;
} IMAGE_FILE_HEADER, *PIMAGE_FILE_HEADER;

#define IMAGE_SIZEOF_FILE_HEADER             20

#define IMAGE_FILE_RELOCS_STRIPPED           0x0001  // Relocation info stripped from file.
#define IMAGE_FILE_EXECUTABLE_IMAGE          0x0002  // File is executable  (i.e. no unresolved externel references).
#define IMAGE_FILE_LINE_NUMS_STRIPPED        0x0004  // Line nunbers stripped from file.
#define IMAGE_FILE_LOCAL_SYMS_STRIPPED       0x0008  // Local symbols stripped from file.
#define IMAGE_FILE_MINIMAL_OBJECT            0x0010  // Reserved.
#define IMAGE_FILE_UPDATE_OBJECT             0x0020  // Reserved.
#define IMAGE_FILE_16BIT_MACHINE             0x0040  // 16 bit word machine.
#define IMAGE_FILE_BYTES_REVERSED_LO         0x0080  // Bytes of machine word are reversed.
#define IMAGE_FILE_32BIT_MACHINE             0x0100  // 32 bit word machine.
#define IMAGE_FILE_DEBUG_STRIPPED            0x0200  // Debugging info stripped from file in .DBG file
#define IMAGE_FILE_PATCH                     0x0400  // Reserved.
#define IMAGE_FILE_SYSTEM                    0x1000  // System File.
#define IMAGE_FILE_DLL                       0x2000  // File is a DLL.
#define IMAGE_FILE_BYTES_REVERSED_HI         0x8000  // Bytes of machine word are reversed.

#define IMAGE_FILE_MACHINE_UNKNOWN           0
#define IMAGE_FILE_MACHINE_I860              0x14d   // Intel 860.
#define IMAGE_FILE_MACHINE_I386              0x14c   // Intel 386.
#define IMAGE_FILE_MACHINE_R3000             0x162   // MIPS little-endian, 0540 big-endian
#define IMAGE_FILE_MACHINE_R4000             0x166   // MIPS little-endian
#define IMAGE_FILE_MACHINE_ALPHA             0x184   // Alpha_AXP

//
// Directory format.
//

typedef struct _IMAGE_DATA_DIRECTORY {
	ULONG   VirtualAddress;
	ULONG   Size;
} IMAGE_DATA_DIRECTORY, *PIMAGE_DATA_DIRECTORY;

#define IMAGE_NUMBEROF_DIRECTORY_ENTRIES    16

//
// Optional header format.
//

typedef struct _IMAGE_OPTIONAL_HEADER {
	//
	// Standard fields.
	//

	UWORD   Magic;
	UBYTE   MajorLinkerVersion;
	UBYTE   MinorLinkerVersion;
	ULONG   SizeOfCode;
	ULONG   SizeOfInitializedData;
	ULONG   SizeOfUninitializedData;
	ULONG   AddressOfEntryPoint;
	ULONG   BaseOfCode;
	ULONG   BaseOfData;

	//
	// NT additional fields.
	//

	ULONG   ImageBase;
	ULONG   SectionAlignment;
	ULONG   FileAlignment;
	UWORD   MajorOperatingSystemVersion;
	UWORD   MinorOperatingSystemVersion;
	UWORD   MajorImageVersion;
	UWORD   MinorImageVersion;
	UWORD   MajorSubsystemVersion;
	UWORD   MinorSubsystemVersion;
	ULONG   Reserved1;
	ULONG   SizeOfImage;
	ULONG   SizeOfHeaders;
	ULONG   CheckSum;
	UWORD   Subsystem;
	UWORD   DllCharacteristics;
	ULONG   SizeOfStackReserve;
	ULONG   SizeOfStackCommit;
	ULONG   SizeOfHeapReserve;
	ULONG   SizeOfHeapCommit;
	ULONG   LoaderFlags;
	ULONG   NumberOfRvaAndSizes;
	IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
} IMAGE_OPTIONAL_HEADER, *PIMAGE_OPTIONAL_HEADER;

#define IMAGE_SIZEOF_STD_OPTIONAL_HEADER      28
#define IMAGE_SIZEOF_NT_OPTIONAL_HEADER      224

typedef struct _IMAGE_NT_HEADERS {
	ULONG Signature;
	IMAGE_FILE_HEADER FileHeader;
	IMAGE_OPTIONAL_HEADER OptionalHeader;
} IMAGE_NT_HEADERS, *PIMAGE_NT_HEADERS;

// Subsystem Values

#define IMAGE_SUBSYSTEM_UNKNOWN              0   // Unknown subsystem.
#define IMAGE_SUBSYSTEM_NATIVE               1   // Image doesn't require a subsystem.
#define IMAGE_SUBSYSTEM_WINDOWS_GUI          2   // Image runs in the Windows GUI subsystem.
#define IMAGE_SUBSYSTEM_WINDOWS_CUI          3   // Image runs in the Windows character subsystem.
#define IMAGE_SUBSYSTEM_OS2_CUI              5   // image runs in the OS/2 character subsystem.
#define IMAGE_SUBSYSTEM_POSIX_CUI            7   // image run  in the Posix character subsystem.

// Dll Characteristics

#define IMAGE_LIBRARY_PROCESS_INIT           1   // Dll has a process initialization routine.
#define IMAGE_LIBRARY_PROCESS_TERM           2   // Dll has a thread termination routine.
#define IMAGE_LIBRARY_THREAD_INIT            4   // Dll has a thread initialization routine.
#define IMAGE_LIBRARY_THREAD_TERM            8   // Dll has a thread termination routine.

//
// Loader Flags
//

#define IMAGE_LOADER_FLAGS_BREAK_ON_LOAD    0x00000001
#define IMAGE_LOADER_FLAGS_DEBUG_ON_LOAD    0x00000002


// Directory Entries

#define IMAGE_DIRECTORY_ENTRY_EXPORT         0   // Export Directory
#define IMAGE_DIRECTORY_ENTRY_IMPORT         1   // Import Directory
#define IMAGE_DIRECTORY_ENTRY_RESOURCE       2   // Resource Directory
#define IMAGE_DIRECTORY_ENTRY_EXCEPTION      3   // Exception Directory
#define IMAGE_DIRECTORY_ENTRY_SECURITY       4   // Security Directory
#define IMAGE_DIRECTORY_ENTRY_BASERELOC      5   // Base Relocation Table
#define IMAGE_DIRECTORY_ENTRY_DEBUG          6   // Debug Directory
#define IMAGE_DIRECTORY_ENTRY_COPYRIGHT      7   // Description String
#define IMAGE_DIRECTORY_ENTRY_GLOBALPTR      8   // Machine Value (MIPS GP)
#define IMAGE_DIRECTORY_ENTRY_TLS            9   // TLS Directory
#define IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG   10   // Load Configuration Directory

//
// Section header format.
//

#define IMAGE_SIZEOF_SHORT_NAME              8

typedef struct _IMAGE_SECTION_HEADER {
	UBYTE   Name[IMAGE_SIZEOF_SHORT_NAME];
	ULONG   VirtualSize;
	ULONG   VirtualAddress;
	ULONG   SizeOfRawData;
	ULONG   PointerToRawData;
	ULONG   PointerToRelocations;
	union {
		ULONG PointerToLinenumbers;
		void* PhysicalAddress;
	};
	UWORD	 NumberOfRelocations;
	UWORD    NumberOfLinenumbers;
	ULONG   Characteristics;
} IMAGE_SECTION_HEADER, *PIMAGE_SECTION_HEADER;

#define IMAGE_SIZEOF_SECTION_HEADER          40

#define IMAGE_SCN_TYPE_REGULAR               0x00000000  //
#define IMAGE_SCN_TYPE_DUMMY                 0x00000001  // Reserved.
#define IMAGE_SCN_TYPE_NO_LOAD               0x00000002  // Reserved.
#define IMAGE_SCN_TYPE_GROUPED               0x00000004  // Used for 16-bit offset code.
#define IMAGE_SCN_TYPE_NO_PAD                0x00000008  // Reserved.
#define IMAGE_SCN_TYPE_COPY                  0x00000010  // Reserved.

#define IMAGE_SCN_CNT_CODE                   0x00000020  // Section contains code.
#define IMAGE_SCN_CNT_INITIALIZED_DATA       0x00000040  // Section contains initialized data.
#define IMAGE_SCN_CNT_UNINITIALIZED_DATA     0x00000080  // Section contains uninitialized data.

#define IMAGE_SCN_LNK_OTHER                  0x00000100  // Reserved.
#define IMAGE_SCN_LNK_INFO                   0x00000200  // Section contains comments or some other type of information.
#define IMAGE_SCN_LNK_OVERLAY                0x00000400  // Section contains an overlay.
#define IMAGE_SCN_LNK_REMOVE                 0x00000800  // Section contents will not become part of image.
#define IMAGE_SCN_LNK_COMDAT                 0x00001000  // Section contents comdat.

#define IMAGE_SCN_ALIGN_1UBYTES                          0x00100000  //
#define IMAGE_SCN_ALIGN_2UBYTES              0x00200000  //
#define IMAGE_SCN_ALIGN_4UBYTES              0x00300000  //
#define IMAGE_SCN_ALIGN_8UBYTES              0x00400000  //
#define IMAGE_SCN_ALIGN_16UBYTES             0x00500000  // Default alignment if no others are specified.
#define IMAGE_SCN_ALIGN_32UBYTES             0x00600000  //
#define IMAGE_SCN_ALIGN_64UBYTES             0x00700000  //

#define IMAGE_SCN_MEM_DISCARDABLE            0x02000000  // Section can be discarded.
#define IMAGE_SCN_MEM_NOT_CACHED             0x04000000  // Section is not cachable.
#define IMAGE_SCN_MEM_NOT_PAGED              0x08000000  // Section is not pageable.
#define IMAGE_SCN_MEM_SHARED                 0x10000000  // Section is shareable.
#define IMAGE_SCN_MEM_EXECUTE                0x20000000  // Section is executable.
#define IMAGE_SCN_MEM_READ                   0x40000000  // Section is readable.
#define IMAGE_SCN_MEM_WRITE                  0x80000000  // Section is writeable.

//
// Export Format
//

typedef struct _IMAGE_EXPORT_DIRECTORY {
	ULONG   Characteristics;
	ULONG   TimeDateStamp;
	UWORD   MajorVersion;
	UWORD   MinorVersion;
	ULONG   Name;
	ULONG   Base;
	ULONG   NumberOfFunctions;
	ULONG   NumberOfNames;
	ULONG   AddressOfFunctions;
	ULONG   AddressOfNames;
	ULONG   AddressOfNameOrdinals;
} IMAGE_EXPORT_DIRECTORY, *PIMAGE_EXPORT_DIRECTORY;

//
// Based relocation format.
//

typedef struct _IMAGE_BASE_RELOCATION {
	ULONG   VirtualAddress;
	ULONG   SizeOfBlock;
} IMAGE_BASE_RELOCATION, *PIMAGE_BASE_RELOCATION;

#define IMAGE_SIZEOF_BASE_RELOCATION         8

//
// Based relocation types.
//

#define IMAGE_REL_BASED_ABSOLUTE              0
#define IMAGE_REL_BASED_HIGH                  1
#define IMAGE_REL_BASED_LOW                   2
#define IMAGE_REL_BASED_HIGHLOW               3
#define IMAGE_REL_BASED_HIGHADJ               4
#define IMAGE_REL_BASED_MIPS_JMPADDR          5
#define IMAGE_REL_BASED_I860_BRADDR           6
#define IMAGE_REL_BASED_I860_SPLIT            7

typedef struct PE {
	IMAGE_NT_HEADERS	  nh;
	IMAGE_SECTION_HEADER* sh;
} _PE;
