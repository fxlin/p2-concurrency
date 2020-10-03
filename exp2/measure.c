/*
 *
 * Simple tracepoints facility
 *
 * USAGE:
 * see main()
 *
 * TEST:
 *  cc measure.c  -DTEST_MEASUREMENT -o /tmp/measure
 *
 * LIMITATIONS:
 * pre-defined max number of tracepoints. (NR_SAMPLES).
 * Beyond that we will wrap around.
 *
 * K2 project, 2013
 *
 * Jun 2020. Simplified.
 * Aug 2016. Used in the creek project (c++).
 * May 2015. ported to userspace.
 * Contact: Felix Xiaozhu Lin <linxz02@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 * known issues:
 * - on M3 gettimeofday() does not work properly.
 * - even with HAS_LOCK, still crashes the kernel in multicore case.
 * (strnlen?)
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "measure.h"
#include "log.h"

//struct _perf_stat k2_perf_stat = {0};

/* -------------------------------------------------
 * take timestamp and save msg. minimize overhead
 * ------------------------------------------------- */
#define NR_SAMPLES 256
struct measure_sample {
	const char *msg;
	u32 ts; 		/* in cycles. from hw counter. */
	u32 tod;		/* in us. from gettimeofday(). */
};
static int count = 0; /* # of samples since last flush */
static struct measure_sample samples[NR_SAMPLES];
static int overflow = 0; /* tracebuf overflow counters */

/* drop all samples without looking at them (e.g.,
 * they may include invalid pointers).
 * return the # of samples dropped.
 * XXX make this atomic?
 */
int k2_measure_clean(void)
{
	int ret = count;
	count = 0;
	return ret;
}

/* take a snapshot of the current sw perf cnt, timestamped .
   perf sensitive.

   Since perfcnt of A9 won't tick when the core is idle,
   for A9 every measure we take two readings: perfcnt and gettimeofday().
   The latter comes from a 32KHz timer.

   return the current time (in us) for being handy.
 */
u32 k2_measure(const char *msg)
{
	struct timeval stamp;
	u32 tod_us;
	int i = count % NR_SAMPLES;

	if (i == NR_SAMPLES - 1) {
		/* we've reached tail (again) */
		/*
		k2_PRINT("sample overflow. bug!\n");
		k2_crash();
		 */
//		k2_perf_stat.mb_overflow ++;
		overflow ++;
	}

	samples[i].msg = msg;

	k2_gettimeofday(&stamp);
	tod_us = stamp.tv_sec * USEC_PER_SEC + stamp.tv_usec;

	samples[i].tod = tod_us;

	count ++;

	return tod_us;
}

void k2_measure_flush(void)
{
	int i = 0, j, start, end;

	/* header */
	print_to_tracebuffer("--------------------%s------#samples=%d---------------\n",
			__func__, count);

	print_to_tracebuffer("%40s "
			"%15s %15s \n",
			"msg",
			"delta(tod/us)", "now(tod)");

	/* dump all samples in cycles. The way we interpret timestamps as
	 * cycles depends on the hw */

	/* if the sample buffer has wrapped, we find the head and start from it */
	if (count > NR_SAMPLES) { /* we overflew */
		start = count % NR_SAMPLES;
		end = start + NR_SAMPLES;
	} else {
		start = 0;
		end = count;
	}

	for (j = start; j < end; j++) {
		i = j % NR_SAMPLES;
		if (i == 0)
		  print_to_tracebuffer("*"); /* dbg */
		else
		  print_to_tracebuffer(" ");
		print_to_tracebuffer("%40s "
				"%15u %15u \n",
				samples[i].msg,
				i == 0 ? 0 : (samples[i].tod - samples[i-1].tod), samples[i].tod);
	}

	print_to_tracebuffer(" TOTAL: %u us(gettimeofday) %s tracebuf overflow: %d\n",
				samples[i].tod - samples[0].tod,
				overflow == 0 ? "":"WARNING:", overflow);

	print_to_tracebuffer("---------------------------------------------------------------\n");
	count = 0;
}

/* test */
#include <unistd.h>

#ifdef TEST_MEASUREMENT
int main(int argc, char**argv) {


	k2_measure("start");
	for (int i = 0; i < 2; i++) {
		sleep(1);
		k2_measure("slept 1sec");
	}
	k2_measure_flush();

	printf("after flushing, to add two new tracepoints\n");
	for (int i = 0; i < 2; i++) {
			sleep(1);
			k2_measure("slept 1sec");
	}
	k2_measure_flush();
	printf("should see 2 tracepoints\n");

	printf("to add two more tracepoints. clean w/o output \n");
	for (int i = 0; i < 2; i++) {
			sleep(1);
			k2_measure("slept 1sec");
	}
	k2_measure_clean();

	printf("should see no tracepoint\n");
	k2_measure_flush();
}
#endif
