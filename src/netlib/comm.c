// #define DEBUG                              /* comment me to eliminate main() */

/*     Last modification by Rick Wagner, May 21, 1993.
 *
 *     - Changed some declarations for compilation under MS QC2.5
 *     - Changed some function names for compilation under MS QC2.5
 *     - Changed delay()
 *     -
 *     Needs: A more specific interuppt mask, disable() may cause
 *            problems (e.g. missed kbhits).
 */

/*     Last modification by Al Sharpe, April 28, 1990.
 *
 *     - corrected comm_close() to drop RTS and DTR, not just DTR.
 *     - correct flsh_dtr( ) to drop only DTR and leave RTS active.
 *     - flsh_dtr( ) duration extented to 1/2 second.
 *     - correct all bugs in function main( ) so test program will work.
 *     - removed global CARRIER as it was redundant.
 *     - conio.h added for cprintf( ) function.
 */

/*    Modification, to support comm ports 3 & 4
 *       Al Sharpe, June 7, 1989
 *    Minor changes added to handle DTR on port close.
 *
 *    New function added :
 *      flsh_dtr( )   -  drops DTR for 200 milliseconds
 *
 *    Old function modified :
 *      comm_getc( unsigned seconds )   -  (1) gets a char from input buffer
 *                                             with timeout
 *                                         (2) if a key is pressed a Timeout
 *                                             is returned
 */

/*    Modification, to support other compiler(s):
 *       Pete Gontier, July 20th, 1988
 *    This code is no longer portable to the UNIX systems mentioned.
 *    The function names were in accordance with the TT teletype conventions,
 *    and I couldn't stand for it. So sue me.
 */

/*    From CompuServe's BPROGB Borland Int'l forum
 *
 *    #:  11879 S4/Turbo C
 *        02-Jun-88  18:54:50
 *    Sb: #11817-VT100
 *    Fm: Jerry Joplin 70441,2627
 *    To: Pete Gontier 72261,1754 (X)
 *
 *    Pete, here are the communication ISR's I've been using.   Hope these
 *    don't overflow the message data base for BPROGB.  :-)
 *    I think all is included to set up COM1/COM2 for receiver interrupts.
 *
 *    Also, these are straight ports from routines in MSKERMIT 2.30 and CKERMIT
 *    for UNIX, which are public domain but copyrighted (c) by Columbia
 *    University.
 */


#include <stdlib.h>
#include <dos.h>
#include <bios.h>
#include <conio.h>
#include <time.h>
#include "comm.h"                                /* comm prototypes */

#define TCICOMM                                  /* avoid extern declaration in... */

#define MDMDAT1 0x03F8                           /* Address of modem port 1 data */
#define MDMSTS1 0x03FD                           /* Address of modem port 1 status  */
#define MDMCOM1 0x03FB                           /* Address of modem port 1 command */
#define MDMDAT2 0x02F8                           /* Address of modem port 2 data */
#define MDMSTS2 0x02FD                           /* Address of modem port 2 status */
#define MDMCOM2 0x02FB                           /* Address of modem port 2 command */
#define MDMDAT3 0x03E8                           /* Address of modem port 3 data */
#define MDMSTS3 0x03ED                           /* Address of modem port 3 status  */
#define MDMCOM3 0x03EB                           /* Address of modem port 3 command */
#define MDMDAT4 0x02E8                           /* Address of modem port 4 data */
#define MDMSTS4 0x02ED                           /* Address of modem port 4 status */
#define MDMCOM4 0x02EB                           /* Address of modem port 4 command */
#define MDMINTV 0x000C                           /* Com 1 & 3 interrupt vector */
#define MDINTV2 0x000B                           /* Com 2 & 4 interrupt vector */
#define MDMINTO 0x0EF                            /* Mask to enable IRQ3 for port 1 & 3 */
#define MDINTO2 0x0F7                            /* Mask to enable IRQ4 for port 2 & 4 */
#define MDMINTC 0x010                            /* Mask to Disable IRQ4 for port 1 & 3 */
#define MDINTC2 0x008                            /* Mask to Disable IRQ3 for port 2 & 4 */
#define INTCONT 0x0021                           /* 8259 interrupt controller ICW2-3 */
#define INTCON1 0x0020                           /* Address of 8259 ICW1 */
#define CBS     2048                             /* Communications port buffer size */
#define XOFF    0x13                             /* XON/XOFF */
#define XON     0x11
#define CARRIER_ON  0x80                         /* Return value for carrier detect */


/** globals not externally accessible ***************************************/

static int    MODEM_STAT;                        /* 8250 modem status register */
static int    dat8250;                           /* 8250 data register */
static int    stat8250;                          /* 8250 line-status register */
static int    com8250;                           /* 8250 line-control register */
static char   en8259;                            /* 8259 IRQ enable mask */
static char   dis8259;                           /* 8259 IRQ disable mask */
static unsigned int intv;                        /* interrupt number to usurp */

static char   buffer[CBS];                       /* Communications circular buffer */
static char   *inptr;                            /* input address of circular buffer */
static char   *outptr;                           /* output address of circular buffer */
static int    c_in_buf = 0;                      /* count of characters received */
static int    xoffpt;                            /* amount of buffer that forces XOFF */
static int    xonpt;                             /* amount of buffer that unXOFFs */

static void (_interrupt _far *oldvec) ( );       /* vector of previous comm interrupt */
int xonxoff = 0;                                 /* auto xon/xoff support flag */
int xofsnt  = 0;                                 /* XOFF transmitted flag */
int xofrcv  = 0;                                 /* XOFF received flag */

void _interrupt _far serint ( void )
{                                                /* ISR to receive character */
   *inptr++ = (char) inp ( dat8250 );            /* Store character in buffer */
   c_in_buf++;                                   /* and increment count */
   if ( xonxoff )
   {                                             /* if xon/xoff auto-support is on */
      if ( c_in_buf > xoffpt && ! xofsnt )
      {                                          /* then if buf nearly full */
	 comm_putc ( XOFF );                     /* send an XOFF */
	 xofsnt = 1;                             /* and say so */
      }
   }
   _disable ( );                                 /* ints off for ptr change */
   if ( inptr == &buffer[CBS] )
   {                                             /* Set buffer input pointer */
      inptr = buffer;
   }
   _enable ( );
   outp ( 0x20, 0x20 );                          /* Generic EOI to 8259 */
}


void comm_close ( void )
{                                                // restore previous settings of 8259
    outp ( com8250 + 1, 0x08 );                  // Drop OUT2
    outp ( com8250 + 1, 0x00 );                  // Drop DTR and RTS
    outp ( INTCONT, dis8259 | inp ( INTCONT ) ); // Disable com interrupt at 8259

    _dos_setvect ( intv, oldvec );               // Reset original interrupt vector
}

void dobaud ( unsigned baudrate )                // parses baud integer to mask,
{                                                // re-inits port accordingly
   unsigned char portval;
   unsigned char blo, bhi;
   switch  ( baudrate )                          /* Baud rate LSB's and MSB's */
   {
       case 50:     bhi = 0x9;  blo = 0x00;  break;
       case 75:     bhi = 0x6;  blo = 0x00;  break;
       case 110:    bhi = 0x4;  blo = 0x17;  break;
       case 150:    bhi = 0x3;  blo = 0x00;  break;
       case 300:    bhi = 0x1;  blo = 0x80;  break;
       case 600:    bhi = 0x0;  blo = 0xC0;  break;
       case 1200:   bhi = 0x0;  blo = 0x60;  break;
       case 1800:   bhi = 0x0;  blo = 0x40;  break;
       case 2000:   bhi = 0x0;  blo = 0x3A;  break;
       case 2400:   bhi = 0x0;  blo = 0x30;  break;
       case 4800:   bhi = 0x0;  blo = 0x18;  break;
       case 9600:   bhi = 0x0;  blo = 0x0C;  break;
       case 19200:  bhi = 0x0;  blo = 0x06;  break;
       default:
	   return;
   }
   portval = (unsigned char) inp ( com8250 );    /* read Line-Control Reg val */
   outp ( com8250, portval | 0x80 );             /* set high bit for baud init */
   outp ( dat8250, blo );                        /* Send LSB for baud rate */
   outp ( dat8250 + 1, bhi );                    /* Send MSB for baud rate */
   outp ( com8250, portval );                    /* Reset initial value at LCR */
}


/* installs comm interrupts */

int comm_open ( int portid, unsigned speed )
{
   int be = _bios_equiplist ( );                 /* to get # installed serial ports */
   be <<= 4;                                     /* shift-wrap high bits off */
   be >>= 13;                                    /* shift down to low bits */
   if ( be >= portid || portid >= 3)
   {
      if ( portid == 1 )
      {
	  dat8250  = MDMDAT1;
	  stat8250 = MDMSTS1;
	  com8250  = MDMCOM1;
	  dis8259  = MDMINTC;
	  en8259   = MDMINTO;
	  intv = MDMINTV;
      }
      if ( portid == 2 )
      {
	  dat8250  = MDMDAT2;
	  stat8250 = MDMSTS2;
	  com8250  = MDMCOM2;
	  dis8259  = MDINTC2;
	  en8259   = MDINTO2;
	  intv = MDINTV2;
      }
      if ( portid == 3 )                         /* Ports 3 & 4 cannot be checked */
      {                                          /* with biosquip( ) */
	  dat8250  = MDMDAT3;
	  stat8250 = MDMSTS3;
	  com8250  = MDMCOM3;
	  dis8259  = MDMINTC;
	  en8259   = MDMINTO;
	  intv = MDMINTV;
      }
      if ( portid == 4 )
      {
	  dat8250  = MDMDAT4;
	  stat8250 = MDMSTS4;
	  com8250  = MDMCOM4;
	  dis8259  = MDINTC2;
	  en8259   = MDINTO2;
	  intv = MDINTV2;
      }
      if (portid <= 0 && portid >= 5)
      {
	 return ( 0 );
      }

      MODEM_STAT = dat8250 + 6;                  /* Define Modem Status Register */
      dobaud ( speed );                          /* set baud */
      inptr = outptr = buffer;                   /* set circular buffer values */
      c_in_buf = 0;
      oldvec = _dos_getvect ( intv );            /* Save old int vector */
      _dos_setvect  ( intv, serint );            /* Set up SERINT as com ISR */
      outp ( com8250,     0x3 );                 /* 8 bits no parity */
      outp ( com8250 + 1, 0xb );                 /* Assert OUT2, RTS, and DTR */
      inp  ( dat8250 );
      outp ( dat8250 + 1, 0x1 );                 /* Receiver-Data-Ready int */
      outp ( INTCONT, en8259 & inp ( INTCONT ) );
						 /* Enable 8259 interrupts */
      xoffpt = CBS - 128;                        /* chars in buff to send XOFF */
      xonpt  = CBS - xoffpt;                     /* chars in buff to send XON */
   }                                             /* END of if(be >= portid || portid >= 3) */
   else
   {
      be = 0;
   }
   return ( be );
}


int comm_avail ( )                               /* returns # characters available in buffer */
{
    return ( c_in_buf );
}


void comm_putc ( unsigned char c )               /* sends char out port */
{
    while ( ( inp ( stat8250 ) & 0x20 ) == 0 );  /* Wait til transmitter is ready */
    outp ( dat8250, c );                         /* then send it */
}


int comm_getc ( unsigned seconds )               /* gets char from buffer */
{
int c;
//    long get_tm, end_tm;
register char * ptr;
  if ( c_in_buf < xonpt && xofsnt )            // Check if we need to send
  {
    xofsnt = 0;                               // an XON to the host after
    comm_putc ( XON );                        // we had to send an XOFF
  }

/*
    _bios_timeofday(_TIME_GETCLOCK,&get_tm);     // If character not ready
    end_tm = get_tm + (18 * seconds);            // then wait til one is
                                                 // or return TIMEOUT
    do
    {
	if(kbhit())
    {                                        // If key pressed, Timeout
      getch();                               // is returned
      comm_putc('\20');                      // send 'space' to modem to
      return(USR_BRK);                       // cancel off-hook
	}
    _bios_timeofday(_TIME_GETCLOCK,&get_tm); // check Timeout counter
    }
    while(get_tm <= end_tm && c_in_buf == 0) ;

    if(get_tm >= end_tm) return(TIMEOUT);        // port timed out
*/

    ptr = outptr;
    c = *ptr++;                                  /* Get next character in circular buff */
    if ( ptr == &buffer[CBS] )                   /* Check for end of circular buffer */
    {
      ptr = buffer;                            /* start from bottom of buff */
    }
    _disable();                                  /* no interrupts during pointer manips */
    outptr = ptr;                                /* set character output pointer */
    c_in_buf--;                                  /* and decrement the character count */
    _enable();                                   /* then allow interrupts to continue */
    return(c);                                   /* Return the character */
}

void comm_flush ( )                             /* flushes all chars out of buffer */
{
    if ( xofsnt )                               /* Check if XON needs to be sent */
    {
       xofsnt = 0;
       comm_putc ( XON );
    }
    _disable ( );                                /* no interrupts during pointer manips */
    inptr = outptr = buffer;                     /* reset buffer pointers */
    c_in_buf = 0;                                /* and indicate no chars received */
    _enable ( );
}

void flsh_dtr ( )                                /* Drop DTR for 1/2 second */
{
    outp( com8250 + 1, 0xa);                     /* Un-set DTR (set RTS, OUT2) */
    comm_delay(500);                                  /* Wait for 1/2 second */
    outp( com8250 + 1, 0xb);                     /* Set DTR, RTS, OUT2 */

}


int carrier( )                                   /* TEST FOR CARRIER DETECT */
{

  comm_delay(200);                                    /* Short pause */
  if((inp( MODEM_STAT ) & CARRIER_ON ) == CARRIER_ON)
  {
    return(1);                                   /* Check MSR for DCD */
  }
  return(0);
}

void comm_delay( int msecs)
{
 clock_t imsecs,goal;
 imsecs=(clock_t)msecs*(CLOCKS_PER_SEC/1000);
 goal=imsecs+clock();
 while (goal >clock());
}


/****************************************************************************/

#ifdef _DEBUG

#include <string.h>

main( int argc, char *argv[] )
{
   char result = 0;
   int port = 2, speed = 19200;                   /* Default speed & port */
   unsigned int ser_stat;
   char c=0;

   if( argc >= 2 ) port = *argv[1] - 48;
   if( argc == 3 ) speed = atoi( argv[2] );

   ser_stat=_bios_serialcom(_COM_INIT,0,
			    _COM_9600|_COM_NOPARITY|_COM_CHR8);

   cprintf ( "\r\n"
       "Serial port %d open at %d baud\r\n"
       "test terminal for C comm isr's in power 1.X, turbo 1.X\r\n"
	   "hit \"ESC\" to exit\r\n\n", port, speed );
   if ( comm_open ( port, speed ) )
   {
      while ( result != 27 && c != TIMEOUT)
      {
	 if ( kbhit() )                          /* Check for keyboard press */
	 {
	    result = (char) getch();
	    if( result != 27 )
	    {
	      comm_putc ( result );              /* If ! Escape */
	    }                                    /* Output the char */
	 }
	 if(comm_avail())
	 {                                       /* Is a char waiting? */
	   c = (char) comm_getc(3);
	   switch ( c )
	   {
	     case '\t' :                         /* convert tabs */
		for ( c = 0; c < 8; c++ )
		putch ( ' ' );
		break;
	     case TIMEOUT :                      /* check for timeout */
		cprintf("\r\nPort Timed Out\r\n");
		break;                           /* error */
	     case '\r':
		cprintf("%c%c",'\015','\012');
		break;
	     default :
		cprintf ("%c", c );              /* put char to screen */
		break;
	   }                                     /* End of switch */
	 }                                       /* End of if (comm_avail()) */
      }                                          /* End of while */

      cprintf("\nClosing Port %d\n",port);
      comm_close();
   }
   else
   {
      cprintf ( "\r\nUnable to find port %d\r\n", port );
   }
   return ( 0 );
}

#endif

