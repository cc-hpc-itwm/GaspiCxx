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
 * along with GaspiCxx. If not, see <http://www.gnu.org/licenses/>.
 *
 * BufferDescription.cpp
 *
 */

#include <GaspiCxx/singlesided/BufferDescription.hpp>
#include <GaspiCxx/utility/Macros.hpp>
#include <GaspiCxx/utility/serialization.hpp>

namespace gaspi {
namespace serialization {

using namespace singlesided;

template<>
size_t
size
  (BufferDescription const & desc)
{
  return ( size (desc.rank())
         + size (desc.segmentId())
         + size (desc.offset())
         + size (desc.size())
         + size (desc.notificationId()) );
}

template<>
void
serialize
  ( std::ostream& os
  , BufferDescription const& desc)
{
  serialize (os, desc.rank());
  serialize (os, desc.segmentId());
  serialize (os, desc.offset());
  serialize (os, desc.size());
  serialize (os, desc.notificationId());
}


template<>
size_t
serialize
  ( void* const buffer
  , BufferDescription const& desc )
{
  char * cPtr (static_cast<char *> (buffer));

  cPtr += serialize (cPtr, desc.rank());
  cPtr += serialize (cPtr, desc.segmentId());
  cPtr += serialize (cPtr, desc.offset());
  cPtr += serialize (cPtr, desc.size());
  cPtr += serialize (cPtr, desc.notificationId());

  return (cPtr - static_cast<char const *> (buffer));
}

template<>
void
deserialize
  ( BufferDescription & desc
  , std::istream & in )
{
  deserialize (desc.rank(), in);
  deserialize (desc.segmentId(), in);
  deserialize (desc.offset(), in);
  deserialize (desc.size(), in);
  deserialize (desc.notificationId(), in);
}

template<>
size_t
deserialize
  ( BufferDescription & desc
  , void const * const buffer)
{
  char const * cPtr (static_cast<char const *> (buffer));

  cPtr += deserialize (desc.rank(), cPtr);
  cPtr += deserialize (desc.segmentId(), cPtr);
  cPtr += deserialize (desc.offset(), cPtr);
  cPtr += deserialize (desc.size(), cPtr);
  cPtr += deserialize (desc.notificationId(), cPtr);

  return (cPtr - static_cast<char const *> (buffer));
}

}   // namespace serialization

namespace singlesided {

std::ostream&
operator<<
  ( std::ostream& os
  , const BufferDescription& obj )
{
  os << "address ("
     << obj.rank() << ", "
     << static_cast<unsigned int>(obj.segmentId()) << ", "
     << obj.offset() << "), "
     << "size " << obj.size() << ", synch. id "
     << obj.notificationId();
  return os;
}

} // namespace singlesided
} // namespace gaspi
