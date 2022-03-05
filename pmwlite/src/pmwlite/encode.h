#ifndef __ENCODE_H
#define __ENCODE_H

/*様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様*/
//int Compress (int dst, int src, unsigned size, unsigned probes, int show);
int Compress (FILE *dst, FILE *src, unsigned size, unsigned probes, int show);

/*様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様*/
#define ENCODE_NOTENOUGHMEM     -1
#define ENCODE_ERRORREADING     -2
#define ENCODE_ERRORWRITING     -3

#define ENCODE_PROBESMAX        4080
#define ENCODE_PROBESMANY       100
#define ENCODE_PROBESNORM       30
#define ENCODE_PROBESFEW        10
#define ENCODE_PROBESMIN        4

#endif

