/*
** $Id: lualib.h $
** Lua standard libraries
** See Copyright Notice in lua.h
*/


#ifndef lualib_h
#define lualib_h

#include "lua.h"


/* version suffix for environment variable names */
#define LUA_VERSUFFIX          "_" LUA_VERSION_MAJOR "_" LUA_VERSION_MINOR

constexpr inline int LUA_GLIBK = 1;
LUAMOD_API int (luaopen_base) (lua_State *L);

#define LUA_LOADLIBNAME	"package"
constexpr inline int LUA_LOADLIBK = LUA_GLIBK <<1;
LUAMOD_API int (luaopen_package) (lua_State *L);


#define LUA_COLIBNAME	"coroutine"
constexpr inline int LUA_COLIBK = LUA_LOADLIBK << 1;
LUAMOD_API int (luaopen_coroutine) (lua_State *L);

#define LUA_DBLIBNAME	"debug"
constexpr inline int LUA_DBLIBK = LUA_COLIBK << 1;
LUAMOD_API int (luaopen_debug) (lua_State *L);

#define LUA_IOLIBNAME	"io"
constexpr inline int LUA_IOLIBK = LUA_DBLIBK << 1;
LUAMOD_API int (luaopen_io) (lua_State *L);

#define LUA_MATHLIBNAME	"math"
constexpr inline int LUA_MATHLIBK = LUA_IOLIBK << 1;
LUAMOD_API int (luaopen_math) (lua_State *L);

#define LUA_OSLIBNAME	"os"
constexpr inline int LUA_OSLIBK = LUA_MATHLIBK << 1;
LUAMOD_API int (luaopen_os) (lua_State *L);

#define LUA_STRLIBNAME	"string"
constexpr inline int LUA_STRLIBK = LUA_OSLIBK << 1;
LUAMOD_API int (luaopen_string) (lua_State *L);

#define LUA_TABLIBNAME	"table"
constexpr inline int LUA_TABLIBK = LUA_STRLIBK << 1;
LUAMOD_API int (luaopen_table) (lua_State *L);

#define LUA_UTF8LIBNAME	"utf8"
constexpr inline int LUA_UTF8LIBK = LUA_TABLIBK << 1;
LUAMOD_API int (luaopen_utf8) (lua_State *L);


/* open selected libraries */
LUALIB_API void (luaL_openselectedlibs) (lua_State *L, int load, int preload);

/* open all libraries */
LUA_INL void luaL_openlibs(lua_State* L) {
    return luaL_openselectedlibs(L, ~0, 0);
}


#endif
