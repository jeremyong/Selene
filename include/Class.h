#pragma once

namespace sel {
struct BaseClass {
    virtual ~BaseClass() {}
};
template <typename Ctor, typename... Funs>
class Class {
public:
    Class(Ctor ctor, Funs... funs) {
    }
};
}
