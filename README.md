# Overview

This is a quick proof of concept demonstrating best practice for high-performance LuaJIT C Binding.

This is a proof of concept applying some of the approaches documented in:
[Niklas Frykholm's presentation, LUA WorkShop 2015](http://www.frykholm.se/presentations/lua-workshop-2015/index.html#26)

- Exposing "object" as lightuserddata, with type tag is struct to detect when incorrect type is passed
- Using a shared meta-table for all lightuserdata to support operator overloading

Then further extended to leverage the LUA JIT Allocation/Sinking optimization documented there:
http://wiki.luajit.org/Allocation-Sinking-Optimization
This optimization get ride of allocation of ffi metatype when use is local (temporary vector operation for example).

# Building

luajit is retrieve from luarocks git repository as a sub-module, so make sure to update sub-modules after clone:
```git submodule update --init --recursive```

- Build is done using CMake 3, selecting Visual Studio 2015 64 bits.
- To test with LUA JIT 2.1, ensure WITH_LUAJIT21 is ON in CMake.
- If build fail with something related to read, just remove read-line library autodetected by CMake  (just set readline include/lib to a blank string in CMake GUI)

# Measure

See src/luajit_poc/bench.lua for bench code.

Rough performance measured on Intel Core i7-6700K CPU @ 4.00GHz running on Windows 10 64 bits, compiled with Visual Studio 2015 in 64 bits with luajit 2.1.
Notes that this is an ideal setup, no icode cache pressure... Just some simple measurement to get a rough idea of where we stand.

1) Set the x, y integer coordinate of a Vector2D struct to current loop index.

Call to A C LUA Binding associated to lightuserdata:
- Vector2D.set * 100,000,000.00 in 2.231s = 44,822,793.46 operation/s [64bits]
- Vector2D.set * 100,000,000.00 in 2.230s = 44,851,959.06 operation/s [32bits]

Caching Vector2D.set in a local variable (improved performance with luajit 2.0, no longer need with luajit 2.1):
- Vector2D_set * 100,000,000.00 in 2.246s = 44,514,743.73 operation/s [64bits]
- Vector2D_set * 100,000,000.00 in 2.075s = 48,197,507.80 operation/s [32bits]

Both of the above call have to cross the C/Lua language barrier. The C function has to read the parameters from the LUA Stack. 
IMHO, those performance a very impressive...

Using LUA JIT ffi metatype, bring us to performance comparable to pure C++:
- setLuaV2 (ffi struct) * 100,000,000.00 in 0.048s = 2,073,420,998.68 operation/s [64bits]
- setLuaV2 (ffi struct) * 100,000,000.00 in 0.049s = 2,054,834,322.41 operation/s [32bits]

2) Creates a new Vector2D initialized to 1, 1 and add it to a sum Vector2D on each loop iteration.

Call to A C LUA Binding associated to lightuserdata, goes through the C/LUA language barrier and call new/delete:
- Application.create/sum/DestroyVector2D * 10,000,000.00 in 1.310s = 7,631,665.20 operation/s [64bits]
- Application.create/sum/DestroyVector2D * 10,000,000.00 in 1.491s = 6,707,879.24 operation/s [32bits]

Using LUA JIT ffi metatype struct for Vector2D:
- LuaV2Create/sum/Destroy (ffi struct) * 10,000,000.00 in 0.002s = 4,057,280,688.76 operation/s [64bits]
- LuaV2Create/sum/Destroy (ffi struct) * 10,000,000.00 in 0.002s = 4,071,216,984.47 operation/s [32bits]
The LUA JIT Allocation/Sinking optimization is clearly triggered and got ride of the struct allocation. Performance is comparable with what you could get in fullly inlined C++. Since is this a 4GHz processor, we're basically at 1 cycle per iteration...

3) Pass a Vector2D to the Application

Call to A C LUA Binding, passing a Vector2D lightuserdata: (nothing new, comparable to previous LUA C binding performance)
- Application_setOrigin * 100,000,000.00 in 1.879s = 53,219,384.30 operation/s [64bits]
- Application_setOrigin * 100,000,000.00 in 1.469s = 68,081,362.24 operation/s [32bits]

Call C function export by DLL passing a pointer on ffi Vector2D struct:
- ffi_c_Application_setOrigin * 100,000,000.00 in 0.122s = 821,501,223.82 operation/s [64bits]
- ffi_c_Application_setOrigin * 100,000,000.00 in 0.121s = 828,446,273.54 operation/s [32bits]
=> This show a reduction of the C/LUA language barrier by a magnitude, comparable to the cost of a non-inlined function call in C++.

# Conclusion
LUA FFI is clearly very interesting to optimize away temporary allocation of vector objects. 

LUA FFI function call overhead to C is ~10 times smaller.

32bits build custom allocator show that FFI struct is not allocated using the allocator (and therefore likely kept on a stack).

Baptiste Lepilleur.

