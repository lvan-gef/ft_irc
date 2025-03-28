#ifndef EpollNotifier_HPP
#define EpollNotifier_HPP

class EpollNotifier {
  public:
    EpollNotifier() = default;

    EpollNotifier(const EpollNotifier &) = default;
    EpollNotifier &operator=(const EpollNotifier &) = default;

    EpollNotifier(EpollNotifier &&) = default;
    EpollNotifier &operator=(EpollNotifier &&) = default;

    virtual ~EpollNotifier() = default;

  public:
    virtual void notifyEpollUpdate(int fd) = 0;
};

#endif // EpollNotifier_HPP
