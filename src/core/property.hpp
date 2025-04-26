#ifndef __PROPERTY_H__
#define __PROPERTY_H__

#include <functional>
namespace Speed::Core
{
    template <class T>
    class Property
    {
        std::function<T(void)> _get;
        std::function<void(const T &)> _set;

    public:
        Property(
            std::function<T(void)> get,
            std::function<void(const T &)> set)
            : _get(get),
              _set(set)
        {
        }

        operator T() const { return _get(); }
        void operator=(const T &t) { _set(t); }
    }; // class Property

    template <class T>
    class ReadOnlyProperty
    {
        std::function<T(void)> _get;

    public:
        ReadOnlyProperty(
            std::function<T(void)> get)
            : _get(get)
        {
        }

        operator T() const { return _get(); }
    }; // class ReadOnlyProperty

} // namespace Speed::Core

#endif // __PROPERTY_H__