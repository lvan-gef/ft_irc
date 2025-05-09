#ifndef OPTIONAL_TPP
#define OPTIONAL_TPP

#include <stdexcept>
#include <utility>

template <typename T>
Optional<T>::Optional() = default;

template <typename T>
Optional<T>::Optional(const Optional &rhs)
    : _has_value(rhs._has_value), _value(rhs._value) {
}

template <typename T>
Optional<T> &Optional<T>::operator=(const Optional &rhs) {
    if (this != &rhs) {
        _has_value = rhs._has_value;
        _value = rhs._value;
    }

    return *this;
}

template <typename T>
Optional<T>::Optional(Optional &&rhs) noexcept
    : _has_value(rhs._has_value), _value(std::move(rhs._value)) {
}

template <typename T>
Optional<T> &Optional<T>::operator=(Optional &&rhs) noexcept {
    if (this != &rhs) {
        reset();
        _has_value = rhs._has_value;
        _value = std::move(rhs._value);
        rhs._has_value = false;
    }

    return *this;
}

template <typename T>
T &Optional<T>::get_value() {
    if (_has_value != true) {
        throw std::runtime_error("No value stored");
    }

    return _value;
}

template <typename T>
const T &Optional<T>::get_value() const {
    if (_has_value != true) {
        throw std::runtime_error("No value stored");
    }

    return _value;
}

template <typename T>
void Optional<T>::set_value(const T &value) noexcept {
    _has_value = true;
    _value = value;
}

template <typename T>
void Optional<T>::reset() noexcept {
    _has_value = false;
}

template <typename T>
bool Optional<T>::has_value() const noexcept {
    return _has_value;
}

#endif // !OPTIONAL_HPP
