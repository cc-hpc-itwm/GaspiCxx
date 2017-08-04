/*
 * Copyright (c) Fraunhofer ITWM - <http://www.itwm.fraunhofer.de/>, 2016
 * 
 * This file is part of GaspiLS.
 * 
 * GaspiLS is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public License
 * version 3 as published by the Free Software Foundation.
 * 
 * GaspiLS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with GaspiLS. If not, see <http://www.gnu.org/licenses/>.
 *
 * BufferDescription.hpp
 *
 */

#ifndef BUFFERDESCRIPTION_HPP_
#define BUFFERDESCRIPTION_HPP_

#include <ostream>
extern "C" {
#include <GASPI.h>
#include <GASPI_Ext.h>
}

namespace gaspi {
namespace singlesided {

class BufferDescription {

  public:

    BufferDescription
      ()
    : _rank()
    , _segmentId()
    , _offset()
    , _size()
    , _notificationId()
    {}

    BufferDescription
      ( gaspi_rank_t rank
      , gaspi_segment_id_t segmentId
      , gaspi_offset_t offset
      , gaspi_size_t size
      , gaspi_notification_id_t notificationId )
    : _rank(rank)
    , _segmentId(segmentId)
    , _offset(offset)
    , _size(size)
    , _notificationId(notificationId)
    {}

    gaspi_rank_t
    rank
      () const
    {
      return _rank;
    }

    gaspi_rank_t &
    rank
      ()
    {
      return _rank;
    }

    gaspi_segment_id_t
    segmentId
      () const
    {
      return _segmentId;
    }

    gaspi_segment_id_t &
    segmentId
      ()
    {
      return _segmentId;
    }

    gaspi_offset_t
    offset
      () const
    {
      return _offset;
    }

    gaspi_offset_t &
    offset
      ()
    {
      return _offset;
    }

    gaspi_size_t
    size
      () const
    {
      return _size;
    }

    gaspi_size_t &
    size
      ()
    {
      return _size;
    }

    gaspi_notification_id_t
    notificationId
      () const
    {
      return _notificationId;
    }

    gaspi_notification_id_t &
    notificationId
      ()
    {
      return _notificationId;
    }

  private:

    gaspi_rank_t            _rank;
    gaspi_segment_id_t      _segmentId;
    gaspi_offset_t          _offset;
    gaspi_size_t            _size;
    gaspi_notification_id_t _notificationId;

};

std::ostream&
operator<<
  ( std::ostream& os
  , const BufferDescription& obj );

}
}

#endif /* BUFFERDESCRIPTION_HPP_ */
