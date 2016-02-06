//
// Compatibility functions so that Selene can work with various flavors of Lua.
//

#pragma once

extern "C" 
{
	#include <lua.h>
	#include <lauxlib.h>
}

#ifndef LUA_OK
#define LUA_OK 0
#endif

namespace sel 
{
	struct compat
	{
		static inline lua_Number _lua_tonumberx(lua_State *l, const int index, int *isnum)
		{
			lua_Number result;
#if LUA_VERSION_NUM >= 502
			result = lua_tonumberx(l, index, isnum);
#else
			if (lua_isnumber(l, index))
			{
				if (isnum) *isnum = 1;
				result = lua_tonumber(l, index);
			}
			else
			{
				if (isnum) *isnum = 0;
				result = 0;
			}
#endif
			return result;
		}

		static inline int _lua_tointegerx(lua_State *l, const int index, int *isnum)
		{
			int result;
#if LUA_VERSION_NUM >= 502
			result = (int)(lua_tointegerx(l, index, isnum));
#else
			if (lua_isnumber(l, index))
			{
				if(isnum) *isnum = 1;
				result = (int)(lua_tointeger(l, index));
			}
			else
			{
				if(isnum) *isnum = 0;
				result = 0;
			}
#endif
			return result;
		}

		static inline unsigned int _lua_tounsignedx(lua_State *l, const int index, int *isnum)
		{
			unsigned int result;
#if LUA_VERSION_NUM >= 503
			result = (unsigned int)(_lua_tointegerx(l, index, isnum));
#elif LUA_VERSION_NUM >= 502
			result = (unsigned int)(lua_tounsignedx(l, index, isnum));
#else
			result = (unsigned int)(_lua_tointegerx(l, index, isnum));
#endif
			return result;
		}

		static inline unsigned int _lua_tounsigned(lua_State *l, const int index)
		{
			unsigned int result;
#if LUA_VERSION_NUM >= 503
			result = (unsigned int)(lua_tointeger(l, index));
#elif LUA_VERSION_NUM >= 502
			result = (unsigned int)(lua_tounsigned(l, index));
#else
			result = (unsigned int)(lua_tointeger(l, index));
#endif
			return result;
		}

		static inline int _luaL_checkinteger(lua_State *l, const int index)
		{
#if LUA_VERSION_NUM >= 503
			return (int)luaL_checkinteger(l, index);
#else
			return (int)luaL_checkint(l, index);
#endif
		}

		static inline unsigned int _luaL_checkunsigned(lua_State *l, const int index)
		{
#if LUA_VERSION_NUM >= 502
			return (unsigned int)luaL_checkunsigned(l, index);
#else
			return (unsigned int)luaL_checkinteger(l, index);
#endif
		}

		static inline void *_luaL_testudata(lua_State *l, const int index, const char *name)
		{
#if LUA_VERSION_NUM >= 502
			return luaL_testudata(l, index, name);
#else
			void *p = lua_touserdata(l, index);

			if (p != nullptr) 
			{  
				if (lua_getmetatable(l, index)) 
				{  
					luaL_getmetatable(l, name);
					if (!lua_rawequal(l, -1, -2))
						p = nullptr;
					lua_pop(l, 2);
				}
			}

			return p;
#endif
		}

		static inline int _lua_absindex(lua_State *l, const int index)
		{
#if LUA_VERSION_NUM >= 502
			return lua_absindex(l, index);
#else
			return (index > 0 || index <= LUA_REGISTRYINDEX) ? index : lua_gettop(l) + index + 1;
#endif
		}

		static inline void _lua_pushglobaltable(lua_State *l)
		{
#if LUA_VERSION_NUM >= 502
			lua_pushglobaltable(l);
#else
			lua_pushvalue(l, LUA_GLOBALSINDEX);
#endif
		}
	};
}