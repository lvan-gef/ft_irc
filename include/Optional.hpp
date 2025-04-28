#ifndef OPTIONAL_HPP
#define OPTIONAL_HPP

template <typename T>
class Optional {
  public:
    Optional();

    Optional(const Optional &rhs);
    Optional &operator=(const Optional &rhs);

    Optional(Optional &&rhs) noexcept;
    Optional &operator=(Optional &&rhs) noexcept;

    ~Optional();

  public:
    T &get_value();
    const T &get_value() const;
    void set_value(const T &value) noexcept;
    void reset() noexcept;
    bool has_value() const noexcept;

  private:
    bool _has_value;
    T _value;
};

#include "../templates/Optional.tpp"

#endif // !OPTIONAL_HPP
