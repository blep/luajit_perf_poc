#include "main.hpp"
#include <lua.hpp>
#include <iostream>
#include <stdint.h>
#include <chrono>

int my_function( lua_State *L )
{
    int argc = lua_gettop( L ); // number of parameters

    std::cerr << "-- my_function() called with " << argc
        << " arguments:" << std::endl;

    for (int n = 1; n <= argc; ++n)
    {
        std::cerr << "-- argument " << n << ": "
            << lua_tostring( L, n ) << std::endl; // parameter n
    }

    lua_pushnumber( L, 123 ); // return value
    return 1; // number of return values
}



struct TypedLightUserObject
{
    static const uint32_t INVALID_TYPE_TAG = 0;
    static const uint32_t STALE_TYPE_TAG = 0xfeca9731;

    /** Tags should always be complex value
    */
    uint32_t typeTag_;

    TypedLightUserObject( uint32_t typeTag )
        : typeTag_( typeTag )
    {
    }
};

struct Vector2DLUO : TypedLightUserObject
{
    static const uint32_t TYPE_TAG = 0x45323789;
    int x_;
    int y_;

    Vector2DLUO() : TypedLightUserObject( TYPE_TAG )
    {
    }
};


int clua_Application_sayHello( lua_State *L )
{
    printf( "Hello World!\n" );
    return 0;
}

static lua_Number getNowTime()
{
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto nowTime = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<lua_Number, std::milli>( nowTime - startTime ).count() / 1000.0f;
}

int clua_Application_now( lua_State *L )
{
    lua_pushnumber( L, getNowTime() );
    return 1;
}


struct Vector2
{
    int x_;
    int y_;
};

static Vector2 appOrigin = {0,0};

int clua_Application_setOrigin( lua_State *L )
{
    int n = lua_gettop( L ); // number of parameters
    if (n != 1)
    {
        printf( "1 parameters required, %d provided", n );
        abort();
    }

    TypedLightUserObject *tluo = static_cast<TypedLightUserObject *>( lua_touserdata( L, 1 ) );
    if (tluo == NULL || tluo->typeTag_ != Vector2DLUO::TYPE_TAG)
    {
        printf( "parameter 1 is not a Vector2D!" );
        abort();
    }

    Vector2DLUO *entity = static_cast<Vector2DLUO *>( tluo );
    appOrigin.x_ = entity->x_;
    appOrigin.y_ = entity->y_;
    return 0;
}

int clua_Application_getOrigin( lua_State *L )
{
    Vector2DLUO *obj = new Vector2DLUO();
    obj->x_ = appOrigin.x_;
    obj->y_ = appOrigin.y_;
    lua_pushlightuserdata( L, obj );
    return 1;
}

extern "C" {

struct TestApi
{
    int( *doPrint1 )( const char *what );
    int( *doPrint2 )( const char *what );
    struct Vector2 (*getOrigin)();
    void (*setOrigin)( struct Vector2 *newOrigin );
};

static int c_TestApi_doPrint1( const char *what )
{
    printf( "1: %s\n", what );
    return 1234;
}

static int c_TestApi_doPrint2( const char *what )
{
    printf( "2: %s\n", what );
    return 1234;
}


LUAJITPOC_API void c_Application_setOrigin( struct Vector2 *newOrigin )
{
    appOrigin.x_ = newOrigin->x_;
    appOrigin.y_ = newOrigin->y_;
}

LUAJITPOC_API struct Vector2 c_Application_getOrigin()
{
    return appOrigin;
}

static TestApi testApi = { &c_TestApi_doPrint1, &c_TestApi_doPrint2, c_Application_getOrigin, c_Application_setOrigin };

LUAJITPOC_API struct TestApi *c_Application_getApi()
{
    return &testApi;
}

}


int tluo_Vector2D_call( lua_State *L )
{
    int n = lua_gettop( L ); // number of parameters
    if (n != 3) // ignore implicit first "table" parameter
    {
        printf( "2 parameters required, %d provided", n-1 );
        abort();
    }

    Vector2DLUO *obj = new Vector2DLUO();
    obj->x_ = static_cast<int>( lua_tointeger(L, 2) );
    obj->y_ = static_cast<int>( lua_tointeger( L, 3) );
    lua_pushlightuserdata( L, obj );
    return 1;
}

int clua_Application_createVector2D( lua_State *L )
{
    Vector2DLUO *obj = new Vector2DLUO();
    obj->x_ = 1;
    obj->y_ = 1;
    lua_pushlightuserdata( L, obj );
    return 1;
}

int clua_Application_destroyVector2D( lua_State *L )
{
    int n = lua_gettop( L ); // number of parameters
    if (n != 1)
    {
        printf( "1 parameters required, %d provided", n );
        abort();
    }

    TypedLightUserObject *tluo = static_cast<TypedLightUserObject *>( lua_touserdata( L, 1 ) );
    if (tluo == NULL || tluo->typeTag_ != Vector2DLUO::TYPE_TAG)
    {
        printf( "parameter 1 is not a Vector2D!" );
        abort();
    }

    Vector2DLUO *entity = static_cast<Vector2DLUO *>( tluo );
    entity->typeTag_ = Vector2DLUO::INVALID_TYPE_TAG;
    delete entity;

    return 0;
}

int tluo_Vector2D_set( lua_State *L )
{
    int n = lua_gettop( L ); // number of parameters
    if ( n < 3 )
    {
        printf( "3 parameters required, %d provided", n );
        abort();
    }

    TypedLightUserObject *tluo = static_cast<TypedLightUserObject *>( lua_touserdata( L, 1 ) );
    if (tluo == NULL  ||  tluo->typeTag_ != Vector2DLUO::TYPE_TAG )
    {
        printf("parameter 1 is not a Vector2D!" );
        abort();
    }

    Vector2DLUO *entity = static_cast<Vector2DLUO *>( tluo );
    entity->x_ = static_cast<int32_t>( lua_tointeger( L, 2 ) );
    entity->y_ = static_cast<int32_t>( lua_tointeger( L, 3 ) );
    return 0;
}

int tluo_Vector2D_x( lua_State *L )
{
    int n = lua_gettop( L ); // number of parameters
    if (n < 1)
    {
        printf( "1 parameters required, %d provided", n );
        abort();
    }

    TypedLightUserObject *tluo = static_cast<TypedLightUserObject *>( lua_touserdata( L, 1 ) );
    if (tluo == NULL || tluo->typeTag_ != Vector2DLUO::TYPE_TAG)
    {
        printf( "parameter 1 is not a Vector2D!" );
        abort();
    }

    Vector2DLUO *entity = static_cast<Vector2DLUO *>( tluo );
    lua_pushinteger( L, entity->x_ );
    return 1;
}

int tluo_Vector2D_y( lua_State *L )
{
    int n = lua_gettop( L ); // number of parameters
    if (n < 1)
    {
        printf( "1 parameters required, %d provided", n );
        abort();
    }

    TypedLightUserObject *tluo = static_cast<TypedLightUserObject *>( lua_touserdata( L, 1 ) );
    if (tluo == NULL || tluo->typeTag_ != Vector2DLUO::TYPE_TAG)
    {
        printf( "parameter 1 is not a Vector2D!" );
        abort();
    }

    Vector2DLUO *entity = static_cast<Vector2DLUO *>( tluo );
    lua_pushinteger( L, entity->y_ );
    return 1;
}


int tluo_meta_add( lua_State *L )
{
    int n = lua_gettop( L ); // number of parameters
    if (n < 2)
    {
        printf( "2 parameters required, %d provided", n );
        abort();
    }

    TypedLightUserObject *tluo1 = static_cast<TypedLightUserObject *>( lua_touserdata( L, 1 ) );
    if (tluo1 == NULL || tluo1->typeTag_ != Vector2DLUO::TYPE_TAG)
    {
        printf( "parameter 1 is not a Vector2D!" );
        abort();
    }
    TypedLightUserObject *tluo2 = static_cast<TypedLightUserObject *>( lua_touserdata( L, 2 ) );
    if (tluo2 == NULL || tluo2->typeTag_ != Vector2DLUO::TYPE_TAG)
    {
        printf( "parameter 1 is not a Vector2D!" );
        abort();
    }

    Vector2DLUO *v1 = static_cast<Vector2DLUO *>( tluo1 );
    Vector2DLUO *v2 = static_cast<Vector2DLUO *>( tluo2 );

    Vector2DLUO *obj = new Vector2DLUO(); // TODO fix leak using ring buffer for temporary objects
    obj->x_ = v1->x_ + v2->x_;
    obj->y_ = v1->y_ + v2->y_;
    lua_pushlightuserdata( L, obj );
    return 1;
}


void report_errors( lua_State *L, int status )
{
    if (status != 0)
    {
        std::cerr << "-- " << lua_tostring( L, -1 ) << std::endl;
        lua_pop( L, 1 ); // remove error message
    }
}

using LuaFn = int (*)( lua_State *L );

static void
push_module_table( lua_State *L, const char *moduleName )
{
    lua_getglobal( L, moduleName );
    if (lua_isnil( L, -1 ))
    {
        lua_pop( L, 1 ); // discard nil

        // Creates new table
        lua_newtable( L );
        lua_setglobal( L, moduleName );
        // Push it on the stack
        lua_getglobal( L, moduleName );
    }
}

static void
set_module_function( lua_State *L, const char *moduleName, const char *functionName, LuaFn fn )
{
    push_module_table( L, moduleName );
    if (!lua_istable( L, -1 ))
    {
        printf( "Failed to make global table %s", moduleName );
        abort();
    }

    lua_pushcfunction( L, fn );
    lua_setfield( L, -2, functionName );
}

static void
set_module_meta_function( lua_State *L, const char *moduleName, const char *functionName, LuaFn fn )
{
    int s1 = lua_gettop(L);
    push_module_table( L, moduleName );
    int s1b = lua_gettop( L );
    if ( lua_getmetatable(L, -1) == 0 ) // module has no meta-table
    {
        int s2 = lua_gettop( L );
        lua_newtable( L );
        lua_setmetatable(L, -2);
        int s3 = lua_gettop( L );
        if ( lua_getmetatable( L, -1 ) == 0 )
        {
            printf( "Failed to set meta table on module" );
            abort();
        }
    }

    lua_pushcfunction( L, fn );
    lua_setfield( L, -2, functionName );
}

//static void 
//set_typed_lightuserdata_function 

static void
set_lightuserdata_metatable( lua_State *L, const char *moduleName )
{
    // Meta-table is used to provide operators on lightuserdata objects.

    lua_pushlightuserdata( L, NULL );
    lua_getfield(L, LUA_REGISTRYINDEX, moduleName );
    lua_setmetatable( L, -2 );
    lua_pop( L, 1 );
}


static void 
set_lightuserdata_meta_function( lua_State *L, const char *moduleName, const char *functionName, LuaFn fn )
{
    lua_getfield( L, LUA_REGISTRYINDEX, moduleName ); // get meta table
    lua_pushcfunction( L, fn );
    lua_setfield( L, -2, functionName );
}

void create_lightuserdata_metatable( lua_State *L, const char *moduleName )
{
    int s1 = lua_gettop( L );
    lua_newtable( L );
    lua_setfield( L, LUA_REGISTRYINDEX, moduleName );
    int s2 = lua_gettop( L );
}

static void 
append_to_lua_path( lua_State *L, const char *modulePath )
{
    static const char *suffixes[] = { "/?.lua", "/?/init.lua" };
    for ( const char *suffix: suffixes )
    {
        lua_getglobal( L, "package" );
        lua_getfield( L, -1, "path" );
        std::string cur_path = lua_tostring( L, -1 );
        cur_path.append( ";" );
        cur_path.append( modulePath );
        cur_path.append( suffix );
        lua_pop( L, 1 );
        lua_pushstring( L, cur_path.c_str() );
        lua_setfield( L, -2, "path" );
        lua_pop( L, 1 ); // get rid of package table from top of stack
    }
}


static void
cppbench_set_vector( Vector2DLUO *v, int x, int y )
{
    v->x_ = x;
    v->y_ = y;
}

static void
cppbench_add_to_vector( Vector2DLUO *v, int x, int y )
{
    v->x_ += x;
    v->y_ += y;
}

typedef void( *BenchCppFn )( Vector2DLUO *v, int x, int y );

static void benchcpp( BenchCppFn func )
{
    lua_Number start = getNowTime();
    Vector2DLUO v;
    auto nbLoop = 1000 * 1000 * 100;
    for ( auto i=0; i < nbLoop; ++i )
    {
        (*func)(&v, i, i);
    }
    lua_Number elapsedSeconds = getNowTime();
    printf( "C++: %d loops in %.3fs = %.1f operation/s\n", nbLoop, elapsedSeconds, nbLoop/elapsedSeconds );
}

#include <unordered_map>
#include <vector>

/// Implementation of the 64 bit MurmurHash2 function
/// http://murmurhash.googlepages.com/
static uint64_t murmur_hash_64( const void * key, uint32_t len, uint64_t seed=0 )
{
    const uint64_t m = 0xc6a4a7935bd1e995ULL;
    const uint32_t r = 47;

    uint64_t h = seed ^ ( len * m );

    const uint64_t * data = (const uint64_t *)key;
    const uint64_t * end = data + ( len / 8 );

    while (data != end)
    {
#ifdef PLATFORM_BIG_ENDIAN
        uint64 k = *data++;
        char *p = (char *)&k;
        char c;
        c = p[ 0 ]; p[ 0 ] = p[ 7 ]; p[ 7 ] = c;
        c = p[ 1 ]; p[ 1 ] = p[ 6 ]; p[ 6 ] = c;
        c = p[ 2 ]; p[ 2 ] = p[ 5 ]; p[ 5 ] = c;
        c = p[ 3 ]; p[ 3 ] = p[ 4 ]; p[ 4 ] = c;
#else
        uint64_t k = *data++;
#endif

        k *= m;
        k ^= k >> r;
        k *= m;

        h ^= k;
        h *= m;
    }

    const unsigned char * data2 = (const unsigned char*)data;

    switch (len & 7)
    {
    case 7: h ^= uint64_t( data2[ 6 ] ) << 48;
    case 6: h ^= uint64_t( data2[ 5 ] ) << 40;
    case 5: h ^= uint64_t( data2[ 4 ] ) << 32;
    case 4: h ^= uint64_t( data2[ 3 ] ) << 24;
    case 3: h ^= uint64_t( data2[ 2 ] ) << 16;
    case 2: h ^= uint64_t( data2[ 1 ] ) << 8;
    case 1: h ^= uint64_t( data2[ 0 ] );
        h *= m;
    };

    h ^= h >> r;
    h *= m;
    h ^= h >> r;

    return h;
}


static int getLuaCallStack( lua_State *L, lua_Debug *stacks, int maxDepth )
{
    int depth = 0;
    while ( depth < maxDepth  &&  lua_getstack( L, depth, &(stacks[depth]) ) )
    {
        // 'S': fills in the fields source, short_src, linedefined, lastlinedefined, and what; 
        // 'l': fills in the field currentline; 
        // 'n': fills in the field name and namewhat; 
        int status = lua_getinfo( L, "Sln", &(stacks[depth]) );
        if ( !status )
            break;
        ++depth;
    }
    return depth;
}

static void luaCallstackToString( std::string &dump, lua_Debug *callstack, int depth )
{
    if (depth == 0)
    {
        dump = "<no callstack info (C initializaiton?)>";
        return;
    }
    char buffer[1024];
    size_t written = 0;
    for ( int index=0; index < depth; ++index )
    {
        lua_Debug &sourceLoc = callstack[index];
        int len = snprintf( buffer + written, sizeof(buffer) - written, "%s(%d): %s\n", 
                  sourceLoc.short_src, sourceLoc.currentline, sourceLoc.name ? sourceLoc.name : "?" );
        if ( len >= 0 )
        {
            written += len;
        }
        else // buffer too small
        {
            break;
        }
    }
    buffer[sizeof(buffer)-1] = 0;
    dump.assign(buffer);
}

template <typename T> int normalizedCompare( T val )
{
    return ( T( 0 ) < val ) - ( val < T( 0 ) );
}

struct CustomAllocState
{
    CustomAllocState()
        : allocatedBytes_( 0 )
        , maxAllocatedBytes_( 0 )
        , totalAllocCall_( 0 )
        , captureCallstacks_( false )
        , L_( NULL )
    {
    }

    void captureCallstack( int64_t allocatedSize )
    {
        const int stackDepth = 5;
        lua_Debug callstack[ stackDepth ] = { 0 };
        int depth = getLuaCallStack( L_, callstack, sizeof(callstack )/sizeof( callstack[0] ) );
        // Lua string are internalized (one pointer for all strings with same value), 
        // therefore taking the binary hash of struct containing string pointer and line number work.
        uint64_t hash = murmur_hash_64( &callstack[ 0 ], sizeof( lua_Debug )*depth );
        AllocData & data = allocsByCallstack_[hash];
        data.callCount_ += 1;
        data.allocatedBytes_ += allocatedSize;
        if ( data.callStack_.empty() )
        {
            luaCallstackToString( data.callStack_, callstack, depth );
        }
    }

    enum SortOrder
    {
        bySize = 0,
        byCallCount
    };

    void printCallstackReport( FILE *fout, SortOrder sortOrder, std::size_t firstN=0 )
    {
        std::vector<AllocData *> sites;
        sites.reserve( allocsByCallstack_.size() );
        for ( auto &entry: allocsByCallstack_ )
        {
            sites.push_back( &(entry.second) );
        }

        int( *compareAllocDataBy )( const void *a, const void *b ) = &compareAllocDataBySize;
        const char *order = "by size (in bytes)";
        if ( sortOrder != bySize )
        {
            order = "by call count";
            compareAllocDataBy = &compareAllocDataByCount;
        }
        qsort(sites.data(), sites.size(), sizeof(sites[0]), compareAllocDataBy );

        fprintf( fout, "Memory allocation stack sorted by %s:\n", order );
        std::size_t limit = firstN == 0 ? sites.size() : firstN;
        for ( std::size_t index=0; index < limit; ++index )
        {
            AllocData &data = *( sites[index] );
            fprintf( fout, "* %lld bytes for %lld calls: %s", data.allocatedBytes_, data.callCount_, data.callStack_.c_str() );
        }
    }

    static int compareAllocDataBySize( const void *a, const void *b )
    {
        const CustomAllocState::AllocData *pa = *static_cast<CustomAllocState::AllocData *const*>( a );
        const CustomAllocState::AllocData *pb = *static_cast<CustomAllocState::AllocData *const*>( b );
        return -normalizedCompare( pa->allocatedBytes_ - pb->allocatedBytes_ );
    }

    static int compareAllocDataByCount( const void *a, const void *b )
    {
        const CustomAllocState::AllocData *pa = *static_cast<CustomAllocState::AllocData *const*>( a );
        const CustomAllocState::AllocData *pb = *static_cast<CustomAllocState::AllocData *const*>( b );
        return -normalizedCompare( pa->callCount_ - pb->callCount_ );
    }

    void clear()
    {
        // reseting everything messup stats as free occurs...
        //allocatedBytes_ = 0;
        //maxAllocatedBytes_ = 0;
        //totalAllocCall_ = 0;
        allocsByCallstack_.clear();
    }

    int64_t allocatedBytes_;
    int64_t maxAllocatedBytes_;
    int64_t totalAllocCall_;
    lua_State *L_;

    struct AllocData
    {
        std::string callStack_;
        int64_t callCount_;
        int64_t allocatedBytes_;
    };

    bool captureCallstacks_;

    typedef std::unordered_map<uint64_t, AllocData> AllocsByCallstack;
    AllocsByCallstack allocsByCallstack_;
};

CustomAllocState customAllocState;
#include <assert.h>
static void *
custom_lua_alloc( void *userData, void *ptr, size_t osize, size_t nsize )
{
    CustomAllocState *state = static_cast<CustomAllocState *>( userData );
    state->allocatedBytes_ += -static_cast<int64_t>(osize) + static_cast<int64_t>( nsize );
    if (state->allocatedBytes_ > state->maxAllocatedBytes_ )
        state->maxAllocatedBytes_ = state->allocatedBytes_;
    if ( nsize == 0 )
    {
        free( ptr );
        return NULL;
    }
    else
    {
        // Remarks: nsize may be < osize.
        state->totalAllocCall_ += 1;
        if (customAllocState.captureCallstacks_)
            customAllocState.captureCallstack( static_cast<int64_t>(nsize) - static_cast<int64_t>(osize) );
        return realloc( ptr, nsize );
    }
}

static int panic( lua_State*L )
{
    fprintf( stderr, "PANIC: unprotected error in call to Lua API (%s)\n",
             lua_tostring( L, -1 ) );
    return 0;
}



int lib_main( int argc, char** argv )
{
    BenchCppFn benchCppFn = argc > 1 ? cppbench_set_vector : cppbench_add_to_vector;
    benchcpp( benchCppFn );

#if !defined(NDEBUG) &&  !defined(_WIN64) /* debug & 32bits only */
# define LUAPOC_USE_CUSTOM_ALLOC 1
#else
# undef LUAPOC_USE_CUSTOM_ALLOC
#endif
#if defined(LUAPOC_USE_CUSTOM_ALLOC)
        // Doesn't work with LUA JIT 64 bits:
        // See: https://gist.github.com/nddrylliog/8722197
        lua_State *L = lua_newstate( custom_lua_alloc, &customAllocState );
        if (L != NULL)
        {
            lua_atpanic( L, &panic );
        }
#else
        lua_State *L = lua_open();
#endif
#if !defined(NDEBUG)
        customAllocState.captureCallstacks_ = true;
        customAllocState.L_ = L;
#endif
    //    lua_pushlightuserdata( L, (void *)wrap_exceptions );
/*        static const luaL_Reg lj_lib_load[] = {
            { "",			luaopen_base },
            { LUA_LOADLIBNAME,	luaopen_package },
            { LUA_TABLIBNAME,	luaopen_table },
            { LUA_IOLIBNAME,	luaopen_io },
            { LUA_OSLIBNAME,	luaopen_os },
            { LUA_STRLIBNAME,	luaopen_string },
            { LUA_MATHLIBNAME,	luaopen_math },
            { LUA_DBLIBNAME,	luaopen_debug },
            { LUA_BITLIBNAME,	luaopen_bit },
            { LUA_JITLIBNAME,	luaopen_jit },
            { NULL,		NULL }
        };*/
        luaL_openlibs( L );
        //luaopen_io( L ); // provides io.*
        //luaopen_base( L );
        //luaopen_table( L );
        //luaopen_string( L );
        //luaopen_math( L );
        //luaopen_ffi( L );
        //luaopen_debug( L );
        //luaopen_jit( L );
        //    luaopen_loadlib( L );

        append_to_lua_path( L, LUAJIT_POC_DATA_DIR );

        // make my_function() available to Lua programs
        lua_register( L, "my_function", my_function );

        set_module_function( L, "Application", "sayHello", &clua_Application_sayHello );
        set_module_function( L, "Application", "now", &clua_Application_now );
        set_module_function( L, "Application", "getOrigin", &clua_Application_getOrigin );
        set_module_function( L, "Application", "setOrigin", &clua_Application_setOrigin );
        set_module_function( L, "Application", "createVector2D", &clua_Application_createVector2D );
        set_module_function( L, "Application", "destroyVector2D", &clua_Application_destroyVector2D );
        set_module_function( L, "Application", "setOrigine", &clua_Application_setOrigin );
        // Makes Vector2D callable, allowing new instance to be created using "Vector2D(x,y)"
        // Notes this means that on the C++ side there is no "parent" object
        set_module_meta_function( L, "Vector2D", "__call", &tluo_Vector2D_call );
        set_module_function( L, "Vector2D", "x", &tluo_Vector2D_x );
        set_module_function( L, "Vector2D", "y", &tluo_Vector2D_y );
        set_module_function( L, "Vector2D", "set", &tluo_Vector2D_set);
        create_lightuserdata_metatable(L, "demo");
        set_lightuserdata_meta_function( L, "demo", "__add", &tluo_meta_add );
        set_lightuserdata_metatable( L, "demo" );

        customAllocState.clear();

        std::string filePath;
        if ( argc < 2 )
            filePath = std::string(LUAJIT_POC_DATA_DIR) + "/bench.lua";
        else
            filePath = argv[1];
        printf( "Loading %s\n", filePath.c_str() );
        int s = luaL_loadfile( L, filePath.c_str() );

        if (s == 0)
        {
            // execute Lua program
            s = lua_pcall( L, 0, LUA_MULTRET, 0 );
            if ( s != 0 )
            {
                printf("Error while executing file: %s", lua_tostring(L, -1));
            }
        }

        // Force a full GC
        lua_gc( L, LUA_GCCOLLECT, 0 );

        lua_close( L );
#if defined(LUAPOC_USE_CUSTOM_ALLOC)
        printf( "\nCustom Alloc Report:\n" );
        printf( "Bytes currently allocated on heap: 0x%llx bytes, max 0x%llx bytes\n", customAllocState.allocatedBytes_, customAllocState.maxAllocatedBytes_ );
        printf( "#calls: alloc: %lld\n", customAllocState.totalAllocCall_ );
        customAllocState.printCallstackReport(stdout, CustomAllocState::bySize, 10);
#endif
    return 0;
}
