# injector (ARM64 Edition)

This is a fork of [injector by thelink2012](https://github.com/thelink2012/injector) aimed at `arm64-v8a / aarch64` platform (Android & Linux).

It doesn't support same exact functionality but the basics are there. 

Here's a list of supported functions:

```
WriteMemory
WriteMemoryRaw
ReadMemory
ReadMemoryRaw
ProtectMemory
UnprotectMemory
MakeB (equivalent to MakeJMP)
MakeNOP
MakeRET
```

There are a few new functions introduced:

- `WriteMemoryNoTr` and `ReadMemoryNoTr` - Write/Read memory addresses without translation

- `MakeBR` - for use with branches to memory areas further away than `0x7fffffc` - takes 3 instructions, uses register `X16`

- `MakeBLR` - for use with calls to memory areas further away than `0x7fffffc` - takes 3 instructions, uses register `X16`

## TODO

- add `MakeBL`

- Memory page protection reading - this is specific to the Linux kernel, requires reading of `/proc/self/maps` - for now you have to manually designate if the memory area is executable or not

- Branch destination calculation

- Other stuff (`hooking.hpp`, `calling.hpp`, `utility.hpp`, `assembly.hpp`)
