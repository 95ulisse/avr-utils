#pragma once

#include "avr-utils/utility.hpp"

namespace avr {

template <typename>
class Function;

/** Container for callable objects. Can store either free functions or member functions. */
template <typename TReturn, typename... TArgs>
class Function<TReturn(TArgs...)> {
public:

    /** Constructs a function object from a free function. */
    Function(TReturn (*f)(TArgs...))
        : _obj(nullptr),
          _f_free(f)
    {
    }

    /** Constructs a function object from a pointer to an object and a member pointer. */
    template <typename TObject>
    Function(TObject* obj, TReturn (TObject::*f)(TArgs...))
        : _obj(reinterpret_cast<Dummy*>(obj)),
          _f_member(reinterpret_cast<MemberFunction>(f))
    {
    }

    TReturn operator()(TArgs&&... args) {
        if (_obj == nullptr) {
            return _f_free(forward<TArgs>(args)...);
        } else {
            return (_obj->*_f_member)(forward<TArgs>(args)...);
        }
    }

private:

    class Dummy;

    using MemberFunction = TReturn (Dummy::* const)(TArgs...);
    using FreeFunction = TReturn (* const)(TArgs...);

    Dummy* const _obj;
    union {
        MemberFunction _f_member;
        FreeFunction _f_free;
    };
};

} // namespace