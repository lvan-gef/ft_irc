#ifndef OPTIONAL_HPP
#define OPTIONAL_HPP

template <typename T>
class Optional {
  public:
    Optional();

    Optional(const Optional &rhs) = delete;
    Optional &operator=(const Optional &rhs) = delete;

    Optional(Optional &&rhs) noexcept;
    Optional &operator=(Optional &&rhs) noexcept;

    ~Optional();

public:
    T &get_value();
    const T &get_value() const;
    void set_value(const T &value);
    void reset();
    bool has_value() const;

  private:
    bool _has_value;
    T _value;
};

#include "../templates/Optional.tpp"

#endif // !OPTIONAL_HPP
