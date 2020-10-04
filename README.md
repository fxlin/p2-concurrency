# Concurrency

This project has two experiments. 

For each experiment, clone the whole repository first. Then follow the demo description, tinker with the given benchmark program, and reproduce the demo results. Then, do the exercises and submit your answers. 

## Prerequisite (optional)

If you are doing this on your own Linux box (assuming Ubuntu/Debian): 
```
sudo apt install build-essential
sudo apt install cmake gcc gcc-aarch64-linux-gnu 
# python for plotting
sudo apt install python3 python3-pip
pip3 install bokeh
```

## How to build the experiments

[CMake](cmake.md)

## Set up profiler/tracer

* [Intel VTune](./vtune.md)

* [A simple tracing facility](./measurement.md) 

## Experiments

* **Exp1: Races & Synchronization** 
  * [Demo](./exp1.md)
  * [Exercises](./exp1-assignment.md) (cs6456 students: refer to MS teams for your assignments instead)
* **Exp2: Scalability** 
  * [Demo](./exp2.md)
  * [Exercises](./exp2-assignment.md)


Credits: inspired by UCLA "Operating Systems Principles "