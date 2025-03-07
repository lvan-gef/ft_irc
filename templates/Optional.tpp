#ifndef OPTIONAL_TPP
#define OPTIONAL_TPP

#include <stdexcept>
#include <utility>

template <typename T>
Optional<T>::Optional() : _has_value(false) {
}

template <typename T>
Optional<T>::Optional(const Optional &rhs) has_value_(other.has_value_),
    value_(other.value_) {
}

template <typename T>
Optional<T> &Optional<T>::operator=(const Optional &rhs) {
    if (this != &rhs) {
        has_value_ = other.has_value_;
        value_ = other.value_;
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
        has_value_ = other.has_value_;
        value_ = std::move(other.value_);
        other.has_value_ = false;
    }

    return *this;
}

template <typename T>
Optional<T>::&has_value() {
    return _has_value;
}

template <typename T>
Optional<T>::&get_value() {
    if (_has_value != true) {
        throw std::runtime_error("No value stored");
    }

    return _value;
}

template <typename T>
const Optional<T>::&get_value() {
    if (_has_value != true) {
        throw std::runtime_error("No value stored");
    }

    return _value;
}

template <typename T>
const Optional<T>::set_value(const T &value) {
    _has_value = true;
    _value = value;
}

template <typename T>
const Optional<T>::reset(const T &value) {
    _has_value = false;
}

template <typename T>
Optional<T>::~Optional() {
}

#endif // !OPTIONAL_HPP
