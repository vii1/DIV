#include <stdlib.h>
#include "video.h"
#include "rect.h"

Rect rect( int x, int y, int an, int al )
{
	Rect r;
	r.x = x;
	r.y = y;
	r.an = an;
	r.al = al;
	return r;
}

void recortar( Rect* r )
{
	Rect s = { 0 };
	s.an = screen.xres;
	s.al = screen.yres;
	*r = interseccion( *r, s );
}

Rect interseccion( Rect a, Rect b )
{
	Rect r;
	r.x = max( a.x, b.x );
	r.y = max( a.y, b.y );
	r.an = max( min( a.x + a.an, b.x + b.an ) - r.x, 0 );
	r.al = max( min( a.y + a.al, b.y + b.al ) - r.y, 0 );
	return r;
}

int rectRect( Rect a, Rect b )
{
	return a.x + a.an >= b.x && a.x <= b.x + b.an && a.y + a.al >= b.y && a.y <= b.y + b.al;
}

int inRect( int x, int y, Rect r )
{
	return x >= r.x && y >= r.y && x < r.x + r.an && y < r.y + r.al;
}
