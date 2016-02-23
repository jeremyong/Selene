#pragma once

#include <tuple>
#include "traits.h"
#include "types.h"

namespace sel {

namespace detail {

template<typename... T>
struct type_t<std::tuple<T...>> {

    template <std::size_t... N>
    void _push_dispatcher(lua_State *l,
                          const std::tuple<T...> &values,
                          _indices<N...>) {
        ::sel::detail::_push_n(l, std::get<N>(values)...);
    }

    void _push(lua_State *l, const std::tuple<T...> &values) {
        constexpr int num_values = sizeof...(T);
        this->_push_dispatcher(l, values,
                               typename _indices_builder<num_values>::type());
    }

};

}

}
