#ifndef CHANNEL_HPP
#define CHANNEL_HPP

class Channel {
  public:
    Channel();

    Channel(const Channel &rhs);
    Channel &operator=(const Channel &rhs);

    Channel(Channel &&rhs) noexcept;
    Channel &operator=(Channel &&rhs) noexcept;

    ~Channel();
};

#endif // !CHANNEL_HPP
