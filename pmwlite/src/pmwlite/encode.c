#ifndef __ENCODE_C
#define __ENCODE_C

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
  INFO
±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/
// Disk to disk compression.
//
/*--------------------------------------------------------------------------*/
// Pseudo-logic to decompress data:
//
// movestring2:
//   len = 2;
//
// movestring:
//   MOVE ($ - off, len);
//
// decode:
//   0:
//     PUT (GETBYTE ());
//
//     GOTO decode;
//
//   1:
//     off[0-7] = GETBYTE ();
//
//     0:
//       0:
//         if (!off)
//           GOTOEXE ();
//         off -= 1;
//
//         GOTO movestring2;
//
//       1:
//         off[10] = GETBIT ();
//         off[9] = GETBIT ();
//         off[8] = GETBIT ();
//
//         GOTO movestring2;
//
//     1:
//       1:
//         count = GETBIT ();
//
//         REPEAT ((count + 1) * 2)
//         {
//           SHL (off[8-15], 1);
//           off[8] = GETBIT ();
//         }
//
//       0:
//         len = 2;
//
//         REPEAT (4)
//         {
//           len ++;
//
//           if (GETBIT ())
//             GOTO movestring;
//         }
//
//         0:
//           len[2] = GETBIT ();
//           len[1] = GETBIT ();
//           len[0] = GETBIT ();
//           len += 7;
//
//           GOTO movestring;
//
//         1:
//           len[0-7] = GETBYTE ();
//           len += 15;
//
//           GOTO movestring;

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
  INTERFACE
±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/
#include <io.h>
#include <malloc.h>
#include <mem.h>
#include <stdio.h>

#include "encode.h"

/*ֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽ*/
#pragma aux StringMatch = \
        "mov ebx,ecx",\
        "repe cmpsb",\
        "mov eax,0",\
        "setne al",\
        "add ecx,eax",\
        "sub ebx,ecx",\
        parm [edi] [esi] [ecx] modify [eax ebx ecx esi edi] value [ebx];

/*ֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽ*/
#define DictBits        12
#define DictSize        (1 << DictBits)

/*תתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתת*/
#define BufInSize       0x20000
#define BufOutSize      0x2000

#define BufOutThreshold 512
#define BufInThreshold  512

#define BufLPos(p)      ((int)((p) - BufIBase + BufLBase))
#define BufPPos(l)      (BufIBase + (l) - BufLBase)

#define BufPutByte(b)   (*OBuf++ = (b))

/*תתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתת*/
#define HashBits        12
#define HashCount       (1 << HashBits)

#define HashVal(h)      ((unsigned short) (((h) >> (16 - HashBits)) + (h)) & \
                        (HashCount - 1))

#define HashFindInit()  (HashLastFound = -1)

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
  DATA
±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/
static int      LPos, LSize, Show;
static unsigned MaxProbes;

/*תתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתת*/
static FILE *BufIHandle, *BufOHandle;

static int  BufLBase;
static int  BufLSize, BufTSize, BufEncodedSize;
static int  *BufBitOPos, BufBitBuf, BufBitCount;
static char *BufIBase, *BufITop, *BufIThreshold;
static char *BufOBase, *BufOTop, *BufOThreshold;
static char *IBuf, *OBuf;

/*תתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתת*/
static int   *HashVal, HashLink, HashLastFound;
static short *HashNext, *HashPrev, *HashBase, *HashTop;

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
  CODE
±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

/*°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
  Compress data from input file to output file.
  In:
    dst - destination file handle (data will be written to the current pos)
    src - source file handle (data will be read from the current pos)
    size - size in bytes of block to compress
    probes - maximum number of probes to do per byte for string matches
    show - show progress percentage during compression flag
  Out:
    int - size of compressed block or error code if compression failed
          (-1 = not enough mem, -2 = error reading file, -3 = error writing)
  Notes:
  ) During compression a progress percentage will be put to the current
    position on the screen in the format "%xx". After completion the
    compressed data as a percentage of the original size will be put in the
    format "xx%", and the cursor will be left right after that.
  ) Do not pass a size smaller than 2 bytes.
  ) If the compression fails, a partially compressed file may have been
    written to disk and you are responsible for cleaning it up.
  ) If compression is successful, file pointers are left at the end of the
    file blocks read and written.
°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°*/
int Compress (FILE *dst, FILE *src, unsigned size, unsigned probes, int show)
{
  int r, i, CodeLen;
  int i0, i1;

/*ֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽֽ*/
  Show = show;

  if (!HashInit ())
    return ENCODE_NOTENOUGHMEM;

  if (r = BufInit (dst, src, size))
  {
    HashUninit ();
    return r;
  }

/*תתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתת*/
  HashAdd (HashVal (*(unsigned short *)IBuf), 0);

  *OBuf++ = *IBuf++;

  MaxProbes = probes;

  LPos = 1;
  LSize = size-1;

/*ִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִ*/
  while (LSize)
  {
    CodeLen = PutCode ();

    LSize -= CodeLen;

/*תתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתת*/
    if (LPos >= DictSize)
    {
      for (i=DictSize; i>DictSize-CodeLen; i--)
        HashDelete (HashVal (*(unsigned short *)(IBuf-i)), LPos-i);
    }
    else if (LPos+CodeLen >= DictSize)
    {
      for (i=LPos; i>LPos-(CodeLen-(DictSize-LPos)); i--)
        HashDelete (HashVal (*(unsigned short *)(IBuf-i)), LPos-i);
    }

/*תתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתת*/
    while (CodeLen--)
    {
      i = *(unsigned short *)(IBuf++);
      HashAdd (HashVal (i), LPos++);
    }

/*תתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתת*/
    if (r = BufUpdate ())
      break;
  }

/*--------------------------------------------------------------------------*/
  if (!r)
  {
    BufPutBit (1);
    BufPutByte (0);
    BufPutBits (0, 2);

    r = BufFlush ();
  }

  BufUninit ();
  HashUninit ();

  return r;
}

/*°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°*/
static int PutCode (void)
{
  int  off, len, hash, i0, i1, i2;
  char *c;

/*ִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִ*/
  if (LSize == 1)
  {
    BufPutBit (0);
    BufPutByte (*IBuf);
    return 1;
  }

/*תתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתת*/
  hash = HashVal (*(unsigned short *)IBuf);
  len = 1;

  HashFindInit ();

/*--------------------------------------------------------------------------*/
  for (i2=0; ((i0 = HashFind (hash)) != -1) && (i2 < MaxProbes); i2++)
  {
    c = BufPPos (i0);
    i0 = (IBuf-1) - c;
    i1 = StringMatch (c, IBuf, min (270, LSize));

    if ((i1 == 2) && (i0 >= 0x800))
      continue;

    if (i1 > len)
    {
      off = i0;
      len = i1;
    }
  }

/*--------------------------------------------------------------------------*/
  if (len < 2)
  {
    BufPutBit (0);
    BufPutByte (*IBuf);
  }
  else if (len == 2)
  {
    BufPutBit (1);

    if (off < 255)
    {
      BufPutByte ((char)off + 1);
      BufPutBits (0, 2);
    }
    else
    {
      BufPutByte ((char)off);
      BufPutBits (8 | ((off >> 8) & 7), 5);
    }
  }
  else
  {
    BufPutBit (1);
    BufPutByte ((char)off);

    if (off < 0x100)
      BufPutBits (2, 2);
    else if (off < 0x400)
      BufPutBits (0x18 | ((off >> 8) & 3), 5);
    else
      BufPutBits (0x70 | ((off >> 8) & 15), 7);

    if (len < 7)
      BufPutBits (1, len - 2);
    else if (len < 15)
      BufPutBits ((len-7) & 7, 8);
    else
    {
      BufPutBits (1, 5);
      BufPutByte (len - 15);
    }
  }

  return len;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

/*°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°*/
static int BufInit (FILE *dst, FILE *src, unsigned size)
{
  int r;

/*--------------------------------------------------------------------------*/
  if (!(BufOBase = malloc (BufOutSize)))
    return ENCODE_NOTENOUGHMEM;

  if (!(BufIBase = malloc (BufInSize)))
  {
    free (BufOBase);
    return ENCODE_NOTENOUGHMEM;
  }

/*תתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתת*/
  BufOTop = BufOBase + BufOutSize;
  BufOThreshold = BufOTop - BufOutThreshold;
  BufITop = BufIBase + BufInSize;
  BufIThreshold = BufITop - BufInThreshold;

  OBuf = BufOBase;
  IBuf = BufITop;

  BufIHandle = src;
  BufOHandle = dst;

  BufLBase = -BufInSize;
  BufLSize = size;
  BufTSize = size;
  BufEncodedSize = 0;

  BufBitBuf = 0;
  BufBitCount = 0;

  if (r = BufUpdate ())
  {
    BufUninit ();
    return r;
  }

  if (Show)
  {
//    fprintf (stdout, "%%00");
//    fflush (stdout);
    cprintf ("%%00");
  }

  return 0;
}

/*°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°*/
static void BufUninit (void)
{
  free (BufIBase);
  free (BufOBase);
}

/*°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°*/
static void BufPutBit (int b)
{
  BufBitBuf = (BufBitBuf << 1) | b;

  if (++BufBitCount == 1)
  {
    BufBitOPos = (int *)OBuf;
    OBuf += 4;
  }
  else if (BufBitCount == 32)
  {
    *BufBitOPos = BufBitBuf;
    BufBitBuf = 0;
    BufBitCount = 0;
  }
}

/*°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°*/
static void BufPutBits (int b, int c)
{
  int i;

/*תתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתת*/
  i = BufBitCount;
  BufBitCount += c;

  if (BufBitCount == c)
  {
    BufBitBuf = b;
    BufBitOPos = (int *)OBuf;
    OBuf += 4;
  }
  else if (BufBitCount < 32)
    BufBitBuf = (BufBitBuf << c) | b;
  else if (BufBitCount == 32)
  {
    *BufBitOPos = (BufBitBuf << c) | b;
    BufBitBuf = 0;
    BufBitCount = 0;
  }
  else
  {
    *BufBitOPos = (BufBitBuf << (32 - i)) | (b >> (BufBitCount - 32));
    BufBitOPos = (int *)OBuf;
    OBuf += 4;
    BufBitCount -= 32;
    BufBitBuf = b & ((1 << BufBitCount) - 1);
  }
}

/*°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°*/
//static void BufPutByte (int b)
//{
//  *OBuf++ = b;
//}

/*°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°*/
static int BufUpdate (void)
{
  int  i0, i1, i2;
  char *c;

/*--------------------------------------------------------------------------*/
  if (IBuf >= BufIThreshold)
  {
    c = IBuf - 0x10000;
    i0 = BufITop - c;
    i1 = c - BufIBase;
    i2 = min (i1, BufLSize);

    memmove (BufIBase, c, i0);

//    if (read (BufIHandle, BufITop - i1, i2) != i2)
//      return ENCODE_ERRORREADING;
    if(i2!=0){                                                /**************/
        if((fread(BufITop-i1,i2,1,BufIHandle))<1)             /**************/
        return ENCODE_ERRORREADING;                           /**************/
    }                                                         /**************/
    IBuf = BufIBase + 0x10000;
    BufLBase += i1;
    BufLSize -= i2;
  }

/*תתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתת*/
  if (OBuf >= BufOThreshold)
  {
    c = (char *)min ((int)OBuf, (int)BufBitOPos);
    i0 = BufOTop - c;
    i1 = c - BufOBase;

//    if (write (BufOHandle, BufOBase, i1) != i1)
//      return ENCODE_ERRORWRITING;
    if(i1!=0){                                                /**************/
        if((fwrite(BufOBase,i1,1,BufOHandle))<1)              /**************/
        return ENCODE_ERRORWRITING;                           /**************/
    }                                                         /**************/
    memmove (BufOBase, c, i0);

    OBuf -= i1;
    BufBitOPos = (int *)((char *)BufBitOPos - i1);
    BufEncodedSize += i1;

    if (Show)
    {
//      fprintf (stdout, "\x08\x08%02d", BufLPos (IBuf) * 100 / BufTSize);
//      fflush (stdout);
      cprintf ("\x08\x08%02d", BufLPos (IBuf) * 100 / BufTSize);
    }
  }

  return 0;
}

/*°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°*/
static int BufFlush (void)
{
  int i;

/*תתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתת*/
  if (BufBitCount)
    *BufBitOPos = BufBitBuf << (32 - BufBitCount);

  i = OBuf - BufOBase;

//  if (write (BufOHandle, BufOBase, i) != i)
//    return ENCODE_ERRORWRITING;
  if((fwrite(BufOBase,i,1,BufOHandle))<1)
   return ENCODE_ERRORWRITING;

  BufEncodedSize += i;

  if (Show)
  {
//    fprintf (stdout, "\x08\x08\x08   \x08\x08\x08");
//    fflush (stdout);
    cprintf ("\x08\x08\x08   \x08\x08\x08");

  }

  return BufEncodedSize;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

/*°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°*/
static int HashInit (void)
{
  if (!(HashVal = malloc (DictSize * sizeof (int))))
    return 0;

  if (!(HashNext = malloc (DictSize * sizeof (short))))
  {
    free (HashVal);
    return 0;
  }

  if (!(HashPrev = malloc (DictSize * sizeof (short))))
  {
    free (HashNext);
    free (HashVal);
    return 0;
  }

  if (!(HashBase = malloc (HashCount * sizeof (short))))
  {
    free (HashPrev);
    free (HashNext);
    free (HashVal);
    return 0;
  }

  if (!(HashTop = malloc (HashCount * sizeof (short))))
  {
    free (HashBase);
    free (HashPrev);
    free (HashNext);
    free (HashVal);
    return 0;
  }

/*תתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתת*/
  memset (HashBase, 0xff, HashCount * sizeof (short));
  memset (HashTop, 0xff, HashCount * sizeof (short));

  HashFindInit ();

  HashLink = 0;

  return 1;
}

/*°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°*/
static void HashUninit (void)
{
  free (HashTop);
  free (HashBase);
  free (HashPrev);
  free (HashNext);
  free (HashVal);
}

/*°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°*/
static void HashAdd (unsigned short h, int v)
{
  int i;

/*תתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתת*/
  i = HashBase[h];
  HashBase[h] = HashLink;

  HashVal[HashLink] = v;
  HashPrev[HashLink] = -1 - h;

  if (i < 0)
  {
    HashNext[HashLink] = -1 - h;
    HashTop[h] = HashLink;
  }
  else
  {
    HashNext[HashLink] = i;
    HashPrev[i] = HashLink;
  }

  HashLink ++;
}

/*°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°*/
static void HashDelete (unsigned short h, int v)
{
  int i;

/*תתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתת*/
  i = HashTop[h];
  HashTop[h] = HashPrev[i];

  if (HashTop[h] < 0)
    HashBase[h] = -1;
  else
    HashNext[HashPrev[i]] = HashNext[i];

  if (--HashLink != i)
  {
    HashVal[i] = HashVal[HashLink];
    HashNext[i] = HashNext[HashLink];
    HashPrev[i] = HashPrev[HashLink];

    if (HashNext[i] < 0)
      HashTop[-1 - HashNext[i]] = i;
    else
      HashPrev[HashNext[i]] = i;

    if (HashPrev[i] < 0)
      HashBase[-1 - HashPrev[i]] = i;
    else
      HashNext[HashPrev[i]] = i;
  }
}

/*°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°*/
static int HashFind (unsigned short h)
{
  if (HashLastFound < 0)
  {
    if ((HashLastFound = HashBase[h]) < 0)
      return -1;
    else
      return HashVal[HashLastFound];
  }
  else
  {
    if ((HashLastFound = HashNext[HashLastFound]) < 0)
      return -1;
    else
      return HashVal[HashLastFound];
  }
}

#endif

