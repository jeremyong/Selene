#pragma once

#include "ExceptionTypes.h"
#include "MetatableRegistry.h"
#include "types.h"

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

namespace sel {

namespace detail {

template<typename T>
struct type_t<T*> : referenceable_tag {

    T* _get(_id<T*>, lua_State *l, const int index) {
        if(MetatableRegistry::IsType(l, typeid(T), index)) {
            return (T*)lua_topointer(l, index);
        }
        return nullptr;
    }

    T* _check_get(_id<T*>, lua_State *l, const int index) {
        MetatableRegistry::CheckType(l, typeid(T), index);
        return (T *)lua_topointer(l, index);
    }

    void _push(lua_State *l, T* t) {
        if(t == nullptr) {
            lua_pushnil(l);
        }
        else {
            lua_pushlightuserdata(l, t);
            MetatableRegistry::SetMetatable(l, typeid(T));
        }
    }

};

template<typename T>
struct type_t : referenceable_tag {

    T& _get(_id<T&>, lua_State *l, const int index) {
        if(!MetatableRegistry::IsType(l, typeid(T), index)) {
            throw TypeError{
                MetatableRegistry::GetTypeName(l, typeid(T)),
                    MetatableRegistry::GetTypeName(l, index)
            };
        }

        T *ptr = (T*)lua_topointer(l, index);
        if(ptr == nullptr) {
            throw TypeError{MetatableRegistry::GetTypeName(l, typeid(T))};
        }
        return *ptr;
    }

    T _get(_id<T>, lua_State *l, const int index) {
        return this->_get(_id<T&>{}, l, index);
    }

    T& _check_get(_id<T&>, lua_State *l, const int index) {
        T *ptr = sel::detail::_check_get(_id<T*>{}, l, index);

        if(ptr == nullptr) {
            throw GetUserdataParameterFromLuaTypeError{
                MetatableRegistry::GetTypeName(l, typeid(T)),
                    index
            };
        }

        return *ptr;
    }

    T _check_get(_id<T>, lua_State *l, const int index) {
        return this->_check_get(_id<T&>{}, l, index);
    };

    void _push(lua_State *l, T& t) {
        lua_pushlightuserdata(l, &t);
        MetatableRegistry::SetMetatable(l, typeid(T));
    }

    void _push(lua_State *l, T&& t) {
        if(!MetatableRegistry::IsRegisteredType(l, typeid(t)))
        {
            throw CopyUnregisteredType(typeid(t));
        }

        void *addr = lua_newuserdata(l, sizeof(T));
        new(addr) T(std::forward<T>(t));
        MetatableRegistry::SetMetatable(l, typeid(T));
    }

};

}

}
