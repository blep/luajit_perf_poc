-- C++, forced function call: 0.119s
-- Vector2D.set * 100000000 in 2.557s
-- Vector2D_set * 100000000 in 1.815s
-- setLuaV2 (ffi struct) * 100000000 in 0.048s  (function call is likely inlined)

local inspect = require("inspect")
print( "Application: ", inspect(Application), "\n" )
print( "Vector2D: ", inspect(Vector2D), "\n" )
print( "require: ", inspect(require), "\n" )
print( "ffi: ", inspect(ffi), "\n" )
-- print( "_G: ", inspect(_G), "\n" )
print( "package.path: ", inspect(package.path), "\n" )

io.write("Running ", _VERSION, "\n")
a = my_function(1, 2, 3, "abc", "def")
io.write("my_function() returned ", a, "\n")

Application.sayHello()
v1 = Application.createVector2D()

io.write( "Vector2D.x(v1) = ", Vector2D.x(v1), "\n" )
io.write( "Vector2D.y(v1) = ", Vector2D.y(v1), "\n" )
Vector2D.set(v1, 12, 21 )
io.write( "Vector2D.x(v1) = ", Vector2D.x(v1), "\n" )
io.write( "Vector2D.y(v1) = ", Vector2D.y(v1), "\n" )

v2 = Vector2D(7, 9)
io.write( "Vector2D.x(v2) = ", Vector2D.x(v2), "\n" )
io.write( "Vector2D.y(v2) = ", Vector2D.y(v2), "\n" )
v3 = v1 + v2
io.write( "Vector2D.x(v3) = ", Vector2D.x(v3), "\n" )
io.write( "Vector2D.y(v3) = ", Vector2D.y(v3), "\n" )

Application.destroyVector2D(v1)

function format_thousand_sep(v)
	local s = string.format("%d", math.floor(v))
	local pos = string.len(s) % 3
	if pos == 0 then pos = 3 end
	return string.sub(s, 1, pos)
		.. string.gsub(string.sub(s, pos+1), "(...)", ",%1")
		.. string.sub(string.format("%.2f", v - math.floor(v)), 2)
end


function timeIt( func, ... )
  start = Application.now()
  what, nbLoop = func(...)
  elapsed = Application.now() - start
  
  startGC = Application.now()
  collectgarbage()
  elapsedGC = Application.now() - startGC
  print( string.format("%s * %s in %.3fs = %s operation/s; post GC time = %fs", what, format_thousand_sep(nbLoop), elapsed, format_thousand_sep(nbLoop/elapsed), elapsedGC ) )
end

local nbSetLoop = 1000*1000*100

function bench_Vector2D_dot_set()
  local v1 = Application.createVector2D()
  for i=1,nbSetLoop do
    Vector2D.set(v1, i, i )
  end
  io.write( "Vector2D.x(v1) = ", Vector2D.x(v1), "\n" )
  io.write( "Vector2D.y(v1) = ", Vector2D.y(v1), "\n" )
  return "Vector2D.set", nbSetLoop
end

function bench_Vector2D_set()
  local v1 = Application.createVector2D()
  local Vector2D_set = Vector2D.set
  for i=1,nbSetLoop do
    Vector2D_set(v1, i, i )
  end
  io.write( "Vector2D.x(v1) = ", Vector2D.x(v1), "\n" )
  io.write( "Vector2D.y(v1) = ", Vector2D.y(v1), "\n" )
  return "Vector2D_set", nbSetLoop
end


-- FFI metatype can be used to avoid the function call and allocation
-- See http://wiki.luajit.org/Allocation-Sinking-Optimization
local ffi = require("ffi")
ffi.cdef( [[
typedef struct CVector2
{
    int x;
    int y;
} CVector2;

void c_Application_setOrigin( struct CVector2 *newOrigin );
struct CVector2 c_Application_getOrigin();

  ]] )
  

local LuaV2 = ffi.typeof ('CVector2')

LuaV2 = ffi.metatype(LuaV2, {
  __add = function(a, b)
   return LuaV2(a.x + b.x, a.y + b.y)
  end
})

function setLuaV2(v, x, y)
  v.x = x
  v.y = y
end

local poclib = ffi.load( "luajit_poc" )
poclib.c_Application_setOrigin( LuaV2(7,13) )


function bench_setLuaV2FFI()
  local v1 = LuaV2(15, 12)
  local Vector2D_set = Vector2D.set
  for i=1,nbSetLoop do
    setLuaV2( v1, i, i )
  end
  io.write( "v1.x = ", v1.x, "\n" )
  io.write( "v1.y = ", v1.y, "\n" )
  return "setLuaV2 (ffi struct)", nbSetLoop
end

local v1 = LuaV2(15, 12)
setLuaV2( v1, 7, 9 )
io.write( "v1.x = ", v1.x, "\n" )
io.write( "v1.y = ", v1.y, "\n" )
-- Application.printFFIV2( v1 )

local nbCreateLoop = 1000*1000*10

function bench_createSumDestroyVector2D()
  local create = Application.createVector2D
  local destroy = Application.destroyVector2D
  local x = Vector2D.x
  local y = Vector2D.y
  local sum = create()
  for i=1,nbCreateLoop do
    local v1 = create()
    sum = sum + v1
    destroy( v1 )
  end
  io.write( string.format("sum: x=%d, y=%d\n", Vector2D.x(sum), Vector2D.y(sum)) )
  return "Application.create/sum/DestroyVector2D", nbCreateLoop
end

function bench_LuaV2CreateSumDestroy()
  local sum = LuaV2(1,1)
  for i=1,nbCreateLoop do
    local v1 = LuaV2(1,1)
    sum = sum + v1
  end
  io.write( string.format("sum: x=%d, y=%d\n", sum.x, sum.y) )
  return "LuaV2Create/sum/Destroy (ffi struct)", nbCreateLoop
end

local nbSetOriginLoop = 1000*1000*100

function bench_Application_setOrigin()
  local v1 = Application.createVector2D()
  Vector2D.set(v1, 7, 13 )
  local Application_setOrigin = Application.setOrigin
  for i=1,nbSetOriginLoop do
    Application_setOrigin(v1)
  end
  local origin = Application.getOrigin()
  io.write( "Application.getOrigin().x = ", Vector2D.x(origin), "\n" )
  io.write( "Application.getOrigin().y = ", Vector2D.y(origin), "\n" )
  return "Application_setOrigin", nbSetOriginLoop
end

function bench_ffi_Application_setOrigin()
  local v1 = LuaV2(7,13)
  for i=1,nbSetOriginLoop do
    poclib.c_Application_setOrigin( v1 )
  end
  local origin = Application.getOrigin()
  io.write( "Application.getOrigin().x = ", Vector2D.x(origin), "\n" )
  io.write( "Application.getOrigin().y = ", Vector2D.y(origin), "\n" )
  return "ffi_c_Application_setOrigin", nbSetOriginLoop
end

timeIt( bench_Vector2D_dot_set )
timeIt( bench_Vector2D_set )
timeIt( bench_setLuaV2FFI )

-- local jit_dump = require("dump")
-- jit_dump.dump()

timeIt( bench_createSumDestroyVector2D )
timeIt( bench_LuaV2CreateSumDestroy )

timeIt( bench_Application_setOrigin )
timeIt( bench_ffi_Application_setOrigin )
