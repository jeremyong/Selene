#pragma once
#include <iostream>
#include <memory>
#include <typeinfo>
#include <unordered_map>

namespace sel {
class MetatableRegistry {
private:
    using TypeID = std::reference_wrapper<const std::type_info>;
    struct Hasher {
        std::size_t operator()(TypeID code) const {
            return code.get().hash_code();
        }
    };
    struct EqualTo {
        bool operator()(TypeID lhs, TypeID rhs) const {
            return lhs.get() == rhs.get();
        }
    };
    std::unordered_map<TypeID, const std::string*, Hasher, EqualTo> _metatables;

public:
    MetatableRegistry() {}

    inline void Insert(TypeID type, const std::string& name) {
        _metatables[type] = &name;
    }

    inline void Erase(TypeID type) {
        _metatables.erase(type);
    }

    inline const std::string* Find(TypeID type) {
        auto it = _metatables.find(type);
        if (it == _metatables.end()) return nullptr;
        return it->second;
    }
};
}
