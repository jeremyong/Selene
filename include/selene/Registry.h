#pragma once

#include "Class.h"
#include "exotics.h"
#include "Fun.h"
#include "Obj.h"
#include <vector>

namespace sel {
namespace detail {
template <typename T>
struct lambda_traits : public lambda_traits<decltype(&T::operator())> {};

template <typename T, typename Ret, typename... Args>
struct lambda_traits<Ret(T::*)(Args...) const> {
    using Fun = std::function<Ret(Args...)>;
};
}
class Registry {
private:
    MetatableRegistry _metatables;
    std::vector<std::unique_ptr<BaseFun>> _funs;
    std::vector<std::unique_ptr<BaseObj>> _objs;
    std::vector<std::unique_ptr<BaseClass>> _classes;
    lua_State *_state;
public:
    Registry(lua_State *state) : _state(state) {}

    template <typename L>
    void Register(L lambda) {
        Register((typename detail::lambda_traits<L>::Fun)(lambda));
    }

    template <typename Ret, typename... Args>
    void Register(std::function<Ret(Args...)> fun) {
        constexpr int arity = detail::_arity<Ret>::value;
        auto tmp = std::unique_ptr<BaseFun>(
            new Fun<arity, Ret, Args...>{_state, _metatables, fun});
        _funs.push_back(std::move(tmp));
    }

    template <typename Ret, typename... Args>
    void Register(Ret (*fun)(Args...)) {
        constexpr int arity = detail::_arity<Ret>::value;
        auto tmp = std::unique_ptr<BaseFun>(
            new Fun<arity, Ret, Args...>{_state, _metatables, fun});
        _funs.push_back(std::move(tmp));
    }

    template <typename T, typename... Funs>
    void Register(T &t, std::tuple<Funs...> funs) {
        Register(t, funs,
                 typename detail::_indices_builder<sizeof...(Funs)>::type{});
    }

    template <typename T, typename... Funs, size_t... N>
    void Register(T &t, std::tuple<Funs...> funs, detail::_indices<N...>) {
        RegisterObj(t, std::get<N>(funs)...);
    }

    template <typename T, typename... Funs>
    void RegisterObj(T &t, Funs... funs) {
        auto tmp = std::unique_ptr<BaseObj>(
            new Obj<T, Funs...>{_state, &t, funs...});
        _objs.push_back(std::move(tmp));
    }

    template <typename T, typename... CtorArgs, typename... Funs, size_t... N>
    void RegisterClass(const std::string &name, std::tuple<Funs...> funs,
                       detail::_indices<N...>) {
        RegisterClassWorker<T, CtorArgs...>(name, std::get<N>(funs)...);
    }

    template <typename T, typename... CtorArgs, typename... Funs>
    void RegisterClassWorker(const std::string &name, Funs... funs) {
        auto tmp = std::unique_ptr<BaseClass>(
            new Class<T, Ctor<T, CtorArgs...>, Funs...>
            {_state, _metatables, name, funs...});
        _classes.push_back(std::move(tmp));
    }
};
}
