#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <env.h>
#include "pmw1.h"
#include "encode.h"

#include "..\pmwver.h"

#define RT_BLOCKSIZE 16380

#define T_STUBEXE 1     /* 32bit Linear Executable w/Stub */
#define T_EXE 2         /* Standard DOS EXE */
#define T_PROEXE 3      /* DOS/4G Professional EXE */
#define T_LE 4          /* Unbound 32bit Linear Executable */
#define T_PMW1 5        /* PMWLITE compressed EXE */

#define E_READ 0        /* Error Reading File! */
#define E_SEEK 1        /* Error Seeking In File! */
#define E_INVALIDEXE 2  /* Invalid Executable Format! */
#define E_UNBOUND 3     /* This Executable Is Already Unbound! */
#define E_WRITE 4       /* Error Writing File! */
#define E_CREATETEMP 5  /* Error Creating Temp File! */
#define E_EXISTS 6      /* Destination File Already Exists! */
#define E_CREATESTUB 7  /* Error Creating Stub File! */
#define E_NOTLE 8       /* Only An LE Format Executable Can Be Bound! */
#define E_OPEN 9        /* Unable To Open File -  */
#define E_NOTEXE 10     /* Stub Must Be PMODEW.EXE! */
#define E_MZ 11         /* Unable To Locate MZ Signature In Stub! */
#define E_FIXUP 12      /* Error Translating Fixup Info! */
#define E_COMPRESS 13   /* Error During Compression! */
#define E_PMW1 14       /* This File Is Already Compressed! */

#define RBUFSIZE 16384

FILE *fp, *tempfp, *newstubfp, *tempfp2, *tempfp3;

unsigned int compress = 1;
int			 cr;

unsigned int ct = 2;
unsigned int ctypes[5] = { ENCODE_PROBESMIN, ENCODE_PROBESFEW, ENCODE_PROBESNORM, ENCODE_PROBESMANY, ENCODE_PROBESMAX };

unsigned int  spc;
unsigned char spinc[5] = "|/-\\";

unsigned int* p = 0x46c;

char tpath[_MAX_PATH];

char drive[_MAX_DRIVE];
char dir[_MAX_DIR];
char file[_MAX_FNAME];
char ext[_MAX_EXT];

unsigned int tp1, tp2;
unsigned int op;
unsigned int fsize;
unsigned int nfsize, onfsize, cnfsize;

unsigned int leobjtbl[50][6];

unsigned int rt_offcount;

unsigned char  tempfilename[_MAX_PATH];
unsigned char  tempfilename2[_MAX_PATH];
unsigned char  tempfilename3[_MAX_PATH];
unsigned char* exefilename;

unsigned int   gargc;
unsigned char* gargv[32];

unsigned char *pt, *ptw, *envptr;

unsigned int  pmwstart;
unsigned char leheader[256];

unsigned char buf[256];
unsigned char envbuf[_MAX_PATH + 64];
unsigned char rbuf[RBUFSIZE];

unsigned short i;

unsigned int findex, exetype, c, d, e, sindex, headersize, sl;

unsigned char* paths[] = { "\\BINB\\PMODEW.EXE", "\\BIN\\PMODEW.EXE", "\\BINW\\PMODEW.EXE", "\\PMODEW.EXE" };
unsigned int   pathc;

#define NUMPATHS 4

unsigned char* exetypes[] = { "Unknown", "32bit Linear Executable w/Stub", "Standard DOS EXE",
	"DOS/4G Professional EXE", "Unbound 32bit Linear Executable" };

unsigned char* errorstrings[] = { "Error Reading File!", "Error Seeking In File!", "Invalid Executable Format!",
	"This Executable Is Already Unbound!", "Error Writing File!", "Error Creating Temp File!",
	"Destination File Already Exists!", "Error Creating Stub File!", "Only An LE Format Executable Can Be Bound!",
	"Unable To Open File - ", "Stub Must Be PMODEW.EXE!", "Unable To Locate MZ Signature In Stub!",
	"Error Translating Fixup Info!", "Error During Compression!", "This File Is Already Compressed!" };

unsigned short swapb( unsigned short );
#pragma aux	   swapb = "xchg ah,al" modify[AX] parm[AX];

FILE* reopentemp3( void );

main( int argc, char* argv[] )
{
	gargc = argc;
	for( c = 0; c < argc; c++ ) gargv[c] = argv[c];

	printf( "PMODE/W Professional Compression Utility v%s\n", PMODEW_TEXT_VERSION );
	printf( "Copyright (C) 1995-2000, Charles Scheffold and Thomas Pytel.\n\n" );
	if( ( findex = findfilename( argc, argv ) ) == 0 ) usage();
	if( ( fp = fopen( argv[findex], "rb" ) ) == NULL ) exiterr( E_OPEN, strupr( argv[findex] ) );
	exefilename = argv[findex];
	exetype = detectexeformat( fp );

	if( c = checkswitch( 'C', argc, argv ) ) ct = atoi( &argv[c][2] );
	if( ct > 4 ) ct = 4;
	ct = ctypes[ct];

	envptr = getenv( "WATCOM" );

	opentempfiles( 1 );
	if( exetype == T_EXE || exetype == 0 ) exiterr( E_INVALIDEXE, NULL );
	if( exetype == T_PMW1 ) exiterr( E_PMW1, NULL );

	if( c = checkswitch( 'S', argc, argv ) ) {
		if( ( newstubfp = fopen( &argv[c][2], "rb" ) ) == NULL )
			exiterr( E_OPEN, strupr( &argv[c][2] ) );
		else
			cprintf( "Using %s\r\n\r\n", strupr( &argv[c][2] ) );
	} else {
		if( ( newstubfp = fopen( "pmodew.exe", "rb" ) ) == NULL ) {
			if( envptr != NULL ) {
				for( pathc = 0; pathc < NUMPATHS; pathc++ ) {
					getenvpath( paths[pathc] );
					if( ( newstubfp = fopen( ptw, "rb" ) ) != NULL ) {
						cprintf( "Using %s\r\n\r\n", strupr( ptw ) );
						break;
					}
				}
				if( newstubfp == NULL ) exiterr( E_OPEN, "PMODEW.EXE" );
			} else
				exiterr( E_OPEN, "PMODEW.EXE" );
		} else
			cprintf( "Using PMODEW.EXE In Current Directory\r\n\r\n" );
	}
	c = detectexeformat( newstubfp );
	if( c != T_EXE ) exiterr( E_NOTEXE, NULL );

	cprintf( "- Unbinding LE                 [ ]" );

	unbind( exetype, fp, tempfp );
	fseek( tempfp, 0, 0 );
	exetype = detectexeformat( tempfp );
	fseek( tempfp, 0, 0 );
	compressle( exetype, tempfp, tempfp2, newstubfp, tempfp3, c );
	info();
	fclose( fp );
	fclose( newstubfp );
	fclose( tempfp );
	fclose( tempfp2 );
	remove( exefilename );
	if( rename( tempfilename2, exefilename ) ) exiterr( E_EXISTS, NULL );
	quit( 0 );
}

void opentempfiles( int force )
{
	if( c = checkswitch( 'D', gargc, gargv ) ) {
		exefilename = &gargv[c][2];
	}
	srand( p[0] );
	d = rand();
	sprintf( buf, "%3d", d );
	buf[3] = NULL;
	if( c = checkswitch( 'D', gargc, gargv ) || force == 1 ) {
		_splitpath( exefilename, drive, dir, file, ext );
		_makepath( tempfilename, drive, dir, "$$PMWB$$", buf );
	} else {
		strcpy( tempfilename, "$$PMWB$$." );
		strcat( tempfilename, buf );
	}
	c = rand();
	if( c == d ) c++;
	sprintf( buf, "%3d", c );
	buf[3] = NULL;
	if( c = checkswitch( 'D', gargc, gargv ) || force == 1 ) {
		_splitpath( exefilename, drive, dir, file, ext );
		_makepath( tempfilename2, drive, dir, "$$PMWB$$", buf );
	} else {
		strcpy( tempfilename2, "$$PMWB$$." );
		strcat( tempfilename2, buf );
	}
	e = rand();
	if( e == c || e == d ) e = c + d;
	sprintf( buf, "%3d", e );
	buf[3] = NULL;
	if( c = checkswitch( 'D', gargc, gargv ) || force == 1 ) {
		_splitpath( exefilename, drive, dir, file, ext );
		_makepath( tempfilename3, drive, dir, "$$PMWB$$", buf );
	} else {
		strcpy( tempfilename3, "$$PMWB$$." );
		strcat( tempfilename3, buf );
	}
	if( ( tempfp = fopen( tempfilename, "wb+" ) ) == NULL ) exiterr( E_CREATETEMP, NULL );
	if( ( tempfp2 = fopen( tempfilename2, "wb+" ) ) == NULL ) exiterr( E_CREATETEMP, NULL );
	if( ( tempfp3 = fopen( tempfilename3, "wb+" ) ) == NULL ) exiterr( E_CREATETEMP, NULL );
}

void compressle( int t, FILE* efp, FILE* tfp, FILE* stubfp, FILE* rfp, int st )
{
	int x, y, z, p;

	if( t != T_LE ) exiterr( E_NOTLE, NULL );

	cprintf( "\x08\x08û]\r\n" );
	cprintf( "- Preparing/Copying Stub       [ ]" );

	if( ( fseek( efp, 0, 0 ) ) != 0 ) exiterr( E_SEEK, NULL );
	if( ( fseek( tfp, 0, 0 ) ) != 0 ) exiterr( E_SEEK, NULL );
	if( ( fseek( stubfp, 0, 0 ) ) != 0 ) exiterr( E_SEEK, NULL );

	if( ( fread( buf, 0x1c, 1, stubfp ) ) < 1 ) exiterr( E_READ, NULL );
	p = ( *(unsigned short*)( buf + 4 ) * 512 );
	if( *(unsigned short*)( buf + 2 ) != 0 ) p -= ( 512 - *(unsigned short*)( buf + 2 ) );
	if( filelength( fileno( stubfp ) ) < p ) p = filelength( fileno( stubfp ) );
	z = *(unsigned short*)( buf + 8 ) * 16;
	*(unsigned short*)( buf + 8 ) = ( ( *(unsigned short*)( buf + 6 ) * 4 ) / 16 ) + 4;
	if( ( *(unsigned short*)( buf + 6 ) * 4 ) % 16 ) *(unsigned short*)( buf + 8 ) += 1;
	y = *(unsigned short*)( buf + 0x18 );
	*(unsigned short*)( buf + 0x18 ) = 0x40;
	for( x = 100; x < 256; x++ ) buf[x] = 0;
	if( ( fwrite( buf, 0x1c, 1, tfp ) ) < 1 ) exiterr( E_WRITE, NULL );
	if( ( fwrite( &buf[100], 0x24, 1, tfp ) ) < 1 ) exiterr( E_WRITE, NULL );
	if( *(unsigned short*)( buf + 6 ) != 0 ) {
		if( ( fseek( stubfp, y, 0 ) ) != 0 ) exiterr( E_SEEK, NULL );
		y = *(unsigned short*)( buf + 6 ) * 4;
		copy( tfp, stubfp, y );
		if( y % 16 ) copy( tfp, stubfp, ( 16 - ( y % 16 ) ) );
	}
	if( ( fseek( stubfp, z, 0 ) ) != 0 ) exiterr( E_SEEK, NULL );
	copy( tfp, stubfp, (int)( p - ftell( stubfp ) ) );
	x = ftell( tfp );
	if( x % 16 )
		if( ( fwrite( &buf[100], ( 16 - ( x % 16 ) ), 1, tfp ) ) < 1 ) exiterr( E_WRITE, NULL );
	p = ftell( tfp );
	x = p / 512;
	y = p % 512;
	if( y != 0 ) x++;
	*(unsigned short*)( buf + 4 ) = (unsigned short)x;
	*(unsigned short*)( buf + 2 ) = (unsigned short)y;
	x = ftell( tfp );
	if( ( fseek( tfp, 0, 0 ) ) != 0 ) exiterr( E_SEEK, NULL );
	if( ( fwrite( buf, 0x1c, 1, tfp ) ) < 1 ) exiterr( E_WRITE, NULL );
	if( ( fseek( tfp, 0x3c, 0 ) ) != 0 ) exiterr( E_SEEK, NULL );
	if( ( fwrite( &x, 4, 1, tfp ) ) < 1 ) exiterr( E_WRITE, NULL );

	// fclose(tfp);
	// tfp=fopen("pmw","wb+");
	// x=0;

	cprintf( "\x08\x08û]\r\n" );
	cprintf( "- Reading LE Information       [ ]" );

	if( ( fseek( tfp, x, 0 ) ) != 0 ) exiterr( E_SEEK, NULL );
	pmwstart = ftell( tfp );
	pmw1header.id = 0x31574D50;
	pmw1header.version = ( PMODEW_MINOR_VERSION << 8 ) + PMODEW_MAJOR_VERSION;
	pmw1header.flags = compress;
	if( ( fread( leheader, 0xC4, 1, efp ) ) < 1 ) exiterr( E_READ, NULL );
	memcpy( &pmw1header.cs_eip_object, &leheader[0x18], 16 );
	pmw1header.objtbl_offset = sizeof( pmw1header );
	pmw1header.objtbl_entries = *(unsigned int*)( leheader + 0x44 );
	if( ( fwrite( &pmw1header, sizeof( pmw1header ), 1, tfp ) ) < 1 ) exiterr( E_WRITE, NULL );

	x = *(unsigned int*)( leheader + 0x40 );
	if( ( fseek( efp, x, 0 ) ) != 0 ) exiterr( E_SEEK, NULL );
	for( x = 0; x < *(unsigned int*)( leheader + 0x44 ); x++ ) {
		spinner();
		if( ( fread( &leobjtbl[x][0], 24, 1, efp ) ) < 1 ) exiterr( E_READ, NULL );
		pmw1objtbl[x].virtualsize = leobjtbl[x][0];
		pmw1objtbl[x].actualsize = leobjtbl[x][4] << 12;
		pmw1objtbl[x].flags = leobjtbl[x][2];
		if( ( x == ( *(unsigned int*)( leheader + 0x44 ) - 1 ) ) && leobjtbl[x][4] != 0 ) {
			if( leobjtbl[x][4] == 1 )
				pmw1objtbl[x].actualsize = *(unsigned int*)( leheader + 0x2C );
			else
				pmw1objtbl[x].actualsize -= 4096 - *(unsigned int*)( leheader + 0x2C );
		}
		if( ( fwrite( &pmw1objtbl[x], sizeof( pmw1objtbl[x] ), 1, tfp ) ) < 1 ) exiterr( E_WRITE, NULL );
	}

	cprintf( "\x08\x08û]\r\n" );
	cprintf( "- Compressing Relocation Data  [ ]" );

	pmw1header.rt_offset = ftell( tfp ) - pmwstart;
	rt_offcount = 0;
	for( x = 0; x < *(unsigned int*)( leheader + 0x44 ); x++ ) {
		z = 0;
		nfsize = 0;
		rfp = reopentemp3();
		if( leobjtbl[x][4] != 0 ) {
			p = *(unsigned int*)( leheader + 0x48 ) + (unsigned int)leobjtbl[x][3] * 4 - 4;
			if( ( fseek( efp, p, 0 ) ) != 0 ) exiterr( E_SEEK, NULL );
			for( y = 0; y < leobjtbl[x][4]; y++ ) {
				if( ( fread( buf, 4, 1, efp ) ) < 1 ) exiterr( E_READ, NULL );
				op = ftell( efp );
				i = swapb( *(unsigned short*)( buf + 1 ) );
				if( i != 0 ) {
					p = *(unsigned int*)( leheader + 0x68 ) + ( i * 4 - 4 );
					if( ( fseek( efp, p, 0 ) ) != 0 ) exiterr( E_SEEK, NULL );
					if( ( fread( buf, 8, 1, efp ) ) < 1 ) exiterr( E_READ, NULL );
					if( *(unsigned int*)( buf ) != *(unsigned int*)( buf + 4 ) ) {
						p = *(unsigned int*)( leheader + 0x6C ) + *(unsigned int*)( buf );
						if( ( fseek( efp, p, 0 ) ) != 0 ) exiterr( E_SEEK, NULL );
						fsize = *(unsigned int*)( buf + 4 ) - *(unsigned int*)( buf );
						if( fsize > RBUFSIZE ) exiterr( E_FIXUP, NULL );
						if( ( fread( rbuf, fsize, 1, efp ) ) < 1 ) exiterr( E_READ, NULL );
						pt = &rbuf[0];
						while( ( pt - &rbuf[0] ) < fsize ) {
							spinner();
							z++;
							if( *(unsigned short*)(pt)&0x720 ) exiterr( E_FIXUP, NULL );
							i = *(unsigned short*)( pt );
							pmw1rtitem.type = (unsigned short)( i & 15 );
							pt += 2;
							pmw1rtitem.sourceoff = *(signed short*)( pt ) + (signed int)( y * 4096 );
							pt += 2;
							if( i & 0x4000 ) {
								if( *(unsigned short*)( pt ) > 255 )
									exiterr( E_FIXUP, NULL );
								else {
									pmw1rtitem.targetobj = *(unsigned char*)( pt );
									pt += 2;
								}
							} else {
								pmw1rtitem.targetobj = *(unsigned char*)( pt );
								pt += 1;
							}
							if( pmw1rtitem.type != 2 ) {
								if( i & 0x1000 ) {
									pmw1rtitem.targetoff = *(unsigned int*)( pt );
									pt += 4;
								} else {
									pmw1rtitem.targetoff = *(unsigned short*)( pt );
									pt += 2;
								}
								if( ( fwrite( &pmw1rtitem, sizeof( pmw1rtitem ), 1, rfp ) ) < 1 )
									exiterr( E_WRITE, NULL );
								nfsize += sizeof( pmw1rtitem );
							} else {
								pmw1rtitem.targetoff = 0;
								if( ( fwrite( &pmw1rtitem, sizeof( pmw1rtitem ), 1, rfp ) ) < 1 )
									exiterr( E_WRITE, NULL );
								nfsize += sizeof( pmw1rtitem );
							}
						}
					}
				}
				if( ( fseek( efp, op, 0 ) ) != 0 ) exiterr( E_SEEK, NULL );
			}
		}

		if( ( fseek( rfp, 0, 0 ) ) != 0 ) exiterr( E_SEEK, NULL );
		onfsize = nfsize;
		pmw1objtbl[x].rt_blocks = 0;
		if( nfsize != 0 ) {
			if( compress == 0 ) {
				for( y = 0; y < nfsize / RT_BLOCKSIZE; y++ ) {
					pmw1rtblock.size = RT_BLOCKSIZE;
					pmw1rtblock.usize = RT_BLOCKSIZE;
					if( ( fwrite( &pmw1rtblock, sizeof( pmw1rtblock ), 1, tfp ) ) < 1 ) exiterr( E_WRITE, NULL );
					copy( tfp, rfp, RT_BLOCKSIZE );
					pmw1objtbl[x].rt_blocks++;
				}
				if( ( nfsize % RT_BLOCKSIZE ) != 0 ) {
					pmw1rtblock.size = nfsize % RT_BLOCKSIZE;
					pmw1rtblock.usize = nfsize % RT_BLOCKSIZE;
					if( ( fwrite( &pmw1rtblock, sizeof( pmw1rtblock ), 1, tfp ) ) < 1 ) exiterr( E_WRITE, NULL );
					copy( tfp, rfp, ( nfsize % RT_BLOCKSIZE ) );
					pmw1objtbl[x].rt_blocks++;
				}
			}

			else {
				cnfsize = 0;
				// if(nfsize<3) exiterr(E_COMPRESS,NULL);
				for( y = 0; y < nfsize / RT_BLOCKSIZE; y++ ) {
					spinner();
					pmw1rtblock.usize = RT_BLOCKSIZE;
					tp1 = ftell( tfp );
					if( ( fwrite( &pmw1rtblock, sizeof( pmw1rtblock ), 1, tfp ) ) < 1 ) exiterr( E_WRITE, NULL );
					cr = Compress( tfp, rfp, RT_BLOCKSIZE, ct, 0 );
					if( cr < 0 ) exiterr( E_COMPRESS, NULL );
					cnfsize += cr;
					pmw1rtblock.size = cr;
					tp2 = ftell( tfp );
					if( ( fseek( tfp, tp1, 0 ) ) != 0 ) exiterr( E_SEEK, NULL );
					if( ( fwrite( &pmw1rtblock, sizeof( pmw1rtblock ), 1, tfp ) ) < 1 ) exiterr( E_WRITE, NULL );
					if( ( fseek( tfp, tp2, 0 ) ) != 0 ) exiterr( E_SEEK, NULL );
					pmw1objtbl[x].rt_blocks++;
				}
				if( ( nfsize % RT_BLOCKSIZE ) != 0 ) {
					spinner();
					pmw1rtblock.usize = nfsize % RT_BLOCKSIZE;
					tp1 = ftell( tfp );
					if( ( fwrite( &pmw1rtblock, sizeof( pmw1rtblock ), 1, tfp ) ) < 1 ) exiterr( E_WRITE, NULL );
					cr = Compress( tfp, rfp, nfsize % RT_BLOCKSIZE, ct, 0 );
					if( cr < 0 ) exiterr( E_COMPRESS, NULL );
					cnfsize += cr;
					pmw1rtblock.size = cr;
					tp2 = ftell( tfp );
					if( ( fseek( tfp, tp1, 0 ) ) != 0 ) exiterr( E_SEEK, NULL );
					if( ( fwrite( &pmw1rtblock, sizeof( pmw1rtblock ), 1, tfp ) ) < 1 ) exiterr( E_WRITE, NULL );
					if( ( fseek( tfp, tp2, 0 ) ) != 0 ) exiterr( E_SEEK, NULL );
					pmw1objtbl[x].rt_blocks++;
				}
				nfsize = cnfsize;
			}
		}
		pmw1objtbl[x].rt_offset = rt_offcount;
		rt_offcount += nfsize + ( pmw1objtbl[x].rt_blocks * sizeof( pmw1rtblock ) );
	}

	cprintf( "\x08\x08û]\r\n" );

	pmw1header.data_offset = ftell( tfp ) - pmwstart;

	if( ( fseek( tfp, ( pmwstart + pmw1header.data_offset ), 0 ) ) != 0 ) exiterr( E_SEEK, NULL );
	if( ( fseek( efp, *(unsigned int*)( leheader + 0x80 ), 0 ) ) != 0 ) exiterr( E_SEEK, NULL );
	y = ftell( efp );
	for( x = 0; x < pmw1header.objtbl_entries; x++ ) {
		sl = pmw1objtbl[x].actualsize;
		if( pmw1objtbl[x].virtualsize < pmw1objtbl[x].actualsize ) pmw1objtbl[x].actualsize = pmw1objtbl[x].virtualsize;
		op = ftell( efp );
		if( ( fseek( efp, 0, 2 ) ) != 0 ) exiterr( E_SEEK, NULL );
		z = ( ftell( efp ) - op );
		if( z < pmw1objtbl[x].actualsize ) {
			pmw1objtbl[x].actualsize = z;
			sl = z;
		}
		if( ( fseek( efp, op, 0 ) ) != 0 ) exiterr( E_SEEK, NULL );
		pmw1objtbl[x].uactualsize = pmw1objtbl[x].actualsize;
		if( pmw1objtbl[x].actualsize != 0 ) {
			if( compress == 0 )
				copy( tfp, efp, pmw1objtbl[x].actualsize );
			else {
				// if(pmw1objtbl[x].actualsize<3) exiterr(E_COMPRESS,NULL);
				cprintf( "\r" );
				cprintf( "- Compressing Object #%-4d         \x08\x08\x08\x08", x + 1 );
				cr = Compress( tfp, efp, pmw1objtbl[x].actualsize, ct, 1 );
				if( cr < 0 )
					exiterr( E_COMPRESS, NULL );
				else
					pmw1objtbl[x].actualsize = cr;
			}
		}
		y += sl;
		if( ( fseek( efp, y, 0 ) ) != 0 ) exiterr( E_SEEK, NULL );
	}
	cprintf( "\x08\x08\x08\x08    Done!\r\n" );

	if( ( fseek( tfp, pmwstart, 0 ) ) != 0 ) exiterr( E_SEEK, NULL );
	if( ( fwrite( &pmw1header, sizeof( pmw1header ), 1, tfp ) ) < 1 ) exiterr( E_WRITE, NULL );
	if( ( fseek( tfp, ( pmwstart + pmw1header.objtbl_offset ), 0 ) ) != 0 ) exiterr( E_SEEK, NULL );
	p = (unsigned int)( sizeof( pmw1objtbl[0] ) ) * (unsigned int)pmw1header.objtbl_entries;
	if( ( fwrite( &pmw1objtbl, p, 1, tfp ) ) < 1 ) exiterr( E_WRITE, NULL );
}

FILE* reopentemp3( void )
{
	FILE* tt;
	fclose( tempfp3 );
	remove( tempfilename3 );
	tt = fopen( tempfilename3, "wb+" );
	return tt;
}

void unbind( int t, FILE* efp, FILE* tfp )
{
	int	  x, y, z, p;
	FILE* stubfp;

	z = 0;
	if( t == T_LE ) exiterr( E_UNBOUND, NULL );
	if( t == T_STUBEXE ) {
		if( ( fseek( efp, 0x3c, 0 ) ) != 0 ) exiterr( E_SEEK, NULL );
		if( ( fread( &p, 4, 1, efp ) ) < 1 ) exiterr( E_READ, NULL );
		if( ( fseek( efp, p, 0 ) ) != 0 ) exiterr( E_SEEK, NULL );
	}
	if( t == T_PROEXE ) {
		if( ( fseek( efp, 0, 0 ) ) != 0 ) exiterr( E_SEEK, NULL );
		if( ( fread( buf, 0x1c, 1, efp ) ) < 1 ) exiterr( E_READ, NULL );
		p = ( *(unsigned short*)( buf + 4 ) * 512 );
		if( *(unsigned short*)( buf + 2 ) != 0 ) p -= ( 512 - *(unsigned short*)( buf + 2 ) );
		if( ( fseek( efp, p, 0 ) ) != 0 ) exiterr( E_SEEK, NULL );

		x = ftell( efp );
		if( ( fread( &buf[100], 0x30, 1, efp ) ) < 1 ) exiterr( E_READ, NULL );
		while( *(unsigned short*)( buf + 100 ) == 0x5742 ) {
			y = *(unsigned int*)( buf + 100 + 0x20 );
			if( ( fseek( efp, x + y, 0 ) ) != 0 ) exiterr( E_SEEK, NULL );
			x = ftell( efp );
			if( ( fread( &buf[100], 0x30, 1, efp ) ) < 1 ) exiterr( E_READ, NULL );
		}
		if( ( fseek( efp, x, 0 ) ) != 0 ) exiterr( E_SEEK, NULL );
		z = ftell( efp );
		if( ( fseek( efp, 0x3c, 1 ) ) != 0 ) exiterr( E_SEEK, NULL );
		if( ( fread( &p, 4, 1, efp ) ) < 1 ) exiterr( E_READ, NULL );
		if( ( fseek( efp, x + p, 0 ) ) != 0 ) exiterr( E_SEEK, NULL );
	}
	x = ftell( efp );
	y = ftell( tfp );
	copy( tfp, efp, (unsigned int)filelength( fileno( efp ) ) - x );
	if( ( fseek( tfp, y, 0 ) ) != 0 ) exiterr( E_SEEK, NULL );
	if( ( fread( buf, 0x84, 1, tfp ) ) < 1 ) exiterr( E_READ, NULL );
	if( ( fseek( tfp, y, 0 ) ) != 0 ) exiterr( E_SEEK, NULL );
	*(unsigned int*)( buf + 0x80 ) -= ( x - z );
	if( ( fwrite( buf, 0x84, 1, tfp ) ) < 1 ) exiterr( E_WRITE, NULL );
}

void copy( FILE* dfp, FILE* sfp, unsigned int numbytes )
{
	int x;
	for( x = 0; x < numbytes / RBUFSIZE; x++ ) {
		if( ( fread( rbuf, RBUFSIZE, 1, sfp ) ) < 1 ) exiterr( E_READ, NULL );
		if( ( fwrite( rbuf, RBUFSIZE, 1, dfp ) ) < 1 ) exiterr( E_WRITE, NULL );
		//    cprintf(".");
		spinner();
	}
	if( numbytes % RBUFSIZE != 0 ) {
		if( ( fread( rbuf, ( numbytes % RBUFSIZE ), 1, sfp ) ) < 1 ) exiterr( E_READ, NULL );
		if( ( fwrite( rbuf, ( numbytes % RBUFSIZE ), 1, dfp ) ) < 1 ) exiterr( E_WRITE, NULL );
		//    cprintf(".");
		spinner();
	}
}

int detectexeformat( FILE* efp )
{
	int t, p, x, y;
	t = 0;
	if( ( fread( buf, 0x1c, 1, efp ) ) < 1 ) exiterr( E_READ, NULL );
	if( *(unsigned short*)( buf ) == 0x5a4d || *(unsigned short*)( buf ) == 0x4d5a ) {
		t = T_EXE;
		if( *(unsigned short*)( buf + 0x18 ) >= 0x40 ) t = T_STUBEXE;
	}

	if( *(unsigned short*)( buf ) == 0x454c ) t = T_LE;

	if( t == T_STUBEXE ) {
		if( ( fseek( efp, 0x3c, 0 ) ) != 0 ) exiterr( E_SEEK, NULL );
		if( ( fread( &p, 4, 1, efp ) ) < 1 ) exiterr( E_READ, NULL );
		if( ( p + 4 ) < filelength( fileno( efp ) ) ) {
			if( ( fseek( efp, p, 0 ) ) != 0 ) exiterr( E_SEEK, NULL );
			if( ( fread( &buf[100], 4, 1, efp ) ) < 1 ) exiterr( E_READ, NULL );
			if( *(unsigned short*)( buf + 100 ) != 0x454c )
				t = T_EXE;
			else
				headersize = ftell( efp ) - 4;
			if( *(unsigned int*)( buf + 100 ) == 0x31574D50 ) t = T_PMW1;
		} else
			t = T_EXE;
	}

	if( t == T_EXE ) {
		p = ( *(unsigned short*)( buf + 4 ) * 512 );
		if( *(unsigned short*)( buf + 2 ) != 0 ) p -= ( 512 - *(unsigned short*)( buf + 2 ) );
		if( ( fseek( efp, p, 0 ) ) != 0 ) exiterr( E_SEEK, NULL );
		x = ftell( efp );
		fread( &buf[100], 0x30, 1, efp );
		while( *(unsigned short*)( buf + 100 ) == 0x5742 ) {
			y = *(unsigned int*)( buf + 100 + 0x20 );
			if( ( fseek( efp, x + y, 0 ) ) != 0 ) exiterr( E_SEEK, NULL );
			x = ftell( efp );
			if( ( fread( &buf[100], 0x30, 1, efp ) ) < 1 ) exiterr( E_READ, NULL );
		}
		y = ftell( efp ) - 0x30;
		if( *(unsigned short*)( buf + 100 ) != 0x5a4d && *(unsigned short*)( buf + 100 ) != 0x4d5a )
			t = T_EXE;
		else {
			t = T_PROEXE;
			fread( &buf[100], 0x10, 1, efp );
			headersize = y + *(unsigned int*)( buf + 100 + 0xc );
		}
	}
	return t;
}

int checkswitch( unsigned char sw, unsigned int numargs, char* args[] )
{
	unsigned int x;
	for( x = 1; x < numargs; x++ )
		if( args[x][0] == '-' || args[x][0] == '/' )
			if( toupper( args[x][1] ) == sw ) return x;
	return 0;
}

int findfilename( unsigned int numargs, char* args[] )
{
	unsigned int x;
	for( x = 1; x < numargs; x++ )
		if( args[x][0] != '-' && args[x][0] != '/' ) return x;
	return 0;
}

int ifindfilename( unsigned int numargs, char* args[], int startindex )
{
	unsigned int x;
	for( x = startindex; x < numargs; x++ )
		if( args[x][0] != '-' && args[x][0] != '/' ) return x;
	return 0;
}

void getenvpath( unsigned char* cep )
{
	strcpy( envbuf, envptr );
	strcat( envbuf, cep );
	ptw = &envbuf;
}

void usage( void )
{
	printf( "Usage:  PMWLITE [options] <filename>\n" );
	printf( "\nOptions:\n" );
	printf( "--------\n" );
	printf( "/Cx            Compression (4=Max, 3=Extra, 2=Normal, 1=Faster, 0=Fastest)\n" );
	printf( "/D<filename>   Use <filename> for destination instead of default\n" );
	printf( "/S<filename>   Specify a stub to use instead of PMODEW.EXE\n" );
	quit( 1 );
}

void info( void )
{
	printf( "\n" );
	c = filelength( fileno( tempfp2 ) ) * 100 / filelength( fileno( fp ) );
	printf( "Original Executable Size:  %d\n", filelength( fileno( fp ) ) );
	printf( "     New Executable Size:  %d (%d%%)\n", filelength( fileno( tempfp2 ) ), c );
	printf( "\n" );
}

spinner()
{
	cprintf( "\x08\x08%c]", spinc[spc] );
	if( spc < 3 )
		spc++;
	else
		spc = 0;
}

void quit( unsigned int errnum )
{
	fclose( tempfp );
	fclose( tempfp2 );
	fclose( tempfp3 );
	remove( tempfilename );
	remove( tempfilename2 );
	remove( tempfilename3 );
	exit( errnum );
}

void exiterr( unsigned int errnum, unsigned char* errordata )
{
	printf( "\n[PMWLITE]: %s%s\n", errorstrings[errnum], errordata );
	quit( 1 );
}
