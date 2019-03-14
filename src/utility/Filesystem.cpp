/*
 * Copyright (c) Fraunhofer ITWM - <http://www.itwm.fraunhofer.de/>, 2017
 *
 * This file is part of GaspiCxx.
 *
 * GaspiCxx is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public License
 * version 3 as published by the Free Software Foundation.
 *
 * GaspiCxx is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GaspiLS. If not, see <http://www.gnu.org/licenses/>.
 *
 * Filesystem.cpp
 *
 */

#include <GaspiCxx/utility/Filesystem.hpp>
#include <GaspiCxx/utility/Macros.hpp>
#include <errno.h>
#include <unistd.h>

namespace gaspi {

std::string
getCurrentWorkingDirectory
  ( ) {

  char* const currentDirectory (getcwd (NULL, 0));

  if (currentDirectory == nullptr) {
    int errsv = errno;

    std::stringstream ss
      ("Could not determine current working directory: ");

    switch (errsv) {
      case EACCES: {
        ss << "Permission to read or search a component of the filename "
           << "was denied";
        break;
      }
      case EFAULT: {
        ss << "buf=NULL points to a bad address";
        break;
      }
      case EINVAL: {
        ss << "The size argument is zero and buf=NULL is not a null pointer";
        break;
      }
      case ENOMEM: {
        ss << "Out of memory";
        break;
      }
      case ENOENT: {
        ss << "The current working directory has been unlinked";
        break;
      }
      case ERANGE: {
        ss << "The size=0 argument is less than the length of the absolute "
           << "pathname of the working directory, including the terminating "
           << "null byte. You need to allocate a bigger array and try again";
        break;
      }
      default: {
        ss << "Unknown error";
      }
    }

    throw std::runtime_error
      (CODE_ORIGIN + ss.str());
  }

  std::string directory (currentDirectory);

  directory.append ("/");

  free (static_cast<void*> (currentDirectory));

  return directory;
}

void
setCurrentWorkingDirectory
  (std::string const & dir) {

  if (chdir(dir.c_str())) {
    int errsv = errno;

    std::stringstream ss
      ("Could not change current working directory: ");

    switch (errsv) {
      case EACCES: {
        ss << "Search permission is denied for one of the components of "
           << dir;
        break;
      }
      case EFAULT: {
        ss << dir
           << " points outside your accessible address space";
        break;
      }
      case EIO: {
        ss << "An I/O error orccured: "
           << dir;
        break;
      }
      case ELOOP: {
        ss << "Too many symbolic links were encountered in resolving "
           << dir;
        break;
      }
      case ENAMETOOLONG: {
        ss << dir
           << " is too long";
        break;
      }
      case ENOENT: {
        ss << "The file does not exist: "
           << dir;
        break;
      }
      case ENOMEM: {
        ss << "Insufficient kernel memory was available";
        break;
      }
      case ENOTDIR: {
        ss << "A component of "
           << dir
           << " is not a directory";
        break;
      }
      default: {
        ss << "Unknown error: "
           << dir;
      }
    }

    throw std::runtime_error
      (CODE_ORIGIN + ss.str());
  }

}

}

