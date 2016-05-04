#ifndef Config_H
#define Config_H

#ifndef READDEPTH
	#define READDEPTH 1024
#endif
#ifndef CELLNUM
	#define CELLNUM 4096
#endif
#ifndef NEGLECTCELL
	#define NEGLECTCELL 3
#endif
#ifndef TIME_PER_SLICE
        #define TIME_PER_SLICE 0.966 //0.483 //[ns/slice]
#endif
#ifndef D_HGAIN
        #define D_HGAIN 6.65 //17.02
#endif

//#ifdef D_LGAIN
//#define D_LGAIN 
//#endif

#endif
