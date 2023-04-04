## To build 

```bash
$ cmake .
# Alternatively to build w/o vtune ittnotify instrumentation (can still be profiled with vtune), do
$ cmake  -DUSE_VTUNE=OFF .

$ make -j`nproc`
```

Will generate different versions of the benchmark program (list-xxx). 

## To run 

```bash
$ ./run.sh
...
./list-pmla trace-pmla.txt 4
test=list-none-4part threadNum=1 iterations=1000000 numList=4 numOperation=1000000 runTime(ms)=6 tput(Mops)=151.61
test=list-none-8part threadNum=2 iterations=1000000 numList=8 numOperation=2000000 runTime(ms)=8 tput(Mops)=234.26
...
```
run.sh will append the above output to trace files, e.g. 
```bash
xzl@DESKTOP-3ECE59G:~/p2-concurrency/exp2$ ls trace* -lh
-rw-r--r-- 1 xzl xzl 7.4K Apr  4 09:30 trace-.txt
-rw-r--r-- 1 xzl xzl 7.5K Apr  4 09:30 trace-p.txt
-rw-r--r-- 1 xzl xzl 7.8K Apr  4 09:30 trace-pm.txt
-rw-r--r-- 1 xzl xzl 7.8K Apr  4 09:30 trace-pml.txt
-rw-r--r-- 1 xzl xzl 7.8K Apr  4 09:30 trace-pmla.txt
```

## To plot the results
```bash
# first time execution, do "pip3 install bokeh"
$ ../scripts/plot.py
```
This python script parses trace files and produces a series of plots in HTML: res-0.html, res-1.html, ...
Each HTML includes additional data lines from a new trace file.

In its code, `show(pp)` will attempt to open a browser and show the htmls. If you run from a remote server or WSL, comment those lines. 

Example command:

```bash
$ ls *.txt
trace-pmla.txt  trace-pml.txt  trace-pm.txt  trace-p.txt  trace-.txt

$ ~/p2-concurrency/scripts/plot.py

$ ls *.html
res-0.html  res-2.html  res-4.html
res-1.html  res-3.html  res.html
```
A sample HTML (you are likely to see different measurement): [res-4.html](res-4.html) 


## Putting it together 

**YOU MUST UNDERSTAND run.sh and plot.py. Don't just call them and gen plot**

`run.sh` will call different benchmark programs (list-xxx) with different arguments (iterations, threads, etc). 

For each run, `run.sh` appends the benchmark program's output to a corresponding trace file (e.g. trace-p.txt). 

`plot.py` then go through all these trace files and generate a scalability plot. It's an interactive HTML. 

## Troubleshoot

If the plot or HTML is empty, it could be some of the trace files are not generated correctly. Figure out why: manually execute list-XXX, tweaking run.sh, etc. 

