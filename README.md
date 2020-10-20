<div align="center">
    <div>
        <img src="https://githacks.org/xerox/nasa-mapper/-/raw/baa56396025feca63e9fa85b7fc8c89efa34b45a/img/mapped.png"/>
    </div>
    <img src="https://githacks.org/xerox/nasa-mapper/-/raw/baa56396025feca63e9fa85b7fc8c89efa34b45a/img/AMD-supported-green.svg"/>
    <img src="https://githacks.org/xerox/nasa-mapper/-/raw/baa56396025feca63e9fa85b7fc8c89efa34b45a/img/Intel-supported-green.svg"/>
    <img src="https://githacks.org/xerox/nasa-mapper/-/raw/baa56396025feca63e9fa85b7fc8c89efa34b45a/img/Windows%2010-Supported-green.svg"/>
    <img src="https://githacks.org/xerox/nasa-mapper/-/raw/baa56396025feca63e9fa85b7fc8c89efa34b45a/img/Spectre-Unsupported-red.svg"/>
</div>

# nasa-mapper

Map a driver into specific processes only, with zero allocations in the kernel. The driver is allocated in a suspended runtimebroker.exe which is created with the sole
purpose of containing allocated memory. The memory is then exposed to the context running this mapper code via a pml4e insertion at index 70. This keeps all memory
out of the kernels paging tables. On the down side, whatever driver mapped with nasa-mapper is not globally mapped! Do not switch contexts whilst executing
your manually mapped driver...

Physmeme and nasa-tables are both inherited into this project, using nasa-patch along with this project is ideal and provides for a very isolated and unique way to 
have execution in CPL0. If there are any issues with the code make an issue (post a minidump) and detail everything about the issue!

# Spectre

Please disable spectre/meltdown when using nasa-tables/nasa-patch/nasa-mapper.