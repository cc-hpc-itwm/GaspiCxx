/*
 * Copyright (c) Fraunhofer ITWM - <http://www.itwm.fraunhofer.de/>, 2019 - 2021
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
 * along with GaspiCxx. If not, see <http://www.gnu.org/licenses/>.
 *
 * Filesystem.hpp
 *
 */

#ifndef FILESYSTEM_HPP_
#define FILESYSTEM_HPP_

#include <string>

namespace gaspi {

/** \brief returns the current working directory*/
std::string
getCurrentWorkingDirectory
  ( );

/** \brief sets the corrent working directory */
void
setCurrentWorkingDirectory
  (std::string const &);

}

#endif /* FILESYSTEM_HPP_ */
