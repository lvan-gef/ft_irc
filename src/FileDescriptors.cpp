/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   FileDescriptors.cpp                                :+:    :+:            */
/*                                                     +:+                    */
/*   By: lvan-gef <lvan-gef@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/03/07 22:51:02 by lvan-gef      #+#    #+#                 */
/*   Updated: 2025/03/07 22:51:02 by lvan-gef      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>

#include "../include/FileDescriptors.hpp"

FileDescriptors::FileDescriptors(int fd) : _fd(fd) {
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

void FileDescriptors::set(int fd) noexcept {
    if (fd >= 0) {
        close(_fd);
        _fd = -1;
    }

    _fd = fd;
}

FileDescriptors &FileDescriptors::operator=(int fd) {
    if (_fd >= 0) {
        close(_fd);
    }

    _fd = fd;
    return *this;
}
