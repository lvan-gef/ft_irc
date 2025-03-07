/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   FileDescriptors.hpp                                :+:    :+:            */
/*                                                     +:+                    */
/*   By: lvan-gef <lvan-gef@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/03/07 22:51:16 by lvan-gef      #+#    #+#                 */
/*   Updated: 2025/03/07 22:51:16 by lvan-gef      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

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
    void set(int fd) noexcept;

  private:
    int _fd;
};

#endif // !FILEDESCRIPTORS_HPP
