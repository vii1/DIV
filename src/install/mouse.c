#include <i86.h>
#include <direct.h>
#include <string.h>
#include "video.h"
#include "mouse.h"

static float m_x = 320.f, m_y = 240.f;
int			 mouse_ratio = 3;
int			 prevMouseX, prevMouseY;
int			 mouseX, mouseY;
word		 mouseButtons;

void read_mouse()
{
	union REGS regs;
	short	   ix, iy;

	memset( &regs, 0, sizeof( regs ) );
	regs.w.ax = 3;
	int386( 0x33, &regs, &regs );

	mouseButtons = regs.w.bx;

	memset( &regs, 0, sizeof( regs ) );
	regs.w.ax = 0xb;
	int386( 0x33, &regs, &regs );

	ix = regs.w.cx;
	iy = regs.w.dx;

	m_x += (float)ix / ( 1.0 + ( (float)mouse_ratio / 3.0 ) );
	m_y += (float)iy / ( 1.0 + ( (float)mouse_ratio / 3.0 ) );

	if( m_x < 0 ) m_x = 0;
	if( m_y < 0 ) m_y = 0;
	if( m_x > screen.xres - 1 ) m_x = screen.xres - 1;
	if( m_y > screen.yres - 1 ) m_y = screen.yres - 1;

	prevMouseX = mouseX;
	prevMouseY = mouseY;

	mouseX = (int)m_x;
	mouseY = (int)m_y;
}
