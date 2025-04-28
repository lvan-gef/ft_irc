#ifndef FILEDESCRIPTORS_HPP
#define FILEDESCRIPTORS_HPP

class FileDescriptors {
  public:
    explicit FileDescriptors(int fd);

    FileDescriptors(const FileDescriptors &rhs) = delete;
    FileDescriptors &operator=(const FileDescriptors &rhs) = delete;

    FileDescriptors(FileDescriptors &&rhs) noexcept;
    FileDescriptors &operator=(FileDescriptors &&rhs) noexcept;

    ~FileDescriptors();

  public:
    FileDescriptors &operator=(int fd);
    int get() const noexcept;

  private:
    int _fd;
};

#endif // !FILEDESCRIPTORS_HPP
