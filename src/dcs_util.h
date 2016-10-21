#ifndef DCS_UTIL_H_VO1MI98D
#define DCS_UTIL_H_VO1MI98D

#include <stddef.h>
#include <stdlib.h>

#define dcs_free(ptr) ({if(ptr) free(ptr); ptr=NULL;})

inline size_t dcs_size_min(size_t a, size_t b) { return a < b ? a : b; }

inline size_t dcs_size_max(size_t a, size_t b) { return a > b ? a : b; }


#ifndef DCS_BUFSIZE
#define DCS_BUFSIZE (1<<20) // 1Mib
#endif



#endif /* end of include guard: DCS_UTIL_H_VO1MI98D */
