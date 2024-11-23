/*
** $Id: lua.h $
** Lua - A Scripting Language
** Lua.org, PUC-Rio, Brazil (www.lua.org)
** See Copyright Notice at the end of this file
*/


#ifndef lua_h
#define lua_h

#include <stdarg.h>
#include <stddef.h>

#define LUA_INL inline


#define LUA_COPYRIGHT	LUA_RELEASE "  Copyright (C) 1994-2024 Lua.org, PUC-Rio"
#define LUA_AUTHORS	"R. Ierusalimschy, L. H. de Figueiredo, W. Celes"


constexpr inline int LUA_VERSION_MAJOR_N	 = 5;
constexpr inline int LUA_VERSION_MINOR_N	 = 5;
constexpr inline int LUA_VERSION_RELEASE_N   = 0;

constexpr inline int LUA_VERSION_NUM         = LUA_VERSION_MAJOR_N * 100 + LUA_VERSION_MINOR_N;
constexpr inline int LUA_VERSION_RELEASE_NUM = LUA_VERSION_NUM * 100 + LUA_VERSION_RELEASE_N;


#include "luaconf.h"


/* mark for precompiled code ('<esc>Lua') */
#define LUA_SIGNATURE	"\x1bLua"

/* option for multiple returns in 'lua_pcall' and 'lua_call' */
constexpr inline int LUA_MULTRET = -1;


/*
** Pseudo-indices
** (-LUAI_MAXSTACK is the minimum valid index; we keep some free empty
** space after that to help overflow detection)
*/
constexpr inline int LUA_REGISTRYINDEX = -LUAI_MAXSTACK - 1000;
#define lua_upvalueindex(i)	(LUA_REGISTRYINDEX - (i))


/* thread status */
constexpr inline int LUA_OK			= 0;
constexpr inline int LUA_YIELD		= 1;
constexpr inline int LUA_ERRRUN		= 2;
constexpr inline int LUA_ERRSYNTAX	= 3;
constexpr inline int LUA_ERRMEM		= 4;
constexpr inline int LUA_ERRERR     = 5;


using lua_State = struct lua_State;
//^^^ makes it so you dont need to define "lua_State" yet (forward declaration)


/*
** basic types
*/
constexpr inline int LUA_TNONE          = -1;

constexpr inline int LUA_TNIL			= 0;
constexpr inline int LUA_TBOOLEAN		= 1;
constexpr inline int LUA_TLIGHTUSERDATA	= 2;
constexpr inline int LUA_TNUMBER		= 3;
constexpr inline int LUA_TSTRING		= 4;
constexpr inline int LUA_TTABLE			= 5;
constexpr inline int LUA_TFUNCTION		= 6;
constexpr inline int LUA_TUSERDATA		= 7;
constexpr inline int LUA_TTHREAD        = 8;

constexpr inline int LUA_NUMTYPES       = 9;



/* minimum Lua stack available to a C function */
constexpr inline int LUA_MINSTACK = 20;


/* predefined values in the registry */
/* index 1 is reserved for the reference mechanism */
constexpr inline int LUA_RIDX_GLOBALS       = 2;
constexpr inline int LUA_RIDX_MAINTHREAD    = 3;
constexpr inline int LUA_RIDX_LAST          = 3;


/* type of numbers in Lua */
using lua_Number = LUA_NUMBER;


/* type for integer functions */
using lua_Integer = LUA_INTEGER;

/* unsigned integer type */
using lua_Unsigned = LUA_UNSIGNED;

/* type for continuation-function contexts */
using lua_KContext = LUA_KCONTEXT;


/*
** Type for C functions registered with Lua
*/
using lua_CFunction = int (*) (lua_State *L);

/*
** Type for continuation functions
*/
using lua_KFunction = int (*) (lua_State *L, int status, lua_KContext ctx);


/*
** Type for functions that read/write blocks when loading/dumping Lua chunks
*/
using lua_Reader =  const char * (*) (lua_State *L, void *ud, size_t *sz);

using lua_Writer = int (*) (lua_State *L, const void *p, size_t sz, void *ud);


/*
** Type for memory-allocation functions
*/
using lua_Alloc = void * (*) (void *ud, void *ptr, size_t osize, size_t nsize);


/*
** Type for warning functions
*/
using lua_WarnFunction = void (*) (void *ud, const char *msg, int tocont);


/*
** Type used by the debug API to collect debug information
*/
using lua_Debug = struct lua_Debug;
//^^^ makes it so you dont need to define "lua_Debug" yet (forward declaration)


/*
** Functions to be called by the debugger in specific events
*/
using lua_Hook = void (*) (lua_State *L, lua_Debug *ar);


/*
** generic extra include file
*/
#if defined(LUA_USER_H)
#include LUA_USER_H
#endif


/*
** RCS ident string
*/
extern const char lua_ident[];


/*
** state manipulation
*/
LUA_API lua_State *(lua_newstate) (lua_Alloc f, void *ud, unsigned seed);
LUA_API void       (lua_close) (lua_State *L);
LUA_API lua_State *(lua_newthread) (lua_State *L);
LUA_API int        (lua_closethread) (lua_State *L, lua_State *from);

LUA_API lua_CFunction (lua_atpanic) (lua_State *L, lua_CFunction panicf);


LUA_API lua_Number (lua_version) (lua_State *L);


/*
** basic stack manipulation
*/
LUA_API int   (lua_absindex) (lua_State *L, int idx);
LUA_API int   (lua_gettop) (lua_State *L);
LUA_API void  (lua_settop) (lua_State *L, int idx);
LUA_API void  (lua_pushvalue) (lua_State *L, int idx);
LUA_API void  (lua_rotate) (lua_State *L, int idx, int n);
LUA_API void  (lua_copy) (lua_State *L, int fromidx, int toidx);
LUA_API int   (lua_checkstack) (lua_State *L, int n);

LUA_API void  (lua_xmove) (lua_State *from, lua_State *to, int n);


/*
** access functions (stack -> C)
*/

LUA_API int             (lua_isnumber) (lua_State *L, int idx);
LUA_API int             (lua_isstring) (lua_State *L, int idx);
LUA_API int             (lua_iscfunction) (lua_State *L, int idx);
LUA_API int             (lua_isinteger) (lua_State *L, int idx);
LUA_API int             (lua_isuserdata) (lua_State *L, int idx);
LUA_API int             (lua_type) (lua_State *L, int idx);
LUA_API const char     *(lua_typename) (lua_State *L, int tp);

LUA_API lua_Number      (lua_tonumberx) (lua_State *L, int idx, int *isnum);
LUA_API lua_Integer     (lua_tointegerx) (lua_State *L, int idx, int *isnum);
LUA_API int             (lua_toboolean) (lua_State *L, int idx);
LUA_API const char     *(lua_tolstring) (lua_State *L, int idx, size_t *len);
LUA_API lua_Unsigned    (lua_rawlen) (lua_State *L, int idx);
LUA_API lua_CFunction   (lua_tocfunction) (lua_State *L, int idx);
LUA_API void	       *(lua_touserdata) (lua_State *L, int idx);
LUA_API lua_State      *(lua_tothread) (lua_State *L, int idx);
LUA_API const void     *(lua_topointer) (lua_State *L, int idx);


/*
** Comparison and arithmetic functions
*/

constexpr inline int LUA_OPADD	= 0; /* ORDER TM, ORDER OP */
constexpr inline int LUA_OPSUB	= 1;
constexpr inline int LUA_OPMUL	= 2;
constexpr inline int LUA_OPMOD	= 3;
constexpr inline int LUA_OPPOW	= 4;
constexpr inline int LUA_OPDIV	= 5;
constexpr inline int LUA_OPIDIV	= 6;
constexpr inline int LUA_OPBAND	= 7;
constexpr inline int LUA_OPBOR	= 8;
constexpr inline int LUA_OPBXOR	= 9;
constexpr inline int LUA_OPSHL	= 10;
constexpr inline int LUA_OPSHR	= 11;
constexpr inline int LUA_OPUNM	= 12;
constexpr inline int LUA_OPBNOT = 13;

LUA_API void  (lua_arith) (lua_State *L, int op);

constexpr inline int LUA_OPEQ	= 0;
constexpr inline int LUA_OPLT	= 1;
constexpr inline int LUA_OPLE   = 2;

LUA_API int   (lua_rawequal) (lua_State *L, int idx1, int idx2);
LUA_API int   (lua_compare) (lua_State *L, int idx1, int idx2, int op);


/*
** push functions (C -> stack)
*/
LUA_API void        (lua_pushnil) (lua_State *L);
LUA_API void        (lua_pushnumber) (lua_State *L, lua_Number n);
LUA_API void        (lua_pushinteger) (lua_State *L, lua_Integer n);
LUA_API const char *(lua_pushlstring) (lua_State *L, const char *s, size_t len);
LUA_API const char *(lua_pushextlstring) (lua_State *L,
		const char *s, size_t len, lua_Alloc falloc, void *ud);
LUA_API const char *(lua_pushstring) (lua_State *L, const char *s);
LUA_API const char *(lua_pushvfstring) (lua_State *L, const char *fmt,
                                                      va_list argp);
LUA_API const char *(lua_pushfstring) (lua_State *L, const char *fmt, ...);
LUA_API void  (lua_pushcclosure) (lua_State *L, lua_CFunction fn, int n);
LUA_API void  (lua_pushboolean) (lua_State *L, int b);
LUA_API void  (lua_pushlightuserdata) (lua_State *L, void *p);
LUA_API int   (lua_pushthread) (lua_State *L);


/*
** get functions (Lua -> stack)
*/
LUA_API int (lua_getglobal) (lua_State *L, const char *name);
LUA_API int (lua_gettable) (lua_State *L, int idx);
LUA_API int (lua_getfield) (lua_State *L, int idx, const char *k);
LUA_API int (lua_geti) (lua_State *L, int idx, lua_Integer n);
LUA_API int (lua_rawget) (lua_State *L, int idx);
LUA_API int (lua_rawgeti) (lua_State *L, int idx, lua_Integer n);
LUA_API int (lua_rawgetp) (lua_State *L, int idx, const void *p);

LUA_API void  (lua_createtable) (lua_State *L, unsigned narr, unsigned nrec);
LUA_API void *(lua_newuserdatauv) (lua_State *L, size_t sz, int nuvalue);
LUA_API int   (lua_getmetatable) (lua_State *L, int objindex);
LUA_API int  (lua_getiuservalue) (lua_State *L, int idx, int n);


/*
** set functions (stack -> Lua)
*/
LUA_API void  (lua_setglobal) (lua_State *L, const char *name);
LUA_API void  (lua_settable) (lua_State *L, int idx);
LUA_API void  (lua_setfield) (lua_State *L, int idx, const char *k);
LUA_API void  (lua_seti) (lua_State *L, int idx, lua_Integer n);
LUA_API void  (lua_rawset) (lua_State *L, int idx);
LUA_API void  (lua_rawseti) (lua_State *L, int idx, lua_Integer n);
LUA_API void  (lua_rawsetp) (lua_State *L, int idx, const void *p);
LUA_API int   (lua_setmetatable) (lua_State *L, int objindex);
LUA_API int   (lua_setiuservalue) (lua_State *L, int idx, int n);


/*
** 'load' and 'call' functions (load and run Lua code)
*/
LUA_API void  (lua_callk) (lua_State *L, int nargs, int nresults,
                           lua_KContext ctx, lua_KFunction k);

LUA_INL void lua_call(lua_State* L, int nargs, int nresults) { 
    lua_callk(L, nargs, nresults, 0, NULL);
}


LUA_API int   (lua_pcallk) (lua_State *L, int nargs, int nresults, int errfunc,
                            lua_KContext ctx, lua_KFunction k);

LUA_INL int lua_pcall(lua_State* L, int nargs, int nresults, int errfunc) {
    return lua_pcallk(L, nargs, nresults, errfunc, 0, NULL); 
}


LUA_API int   (lua_load) (lua_State *L, lua_Reader reader, void *dt,
                          const char *chunkname, const char *mode);

LUA_API int (lua_dump)   (lua_State *L, lua_Writer writer, void *data, int strip);


/*
** coroutine functions
*/
LUA_API int  (lua_yieldk)     (lua_State *L, int nresults, lua_KContext ctx,
                               lua_KFunction k);
LUA_API int  (lua_resume)     (lua_State *L, lua_State *from, int narg,
                               int *nres);
LUA_API int  (lua_status)     (lua_State *L);
LUA_API int (lua_isyieldable) (lua_State *L);

LUA_INL int lua_yield(lua_State* L,  int nresults) {
    return lua_yieldk(L, nresults, 0, NULL);
}


/*
** Warning-related functions
*/
LUA_API void (lua_setwarnf) (lua_State *L, lua_WarnFunction f, void *ud);
LUA_API void (lua_warning)  (lua_State *L, const char *msg, int tocont);


/*
** garbage-collection options
*/

constexpr inline int LUA_GCSTOP		 = 0;
constexpr inline int LUA_GCRESTART	 = 1;
constexpr inline int LUA_GCCOLLECT	 = 2;
constexpr inline int LUA_GCCOUNT	 = 3;
constexpr inline int LUA_GCCOUNTB	 = 4;
constexpr inline int LUA_GCSTEP		 = 5;
constexpr inline int LUA_GCISRUNNING = 6;
constexpr inline int LUA_GCGEN		 = 7;
constexpr inline int LUA_GCINC		 = 8;
constexpr inline int LUA_GCPARAM     = 9;


/*
** garbage-collection parameters
*/
/* parameters for generational mode */
constexpr inline int LUA_GCPMINORMUL	= 0;  /* control minor collections */
constexpr inline int LUA_GCPMAJORMINOR 	= 1;  /* control shift major->minor */
constexpr inline int LUA_GCPMINORMAJOR	= 2;  /* control shift minor->major */

/* parameters for incremental mode */
constexpr inline int LUA_GCPPAUSE		= 3;  /* size of pause between successive GCs */
constexpr inline int LUA_GCPSTEPMUL		= 4;  /* GC "speed" */
constexpr inline int LUA_GCPSTEPSIZE	= 5;  /* GC granularity */

/* number of parameters */
constexpr inline int LUA_GCPN = 6;


LUA_API int (lua_gc) (lua_State *L, int what, ...);


/*
** miscellaneous functions
*/

LUA_API int   (lua_error) (lua_State *L);

LUA_API int   (lua_next) (lua_State *L, int idx);

LUA_API void  (lua_concat) (lua_State *L, int n);
LUA_API void  (lua_len)    (lua_State *L, int idx);

constexpr inline int LUA_N2SBUFFSZ = 64;
LUA_API unsigned  (lua_numbertostrbuff) (lua_State *L, int idx, char *buff);
LUA_API size_t  (lua_stringtonumber) (lua_State *L, const char *s);

LUA_API lua_Alloc (lua_getallocf) (lua_State *L, void **ud);
LUA_API void      (lua_setallocf) (lua_State *L, lua_Alloc f, void *ud);

LUA_API void (lua_toclose) (lua_State *L, int idx);
LUA_API void (lua_closeslot) (lua_State *L, int idx);


/*
** {==============================================================
** some useful macros
** ===============================================================
*/

#define lua_getextraspace(L)	((void *)((char *)(L) - LUA_EXTRASPACE))

LUA_INL lua_Number lua_tonumber(lua_State* L, int idx) {
    return lua_tonumberx(L, idx, nullptr);
}
LUA_INL lua_Integer lua_tointeger(lua_State* L, int idx) {
    return lua_tointegerx(L, idx, nullptr);
}

LUA_INL void lua_pop(lua_State* L, int count) {
    lua_settop(L, -count-1);
}

LUA_INL void lua_newtable(lua_State* L) {
    lua_createtable(L, 0,0);
}

LUA_INL void lua_pushcfunction(lua_State* L,lua_CFunction fn) {
    lua_pushcclosure(L, fn, 0);
}
LUA_INL void lua_register(lua_State* L, const char* name, lua_CFunction fn) {
    lua_pushcfunction(L, fn); lua_setglobal(L, name);
}

LUA_INL bool lua_isfunction(lua_State* L, int idx) {
    return lua_type(L, idx) == LUA_TFUNCTION;
}
LUA_INL bool lua_istable(lua_State* L, int idx) {
    return lua_type(L, idx) == LUA_TTABLE;
}
LUA_INL bool lua_islightuserdata(lua_State* L, int idx) {
    return lua_type(L, idx) == LUA_TLIGHTUSERDATA;
}
LUA_INL bool lua_isnil(lua_State* L, int idx) {
    return lua_type(L, idx) == LUA_TNIL;
}
LUA_INL bool lua_isboolean(lua_State* L, int idx) {
    return lua_type(L, idx) == LUA_TBOOLEAN;
}
LUA_INL bool lua_isthread(lua_State* L, int idx) {
    return lua_type(L, idx) == LUA_TTHREAD;
}
LUA_INL bool lua_isnone(lua_State* L, int idx) {
    return lua_type(L, idx) == LUA_TNONE;
}
LUA_INL bool lua_isnoneornil(lua_State* L, int idx) {
    return lua_type(L, idx) <= 0;
}

#define lua_pushliteral(L, s)	lua_pushstring(L, "" s)

LUA_INL void lua_pushglobaltable(lua_State* L) {
    ((void)lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS));
}

LUA_INL const char* lua_tostring(lua_State* L, int idx) {
    return lua_tolstring(L, idx, nullptr);
}

LUA_INL void lua_insert(lua_State* L, int idx) {
    lua_rotate(L, idx, 1);
}
LUA_INL void lua_remove(lua_State* L, int idx) {
    lua_rotate(L, idx, -1); lua_pop(L, 1);
}
LUA_INL void lua_replace(lua_State* L, int idx) {
    lua_copy(L, -1, idx); lua_pop(L, 1);
}

/* }============================================================== */


/*
** {==============================================================
** compatibility macros
** ===============================================================
*/
#if defined(LUA_COMPAT_APIINTCASTS)

#define lua_pushunsigned(L,n)	lua_pushinteger(L, (lua_Integer)(n))
#define lua_tounsignedx(L,i,is)	((lua_Unsigned)lua_tointegerx(L,i,is))
#define lua_tounsigned(L,i)	    lua_tounsignedx(L,(i),NULL)

#endif

#define lua_newuserdata(L,s)	lua_newuserdatauv(L,s,1)
#define lua_getuservalue(L,idx)	lua_getiuservalue(L,idx,1)
#define lua_setuservalue(L,idx)	lua_setiuservalue(L,idx,1)

#define lua_resetthread(L)	lua_closethread(L,NULL)

/* }============================================================== */

/*
** {======================================================================
** Debug API
** =======================================================================
*/


/*
** Event codes
*/
constexpr inline int LUA_HOOKCALL     = 0;
constexpr inline int LUA_HOOKRET      = 1;
constexpr inline int LUA_HOOKLINE     = 2;
constexpr inline int LUA_HOOKCOUNT    = 3;
constexpr inline int LUA_HOOKTAILCALL = 4;


/*
** Event masks
*/
constexpr inline int LUA_MASKCALL     = 1 << LUA_HOOKCALL;
constexpr inline int LUA_MASKRET      = 1 << LUA_HOOKRET;
constexpr inline int LUA_MASKLINE     = 1 << LUA_HOOKLINE;
constexpr inline int LUA_MASKCOUNT    = 1 << LUA_HOOKCOUNT;


LUA_API int (lua_getstack) (lua_State *L, int level, lua_Debug *ar);
LUA_API int (lua_getinfo) (lua_State *L, const char *what, lua_Debug *ar);
LUA_API const char *(lua_getlocal) (lua_State *L, const lua_Debug *ar, int n);
LUA_API const char *(lua_setlocal) (lua_State *L, const lua_Debug *ar, int n);
LUA_API const char *(lua_getupvalue) (lua_State *L, int funcindex, int n);
LUA_API const char *(lua_setupvalue) (lua_State *L, int funcindex, int n);

LUA_API void *(lua_upvalueid) (lua_State *L, int fidx, int n);
LUA_API void  (lua_upvaluejoin) (lua_State *L, int fidx1, int n1,
                                               int fidx2, int n2);

LUA_API void (lua_sethook) (lua_State *L, lua_Hook func, int mask, int count);
LUA_API lua_Hook (lua_gethook) (lua_State *L);
LUA_API int (lua_gethookmask) (lua_State *L);
LUA_API int (lua_gethookcount) (lua_State *L);


struct lua_Debug {
  int event;
  const char *name;	    	    /* (n) */
  const char *namewhat;		    /* (n) 'global', 'local', 'field', 'method' */
  const char *what;	    	    /* (S) 'Lua', 'C', 'main', 'tail' */
  const char *source;		    /* (S) */
  size_t srclen;	    	    /* (S) */
  int currentline;	    	    /* (l) */
  int linedefined;	    	    /* (S) */
  int lastlinedefined;		    /* (S) */
  unsigned char nups;		    /* (u) number of upvalues */
  unsigned char nparams;	    /* (u) number of parameters */
  char isvararg;        	    /* (u) */
  char istailcall;	    	    /* (t) */
  int ftransfer;        	    /* (r) index of first value transferred */
  int ntransfer;                /* (r) number of transferred values */
  char short_src[LUA_IDSIZE];   /* (S) */

  /* private part */
  struct CallInfo *i_ci; /* active function */
};

/* }====================================================================== */


#define LUAI_TOSTRAUX(x)	#x
#define LUAI_TOSTR(x)		LUAI_TOSTRAUX(x)

#define LUA_VERSION_MAJOR	LUAI_TOSTR(LUA_VERSION_MAJOR_N)
#define LUA_VERSION_MINOR	LUAI_TOSTR(LUA_VERSION_MINOR_N)
#define LUA_VERSION_RELEASE	LUAI_TOSTR(LUA_VERSION_RELEASE_N)

#define LUA_VERSION	"(Star) Lua " LUA_VERSION_MAJOR "." LUA_VERSION_MINOR
#define LUA_RELEASE	LUA_VERSION "." LUA_VERSION_RELEASE


/******************************************************************************
* Copyright (C) 1994-2024 Lua.org, PUC-Rio.
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************/


#endif
