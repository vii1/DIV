
// 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
// DIV Games Studio - Dinamic Link Library - Header File (c) 1998 HAMMER Tech.
// 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴

#ifndef __DIV_H_
#define __DIV_H_

#include <stdio.h>

#ifdef GLOBALS
  #define EXTERN
  void main(){};
#else
  #define EXTERN extern
#endif

// 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
// Common definitions for all libraries
// 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴

#define NULL            0

#define long_header     9
#define end_struct      long_header+14+10*10+10*7+8+11+9+10*4+1026+146+32*3

#define COMMON_PARAMS   void *(*DIV_import)(char *name),\
                        void (*DIV_export)(char *name,void *obj)
#define LIBRARY_PARAMS  void (*COM_export)(char *name,void *obj,int nparms)

#define AutoLoad()      DIV_export("Autoload",divmain)

// 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
// Definitions of basic language structures
// 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴

typedef struct __reserved{
        int     id;            // Process Identifier
        int     idscan;        // Collisions
        int     block;         // Process type
        int     blscan;        // Last getid()
        int     status;        // Actual status
        int     param_number;  // Number of parameters
        int     param;         // Pointer to parameters
        int     ip;            // Program counter
        int     sp;            // Stack pointer
        int     executed;      // Is process executed?
        int     painted;       // Is process painted?
        int     object;        // Number of object
        int     old_ctype;     // Old coordinate type
        int     frame;         // Percent of frame complete
        int     x0,y0,x1,y1;   // Bounding box
        int     f_count;       // Function call count
        int     caller_id;     // Caller identifier
}_reserved;

typedef struct _process{
        _reserved       reserved;       // See above.
        int             father;         // Father's id
        int             son;            // Son's id
        int             smallbro;       // Small brother's id
        int             bigbro;         // Big brother's id
        int             priority;       // Process priority
        int             ctype;          // Type of coordinates
        int             x;              // X of process
        int             y;              // Y of process
        int             z;              // Z of process
        int             graph;          // Code of process graphic
        int             flags;          // Flags of process
        int             size;           // Size of process
        int             angle;          // Angle of process
        int             region;         // Region of process
        int             file;           // File of process
        int             xgraph;         // Pointer to angular graphic table
        int             height;         // Height of process in mode 7
        int             cnumber;        // Scroll or mode 7 for that process
        int             resolution;     // Coordinates resolution of process
        int             radius;         // Radius for collision
        int             wall;           // World wall
        int             sector;         // Wordl sector
        int             nextsector;     // Next sector number
        int             step;           // Height step
}process;

typedef struct __mouse{
        int     x;      // Mouse x
        int     y;      // Mouse y
        int     z;      // Mouse z
        int     file;   // File code for mouse graphic
        int     graph;  // Mouse graphic code
        int     angle;  // Pointer angle
        int     size;   // Pointer size (percent)
        int     flags;  // Mouse flags
        int     region; // Mouse region (clipping)
        int     left;   // Left mouse button
        int     middle; // Middle mouse button
        int     right;  // Right mouse button
        int     cursor; // Cursor emulation
        int     speed;  // Pointer speed
}_mouse;

typedef struct __scroll{
        int     z;              // Scroll Z
        int     camera;         // Process identifier code
        int     ratio;          // Background speed ratio
        int     speed;          // Maximum scrolling speed
        int     region1;        // First region
        int     region2;        // Second region
        int     x0;             // First map coordinates
        int     y0;
        int     x1;             // Second map coordinates
        int     y1;
}_scroll;

typedef struct __m7{
        int     z;              // Mode 7 Z
        int     camera;         // Process identifier code
        int     height;         // Camera height
        int     distance;       // Distance between process and camera
        int     horizon;        // Horizon position
        int     focus;          // Camera focus
        int     color;          // Ground colour
}_m7;

typedef struct __joy{
        int     button1;        // Joystick buttons (0/1)
        int     button2;
        int     button3;
        int     button4;
        int     left;           // Joystick moves (0/1)
        int     right;
        int     up;
        int     down;
}_joy;

typedef struct __setup{
        int     card;
        int     port;
        int     irq;
        int     dma;
        int     dma2;
        int     master;
        int     sound_fx;
        int     cd_audio;
        int     mixer;
        int     rate;
        int     bits;
}_setup;


typedef struct __net{
        int     device;
        int     com;
        int     speed;
        int     number;
        int     init;
        int     mode;
        int     server;
        int     max_players;
        int     num_players;
}_net;

typedef struct __m8{
        int     z;
        int     camera;
        int     height;
        int     angle;
}_m8;

typedef struct __dirinfo{
        int     files;
        int     name[1024];
}_dirinfo;

typedef struct __fileinfo{
        int     fullpath_fix;
        char    fullpath[256];
        int     drive;
        int     dir_fix;
        char    dir[256];
        int     name_fix;
        char    name[12];
        int     ext_fix;
        char    ext[8];
        int     size;
        int     day;
        int     month;
        int     year;
        int     hour;
        int     min;
        int     sec;
        int     attrib;
}_fileinfo;

typedef struct __video_modes{
        int     wide;
        int     height;
        int     mode;
}_video_modes;

// 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
// Description of some basic DIV archives (for modifying them once loaded)
// 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴

typedef struct _FPGHEADER{
        char     header[8];
        char     palette[768];
        char     colorbars[16*36];
}FPGHEADER;

typedef struct _FPGBODY{
        int      code;
        int      lenght;
        char     description[32];
        char     filename[12];
        int      wide;
        int      height;
        int      number_of_points;
        //short  *points;   // 2*number_of_points
        //char   *graphic;  // wide*height
}FPGBODY;

typedef struct _FNTHEADER{
    char    header[7];
    char    eof;
} FNTHEADER;

typedef struct _FNTBODY{
    int    wide;
    int    height;
    int    screen_offset;
    int    file_offset;
}FNTBODY;

// 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
// DIV standard Entry-Points
// You must define these functions on the DLL if you want DIV32RUN to call them up.
// 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴

void set_video_mode(void);                  // Set new video mode (wide x height)
void process_palette(void);                 // Modify a new palette
void process_active_palette(void);          // Modify a new active palette (fadings)
void process_sound(char *sound,int lenght); // Modify a new sound effect
void process_fpg(char *fpg,int fpg_lenght); // Modify a new FPG
void process_map(char *map,int map_lenght); // Modify a new MAP
void process_fnt(char *fnt,int fnt_lenght); // Modify a new FNT
void background_to_buffer(void);            // Dump background to buffer
void buffer_to_video(void);                 // Dump buffer to video
void post_process_scroll(void);             // Apply effect on scroll window
void post_process_m7(void);                 // Apply effect on mode 7 window
void post_process_buffer(void);             // Apply effect on buffer (video)
void post_process(void);                    // Modify process variables
void put_sprite(unsigned char * si, int x,  // Put one sprite
     int y, int an, int al, int xg, int yg, int ang, int size, int flags);
void ss_init(void);                         // Screen Saver initialisation function
void ss_frame(void);                        // Screen Saver frame function
void ss_end(void);                          // Screen Saver ending function

// 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
// DIV exported (shared) variables
// 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴

#define mem             (_mem)              // Memory of all programs (mem[], int array)
#define stack           (_stack)            // mem[] index of stack
#define palette         (_palette)          // Pointer to game palette
#define active_palette  (_active_palette)   // Pointer to active palette (fadings)
#define key             (_key)              // Pointer to keycodes table (128 bytes)

#define sp              (*_sp)              // Stack pointer (as a mem[] index)
#define wide            (*_wide)            // Screen wide (pixels)
#define height          (*_height)          // Screen height (pixels)
#define ss_time         (*_ss_time)         // ScreenSaver calling (def: 3000 = 30 seconds)
#define ss_status       (*_ss_status)       // ScreenSaver status (0/1-Active)
#define ss_exit         (*_ss_exit)         // Set to 1 to end ScrenSaver (ss_frame())
#define process_size    (*_process_size)    // Size of every process (on mem[], number of ints)
#define id_offset       (*_id_offset)       // mem[] index of last executed process
#define id_init_offset  (*_id_init_offset)  // mem[] index of first process
#define id_start_offset (*_id_start_offset) // mem[] index of main process
#define id_end_offset   (*_id_end_offset)   // mem[] index of last process
#define set_palette     (*_set_palette)     // Set to 1 to activate a new palette,
                                            // set to 2 to activate it twice, etc.

#define buffer     ((char*)*((unsigned long *)_buffer))     // Pointer to video buffer
#define background ((char*)*((unsigned long *)_background)) // Pointer to video background
#define ghost      ((char*)*((unsigned long *)_ghost))      // Pointer to ghost-layering table

// 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
// DIV exported variables - Internal - You must use above definitions!
// 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴

EXTERN int  *_stack;
EXTERN int  *_mem;
EXTERN char *_palette;
EXTERN char *_active_palette;
EXTERN char *_key;

EXTERN int  *_sp;
EXTERN int  *_wide;
EXTERN int  *_height;
EXTERN int  *_ss_time;
EXTERN int  *_ss_status;
EXTERN int  *_ss_exit;
EXTERN int  *_process_size;
EXTERN int  *_id_offset;
EXTERN int  *_id_init_offset;
EXTERN int  *_id_start_offset;
EXTERN int  *_id_end_offset;
EXTERN int  *_set_palette;

EXTERN unsigned long _buffer;
EXTERN unsigned long _background;
EXTERN unsigned long _ghost;

// 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
// DIV exported functions
// 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴

EXTERN void *(*div_malloc)(size_t __size);             // Don't use malloc()!!!
EXTERN void  (*div_free  )( void *__ptr );             // Don't use free()!!!
EXTERN FILE *(*div_fopen )(char *,char *);             // Don't use fopen()!!!
EXTERN void  (*div_fclose)(FILE *);                    // Don't use fclose()!!!

EXTERN int   (*div_rand)(int rang_low,int rang_hi);    // Random between two numbers
EXTERN void  (*div_text_out)(char *texto,int x,int y); // Screen print function

// 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
// Macros to get parameters and return values in new functions
// 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴

#define getparm()     stack[sp--]           // Get next parameter
#define retval(_x_sp) (stack[++sp]=(_x_sp)) // Set return value (is a must)

// 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
// Games exported variables (see documentation - Language - Global - Variables)
// 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴

#define timer(x)          mem[end_struct+x]
#define text_z            mem[end_struct+10]
#define fading            mem[end_struct+11]
#define shift_status      mem[end_struct+12]
#define ascii             mem[end_struct+13]
#define scan_code         mem[end_struct+14]
#define joy_filter        mem[end_struct+15]
#define joy_status        mem[end_struct+16]
#define restore_type      mem[end_struct+17]
#define dump_type         mem[end_struct+18]
#define max_process_time  mem[end_struct+19]
#define fps               mem[end_struct+20]
#define _argc             mem[end_struct+21]
#define _argv(x)          mem[end_struct+22+x]
#define channel(x)        mem[end_struct+32+x]
#define vsync             mem[end_struct+64]
#define draw_z            mem[end_struct+65]
#define num_video_modes   mem[end_struct+66]
#define unit_size         mem[end_struct+67]

#define text_offset       0 // Start of text segment (mem[] index)

// 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
// Games exported structures (see documentation - Language - Global - Structures)
// 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴

#define MOUSE       ((_mouse       *)&mem[long_header])
#define SCROLL      ((_scroll      *)&mem[long_header+14])
#define M7          ((_m7          *)&mem[long_header+14+10*10])
#define JOY         ((_joy         *)&mem[long_header+14+10*10+10*7])
#define SETUP       ((_setup       *)&mem[long_header+14+10*10+10*7+8])
#define NET         ((_net         *)&mem[long_header+14+10*10+10*7+8+11])
#define M8          ((_m8          *)&mem[long_header+14+10*10+10*7+8+11+9])
#define DIRINFO     ((_dirinfo     *)&mem[long_header+14+10*10+10*7+8+11+9+10*4])
#define FILEINFO    ((_fileinfo    *)&mem[long_header+14+10*10+10*7+8+11+9+10*4+1026])
#define VIDEO_MODES ((_video_modes *)&mem[long_header+14+10*10+10*7+8+11+9+10*4+1026+146])

// 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
// Definition to get the offset of objects
// 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴

#define GLOBAL_IMPORT()                                       \
stack            =(int *)DIV_import("stack");                 \
mem              =(int *)DIV_import("mem");                   \
palette          =(char*)DIV_import("palette");               \
active_palette   =(char*)DIV_import("active_palette");        \
key              =(char*)DIV_import("key");                   \
_buffer          =(unsigned long)DIV_import("buffer");        \
_background      =(unsigned long)DIV_import("background");    \
_ghost           =(unsigned long)DIV_import("ghost");         \
_sp              =(int *)DIV_import("sp");                    \
_wide            =(int *)DIV_import("wide");                  \
_height          =(int *)DIV_import("height");                \
_ss_time         =(int *)DIV_import("ss_time");               \
_ss_status       =(int *)DIV_import("ss_status");             \
_ss_exit         =(int *)DIV_import("ss_exit");               \
_process_size    =(int *)DIV_import("process_size");          \
_id_offset       =(int *)DIV_import("id_offset");             \
_id_init_offset  =(int *)DIV_import("id_init_offset");        \
_id_start_offset =(int *)DIV_import("id_start_offset");       \
_id_end_offset   =(int *)DIV_import("id_end_offset");         \
_set_palette     =(int *)DIV_import("set_palette");           \
div_malloc  = ( void *(*)(size_t )       ) DIV_import("div_malloc"); \
div_free    = ( void  (*)(void * )       ) DIV_import("div_free"  ); \
div_fopen   = ( FILE *(*)(char *,char *) ) DIV_import("div_fopen" ); \
div_fclose  = ( void  (*)(FILE *)        ) DIV_import("div_fclose"); \
div_rand    = ( int   (*)(int ,int )     ) DIV_import("div_rand"  ); \
div_text_out= ( void  (*)(char *,int,int)) DIV_import("div_text_out");

#endif
