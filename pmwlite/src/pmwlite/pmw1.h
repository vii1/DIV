struct pmw1_header
{
 unsigned int id;                  // 0x31574D50 'PMW1'
 unsigned short version;           // Version Number Lo=Major, Hi=Minor
 unsigned short flags;             // Info Flags
 unsigned int cs_eip_object;       // Initial CS:EIP Object
 unsigned int cs_eip_offset;       // Initial CS:EIP Offset
 unsigned int ss_esp_object;       // Initial SS:ESP Object
 unsigned int ss_esp_offset;       // Initial SS:ESP Offset
 unsigned int objtbl_offset;       // Object Table Offset
 unsigned int objtbl_entries;      // Object Table Entries
 unsigned int rt_offset;           // Relocation Table Offset (RT)
 unsigned int data_offset;         // Data Pages Offset
}pmw1header;

struct pmw1_objtbl
{
 unsigned int virtualsize;         // Virtual Size Of Object
 unsigned int actualsize;          // Actual Size Of Object On Disk
 unsigned int flags;               // Object Flags
 unsigned int rt_offset;           // Beginning Offset From The Start Of RT
 unsigned int rt_blocks;           // Number Of Relocation Blocks
 unsigned int uactualsize;         // Uncompressed Actual Size
}pmw1objtbl[50];

struct pmw1_rtitem
{
 char type;                        // Relocation Type
 signed int sourceoff;             // Source Offset
 char targetobj;                   // Target Object
 signed int targetoff;             // Target Offset
}pmw1rtitem;

struct pmw1_rtblock
{
 unsigned short size;
 unsigned short usize;
}pmw1rtblock;
