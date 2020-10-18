#ifndef DLL_H
#define DLL_H

void  LookForAutoLoadDlls();

extern PE      *pe[128];
extern int     nDLL;
extern void    *ExternDirs[1024];

#endif // DLL_H
