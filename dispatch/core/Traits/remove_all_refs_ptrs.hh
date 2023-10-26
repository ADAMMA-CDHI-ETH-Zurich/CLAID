#pragma once
#include <Traits>

namespace claid
{
    template <typename T>
    struct remove_all_ref_ptr { typedef T type; };

    template <typename T>
    struct remove_all_ref_ptr<T *> : public remove_all_ref_ptr<T> { };

    template <typename T>
    struct remove_all_ref_ptr<T * const> : public remove_all_ref_ptr<T> { };

    template <typename T>
    struct remove_all_ref_ptr<T * volatile> : public remove_all_ref_ptr<T> { };

    template <typename T>
    struct remove_all_ref_ptr<T * const volatile> : public remove_all_ref_ptr<T> { };

    template <typename T>
    struct remove_all_ref_ptr<T &> : public remove_all_ref_ptr<T> { };

    template <typename T>
    struct remove_all_ref_ptr<T &&> : public remove_all_ref_ptr<T> { };
}
