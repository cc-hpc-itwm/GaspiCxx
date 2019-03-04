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
#include <unistd.h>

namespace gaspi {

std::string
getCurrentWorkingDirectory
  ( ) {

  char* const currentDirectory (getcwd (NULL, 0));

  if (currentDirectory == nullptr) {
    throw std::runtime_error
      (CODE_ORIGIN + "Could not determine current working directory");
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
    throw std::runtime_error
      (CODE_ORIGIN + "Could not change current working directory");
  }

}

}

