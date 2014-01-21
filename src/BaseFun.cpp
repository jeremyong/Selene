#include "BaseFun.h"

namespace sel {
namespace detail {
int _lua_dispatcher(lua_State *l) {
    BaseFun *fun = (BaseFun *)lua_touserdata(l, lua_upvalueindex(1));
    return fun->Apply(l);
}
}
}
