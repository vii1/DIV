#ifndef __RECT_H__
#define __RECT_H__

typedef struct _Rect {
	int x, y, an, al;
} Rect;

Rect rect( int x, int y, int an, int al );
void recortar( Rect* r );
Rect interseccion( Rect a, Rect b );
int rectRect( Rect a, Rect b );
int inRect(int x, int y, Rect r);

#endif // __RECT_H__
