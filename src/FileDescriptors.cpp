#include <unistd.h>

#include "../include/FileDescriptors.hpp"

FileDescriptors::FileDescriptors(const int fd) : _fd(fd) {
}

FileDescriptors::FileDescriptors(FileDescriptors &&rhs) noexcept
    : _fd(rhs._fd) {
    rhs._fd = -1;
}

FileDescriptors &FileDescriptors::operator=(FileDescriptors &&rhs) noexcept {
    if (this != &rhs) {
        if (_fd >= 0) {
            close(_fd);
        }

        _fd = rhs._fd;
        rhs._fd = -1;
    }

    return *this;
}

FileDescriptors::~FileDescriptors() {
    if (_fd >= 0) {
        close(_fd);
    }
}

int FileDescriptors::get() const noexcept {
    return _fd;
}

FileDescriptors &FileDescriptors::operator=(const int fd) {
    if (_fd >= 0) {
        close(_fd);
    }

    _fd = fd;
    return *this;
}
