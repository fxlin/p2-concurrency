# Concurrency

```
git clone https://github.com/fxlin/p2-concurrency
```

This project has two experiments. 

Clone the whole repository first. Follow the project description, tinker with the given benchmark program and reproduce the demo results. Finish the assignments. 

## Work with the server
![](figures/workflow.png)
*An overview figure. See details below.* 

## Prerequisite: connect to server

This experiment should be finished on the **granger1** server. NOT **labsrv06** which many of you use in project 1.

### (Optional) Run benchmarks on your local machine instead of granger1? 

#### Linux

Make sure your machine has more than a dozen of cores (e.g. Ryzen 9, Threadripper, some Intel i9). Recommended OS: Ubuntu 20.04. Other Linux distros should work as well. 

```
sudo apt install build-essential
sudo apt install cmake gcc gcc-aarch64-linux-gnu 
# python for plotting
sudo apt install python3 python3-pip
pip3 install bokeh
# for exercises
sudo apt install libglib2.0-dev
```

#### Windows

Native compilation possible with MINGW-64. 

WSL not recommended -- it may have scalability bottleneck. 

## Build the given code

Follow the [CMake](cmake.md) instructions. 

## Set up profiler/tracer

* Follow the instructions on [Intel VTune](./vtune.md)

* [A simple tracing facility](./measurement.md) 

## Experiments

* **Exp1: Races & Synchronization** 
  * [Tutorial](./exp1.md)
<!---  * [Exercises](./exp1-assignment.md) (cs6456 students: refer to MS teams for your assignments instead) --->
* **Exp2: Scalability** 
  * [Tutorial](./exp2.md)
<!----  * [Exercises](./exp2-assignment.md) (cs6456 students: refer to MS teams for your assignments instead) --->

## Assignment weights

| Exp                                 | Weights |
| ----------------------------------- | ------- |
| Exp1               | 50      |
| Exp2        | 50      |

*Credits: inspired by UCLA "Operating Systems Principles"*

