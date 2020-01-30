/****************************************************************************
*
*						MegaGraph Graphics Library
*
*  ========================================================================
*
*    The contents of this file are subject to the SciTech MGL Public
*    License Version 1.0 (the "License"); you may not use this file
*    except in compliance with the License. You may obtain a copy of
*    the License at http://www.scitechsoft.com/mgl-license.txt
*
*    Software distributed under the License is distributed on an
*    "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
*    implied. See the License for the specific language governing
*    rights and limitations under the License.
*
*    The Original Code is Copyright (C) 1991-1998 SciTech Software, Inc.
*
*    The Initial Developer of the Original Code is SciTech Software, Inc.
*    All Rights Reserved.
*
*  ========================================================================
*
*
* Language:     ANSI C
* Environment:  IBM PC (MS DOS)
*
* Description:  Module for performing simplified I/O port manipulations.
*               These routines are not particularly fast, but the serve
*               the purpose that we require since speed is not essential.
*
*
****************************************************************************/

#include "mgl.h"

/*----------------------------- Implementation ----------------------------*/

ushort UV_rdinx(ushort port,ushort index)
/****************************************************************************
*
* Function:     UV_rdinx
* Parameters:   port    - I/O port to read value from
*               index   - Port index to read
* Returns:      Byte read from 'port' register 'index'.
*
****************************************************************************/
{
	if (port == 0x3C0)  UV_inp(0x3DA);	/* Reset attribute reg flip-flop*/
	UV_outp(port,index);
	return UV_inp(port+1);
}

void UV_wrinx(ushort port,ushort index,ushort value)
/****************************************************************************
*
* Function:     UV_wrinx
* Parameters:   port    - I/O port to write to
*               index   - Port index to write
*               value   - Byte to write to port
*
* Description:  Writes a byte value to the 'port' register 'index'.
*
****************************************************************************/
{
	UV_outp(port,index);
	UV_outp(port+1,value);
}

void UV_modinx(ushort port,ushort index,ushort mask,ushort value)
/****************************************************************************
*
* Function:     UV_modinx
* Parameters:   port    - I/O port to modify
*               index   - Port index register to modify
*               mask    - Mask of bits to modify (1 for valid bit)
*               value   - New value to store in the modified bits
*
* Description:  Read the current value of the specified port, and modifies
*               the specified bits and store the result back in the
*               port.
*
****************************************************************************/
{
	UV_wrinx(port,index,(UV_rdinx(port,index) & ~mask) | (value & mask));
}

void UV_setinx(ushort port, ushort index, ushort mask)
/****************************************************************************
*
* Function:		UV_setinx
* Parameters:	port	- I/O port to set
*				index	- Index register to program
*				mask	- Mask of bits to set
*
* Description:	Sets the bits specified by the mask to a logical one for
*				the I/O port index register.
*
****************************************************************************/
{
	UV_wrinx(port,index,UV_rdinx(port,index) | mask);
}

void UV_clrinx(ushort port, ushort index, ushort mask)
/****************************************************************************
*
* Function:		UV_clrinx
* Parameters:	port	- I/O port to set
*				index	- Index register to program
*				mask	- Mask of bits to clear
*
* Description:	Clears the bits specified by the mask to a logical zero for
*				the I/O port index register.
*
****************************************************************************/
{
	UV_wrinx(port,index,UV_rdinx(port,index) & ~mask);
}
