<div align="center">
    <div>
        <img src="https://githacks.org/xerox/nasa-mapper/-/raw/baa56396025feca63e9fa85b7fc8c89efa34b45a/img/mapped.png"/>
    </div>
    <img src="https://githacks.org/xerox/nasa-mapper/-/raw/baa56396025feca63e9fa85b7fc8c89efa34b45a/img/AMD-supported-green.svg"/>
    <img src="https://githacks.org/xerox/nasa-mapper/-/raw/baa56396025feca63e9fa85b7fc8c89efa34b45a/img/Intel-supported-green.svg"/>
    <img src="https://githacks.org/xerox/nasa-mapper/-/raw/baa56396025feca63e9fa85b7fc8c89efa34b45a/img/Windows%2010-Supported-green.svg"/>
    <img src="https://githacks.org/xerox/nasa-mapper/-/raw/baa56396025feca63e9fa85b7fc8c89efa34b45a/img/Spectre-Unsupported-red.svg"/>
</div>

# credit

* buck#0001 - contributed to nasa-tables which is inherited in this project...

# nasa-mapper

Map a driver into specific processes only, with zero allocations in the kernel. The driver is allocated in a suspended runtimebroker.exe which is created with the sole
purpose of containing allocated memory. The memory is then exposed to the context running this mapper code via a pml4e insertion at index 70. This keeps all memory
out of the kernels paging tables. On the down side, whatever driver mapped with nasa-mapper is not globally mapped! Do not switch contexts whilst executing
your manually mapped driver...

Physmeme and nasa-tables are both inherited into this project, using nasa-patch along with this project is ideal and provides for a very isolated and unique way to 
have execution in CPL0. If there are any issues with the code make an issue (post a minidump) and detail everything about the issue!

### What?

nasa-mapper is like every other driver mapper except the driver is not mapped into the kernel, only specific contexts/processes you decide to map the driver into.
There are limitations to this driver mapper. Firstly your driver is not going to be globally mapped. Its only going to be accessable from inside of your context. 
You can still call kernel functions, but you cannot switch contexts. In other words do not call `KeStackAttachProcess` directly. You can call MmCopyVirtualMemory though
since the context switch will happen inside of `ntoskrnl` which is globally mapped.

### How?

memory that is going to be executed in ring-0 in usermode?? SMEP??, well just because the memory is mapped into usermode does not mean it is usermode accessable. nasa-mapper allocates
the driver in a runtimebroker.exe and then makes all the memory in runtimebroker.exe kernel memory & makes it executable. nasa-mapper then inserts a pml4e at index 70 into a desired
processes pointing at the allocated driver in the runtimebroker, thus mapping the "drivers" memory into the process.

### Why?

Keeping your driver out of the kernels paging tables. Most driver mappers map a driver into a kernel pool (ExAllocatePool). Physmeme, Kdmapper, Drvmapper, all do this, its easily
detected and easy to dump. This keeps your driver inside of your context :)

I guess you can call this physmeme v2? You can use any driver that exposes physical memory read/write with this driver mapper, simply replace the vulnerable driver inside of raw_driver.hpp.

# Spectre

Please disable spectre/meltdown when using nasa-tables/nasa-patch/nasa-mapper. You can download a program to disable spectre/meltdown [here](https://www.grc.com/inspectre.htm).