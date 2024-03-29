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

This experiment should be finished on **granger1/2**. Unlike p1, the programs here can be CPU intensive. Be mindful about the server load. 

Use granger2 if granger1 is heavily loaded. 


### (Adventurous) Use your local machine instead of granger1/2? 

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

* [A simple tracing library](./measurement.md) 

## Experiments

* **Exp1: Races & Synchronization** 
  * [Description](./exp1.md)
  <!---  * [Exercises](./exp1-assignment.md) (cs6456 students: refer to MS teams for your assignments instead) --->
* **Exp2: Scalability** 
  * [Description](./exp2.md)
  <!----  * [Exercises](./exp2-assignment.md) (cs6456 students: refer to MS teams for your assignments instead) --->


*Credits: inspired by UCLA "Operating Systems Principles"*

