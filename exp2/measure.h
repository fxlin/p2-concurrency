/* this relies on the logging facility.
 * make sure to include log-xzl.h before this header.
 */
#ifndef _CTM_H
#define _CTM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <sys/types.h>
#include <sys/time.h>

typedef uint32_t u32;

#define USEC_PER_SEC	1000000L

#define	k2_gettimeofday(x) gettimeofday(x, NULL)

#ifndef K2_NO_MEASUREMENT
u32 k2_measure(const char *msg);
void k2_measure_flush(void);
int k2_measure_clean(void);
#else
#define k2_do_measure(msg)
static inline void k2_measure(const char *msg) { }
static inline void k2_measure_flush(void){ }
static inline void k2_flush_measure_format(void){ }
static inline void k2_calibrate(void){ }
static inline int k2_measure_clean(void){ return 0; }
#endif

#if 0
/* global perf stat. note this is indep of per-file KAGE_NO_MEASUREMENT */
struct _perf_stat {
	/* measurement buffer overflow. */
	u32 mb_overflow;
#ifdef CONFIG_ARCH_OMAP_M3
	/* hetero, platform limits */
	u32 m3_redirect_toarm;
	u32 m3_redirect_tothumb;
	u32 m3_l1_swap;
	/* exceptions */
	u32 m3_busfault;
	u32 m3_hardfault;
	u32 m3_usagefault;
	u32 m3_memmanagefault;
#endif
	/* xxx */
};

/* the global perf stat d/s.
 * in theory, any access to it should use lock.
 * XXX do that later XXX
 */
extern struct _perf_stat k2_perf_stat;

extern void k2_perf_stat_snapshot(struct _perf_stat *out);
#endif


#ifdef __cplusplus
}		// extern C
#endif

#endif
