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
MakeBL (equivalent to MakeCALL)
MakeNOP
MakeRET
```

There are a few new functions introduced:

- `WriteMemoryNoTr` and `ReadMemoryNoTr` - Write/Read memory addresses without translation

- `MakeBR` - for use with branches to memory areas further away than `0x7fffffc` - takes 3 instructions, uses register `X16`, +/- 4GB distance from `at`

- `MakeBLR` - for use with calls to memory areas further away than `0x7fffffc` - takes 3 instructions, uses register `X16`, +/- 4GB distance from `at`

- `MakeBRPointer` - creates a BR instruction with a dereferenced function pointer pointer, for use with branches to anywhere in memory - takes 3 instructions, uses registers `X16` and `X17`

- `MakeBLRPointer` - creates a BLR instruction with a dereferenced function pointer pointer, for use with calls to anywhere in memory - takes 3 instructions, uses registers `X16` and `X17`

## TODO

- Memory page protection reading - this is specific to the Linux kernel, requires reading of `/proc/self/maps` - for now you have to manually designate if the memory area is executable or not

- Branch destination calculation

- Other stuff (`hooking.hpp`, `calling.hpp`, `utility.hpp`, `assembly.hpp`)

- Compile-time assertion and validation of memory addresses (must be aligned by 4 bytes for ARM)
