## To build 

```
cmake .
make -j`nproc`
```

Will generate different versions of the benchmark program (list-xxx). 

## To run 

```
./run.sh
../scripts/plot.py
```

## Putting it together 

**YOU MUST UNDERSTAND run.sh and plot.py. Don't just call them and gen plot**

`run.sh` will call different benchmark programs (list-xxx) with different arguments (iterations, threads, etc). 

For each run, `run.sh` appends the benchmark program's output to a corresponding trace file (e.g. trace-p.txt). 

`plot.py` then go through all these trace files and generate a scalability plot. It's an interactive HTML. 

## Troubleshoot

If the plot or HTML is empty, it could be some of the trace files are not generated correctly. Figure out why: manually execute list-XXX, tweaking run.sh, etc. 

