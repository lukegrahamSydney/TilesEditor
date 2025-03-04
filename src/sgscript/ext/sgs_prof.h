
#ifndef SGS_PROF_H_INCLUDED
#define SGS_PROF_H_INCLUDED


#ifdef __cplusplus
extern "C" {
#endif


#ifndef HEADER_SGSCRIPT_H
# define HEADER_SGSCRIPT_H <sgscript.h>
#endif
#include HEADER_SGSCRIPT_H
#ifndef HEADER_SGS_UTIL_H
# define HEADER_SGS_UTIL_H <sgs_util.h>
#endif
#include HEADER_SGS_UTIL_H


typedef struct _sgs_Prof
{
	int mode;
	sgs_VHTable ctx2prof;
	/* mode 1 / 3 */
	sgs_VHTable timings;
	/* mode 2 */
	double* ictrs;
	uint32_t* iexcs;
}
sgs_Prof;

typedef struct _sgs_ProfData
{
	sgs_Prof* prof;
	sgs_HookFunc hfn;
	void* hctx;
	/* mode 1 / 3 */
	sgs_MemBuf keytmp;
	sgs_MemBuf frametmp;
	/* mode 2 */
	int prev;
	int32_t instr;
	double starttime;
}
sgs_ProfData;


#define SGS_PROF_FUNCTIME 1
#define SGS_PROF_OPTIME   2
#define SGS_PROF_MEMUSAGE 3

#define SGS_PROFDUMP_DATAONLY 0 /* only data and nothing else, for parsing */
#define SGS_PROFDUMP_MINREAD  1 /* the minimum readable data, for separate files */
#define SGS_PROFDUMP_FULL     2 /* with prefixes and suffixes, for finding in stdout */


/* initialize profiler and attach to context */
void sgs_ProfInit( SGS_CTX, sgs_Prof* P, int mode );

/* attach profiler to another context */
void sgs_ProfAttach( SGS_CTX, sgs_Prof* P );

/* detach profiler from context */
void sgs_ProfDetach( SGS_CTX, sgs_Prof* P );

/* close profiler, detaching from all contexts */
void sgs_ProfClose( SGS_CTX, sgs_Prof* P );

/* dump profiler measurements */
/* it's using the sgs_Write* API so output file can be overridden there */
#define sgs_ProfDump( C, P ) sgs_ProfDumpExt( C, P, SGS_PROFDUMP_FULL )
void sgs_ProfDumpExt( SGS_CTX, sgs_Prof* P, int pfxsfx );


#ifdef __cplusplus
}
#endif

#endif
