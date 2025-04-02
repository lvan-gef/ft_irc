#ifndef EpollInterface_HPP
#define EpollInterface_HPP

class EpollInterface {
  public:
    EpollInterface() = default;

    EpollInterface(const EpollInterface &) = default;
    EpollInterface &operator=(const EpollInterface &) = default;

    EpollInterface(EpollInterface &&) = default;
    EpollInterface &operator=(EpollInterface &&) = default;

    virtual ~EpollInterface() = default;

  public:
    virtual void notifyEpollUpdate(int fd) = 0;
};

#endif // EpollInterface_HPP
