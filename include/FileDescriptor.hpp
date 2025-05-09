#ifndef FILEDESCRIPTOR_HPP
#define FILEDESCRIPTOR_HPP

class FileDescriptor {
  public:
    explicit FileDescriptor(int fd);

    FileDescriptor(const FileDescriptor &rhs) = delete;
    FileDescriptor &operator=(const FileDescriptor &rhs) = delete;

    FileDescriptor(FileDescriptor &&rhs) noexcept;
    FileDescriptor &operator=(FileDescriptor &&rhs) noexcept;

    ~FileDescriptor();

  public:
    FileDescriptor &operator=(int fd);
    int get() const noexcept;
    void clos() noexcept;
    bool isClosed() const noexcept;

  private:
    int _fd;
};

#endif // !FILEDESCRIPTOR_HPP
