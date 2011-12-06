/*
Sched_Sim.cpp
Ryan Konkul
CS385
Homework 2
CPU Scheduling Simulator
*/

The program may be compiled with

make Sched_Sim

To switch the algorithm used, at the top there are integer constants that determine the algorithm used. const int fifo may be 1 to signal that first in first out is to be the algorithm used. 

Under that, there are values for maxcpubursts and maxprocesses. This allows easy modification to parameters without using command arguments. These are overridden if command arguments are specified.

Command arguments after the input file may be maxcpubursts, maxprocesses or both. If only one int is given, that is set to the maxcpubursts while maxprocesses is left as the default value.

Using diver.bmp I collected data first using fifo and then shortest job first algorithms. I also kept the maxCPUbursts at 20 and maxProcesses at 40.

For the fifo algorithm the average waiting time was
293.917
the standard deviation was
283.549

Under the same parameters, shortest job first had an average waiting time of 249.074 with a standard deviation of 284.506. 

Changing the input file to light_drops.bmp and maxcpubursts to 30 and maxprocesses to 50, under fifo, average waiting time became 525.828. Standard deviation was 515.526.
Under sjf: avg waiting time - 414.923, stddev - 519.848.

Under all situations, shortest job first performs better than first in first out. 
