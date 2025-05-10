#include <unistd.h>

#include "../include/FileDescriptor.hpp"

FileDescriptor::FileDescriptor(const int fd) : _fd(fd) {
}

FileDescriptor::FileDescriptor(FileDescriptor &&rhs) noexcept : _fd(rhs._fd) {
    rhs._fd = -1;
}

FileDescriptor &FileDescriptor::operator=(FileDescriptor &&rhs) noexcept {
    if (this != &rhs) {
        if (_fd >= 0) {
            close(_fd);
        }

        _fd = rhs._fd;
        rhs._fd = -1;
    }

    return *this;
}

FileDescriptor::~FileDescriptor() {
    if (_fd >= 0) {
        close(_fd);
    }
}

int FileDescriptor::get() const noexcept {
    return _fd;
}

void FileDescriptor::clos() noexcept {
    if (_fd >= 0) {
        close(_fd);
    }

    _fd = -1;
}

bool FileDescriptor::isClosed() const noexcept {
    return _fd < 0;
}

FileDescriptor &FileDescriptor::operator=(const int fd) {
    if (_fd >= 0) {
        close(_fd);
    }

    _fd = fd;
    return *this;
}
