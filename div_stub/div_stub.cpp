#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <errno.h>
#include <string.h>

main( int argc, char *argv[] )
{
  char *av[4];
  auto char cmdline[128];

  av[0]="DIV32RUN.DLL";       /* Locate the DIV/32 loader */
  av[1]=argv[0];              /* name of executable to run */
  av[2]=getcmd(cmdline);      /* command line */
  av[3]=NULL;                 /* end of list */
  execvp(av[0],av);
  puts("Error:");
  puts(av[0]);
  puts(strerror(errno));
  exit(1);                    /* indicate error */
}
