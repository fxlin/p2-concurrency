# A simple tracing facility in C

I have a small tracing library (measure.[c|h]) that I used quite a lot. Being simple, it has a few advantages: 

* Easy to integrate. Minimal external dependency. Only one single C file. 
* Simple APIs. Only three functions. 
* Low overhead. No dynamic memory allocation (a static tracebuffer); no string manipulation; no IO until trace collection completes. 

## To integrate

```
// in .C 
#include "measure.h"
// in Makefile
OBJ += measure.o
```

## API

* k2_measure(const char *) emit a tracepoint to the tracebuffer. 
* k2_measure_flush() print out all tracepoints in the tracebuffer and clear the buffer

Not thread-safe. Must be locked in a multithreaded environment.

**Example:**

```
k2_measure("start");
	for (int i = 0; i < 2; i++) {
		sleep(1);
		k2_measure("slept 1sec");
	}
k2_measure_flush();
```

## Output
From the above code example: 

```
--------------------k2_measure_flush------#samples=3---------------
                                     msg   delta(tod/us)        now(tod)
*                                   start               0      3155758523
                               slept 1sec         1000079      3156758602
                               slept 1sec         1000084      3157758686
 TOTAL: 2000163 us(gettimeofday)  tracebuf overflow: 0
---------------------------------------------------------------
```

**Columns**

* "msg": the tag string included in k2_measure() call. 
* "delta": the time difference (in microseconds) as compared to the previous tracepoint. Useful for measuring the time taken in one code region. 
* "now": the absolute timestamps, in microseconds. 

