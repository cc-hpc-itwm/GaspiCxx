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
 * Allreduce.cpp
 *
 */

#include <collectives/Allreduce.hpp>
#include <utility/Macros.hpp>
//#include <passive/Passive.hpp>
//#include <Runtime.hpp>
//#include <singlesided/write/TargetBuffer.hpp>
//#include <utility/serialization.hpp>
//
//#include <vector>
//#include <memory>
#include <complex>

namespace gaspi {
namespace collectives {

namespace detail {

gaspi_operation_t
getGaspiOperationT
  (Allreduce::Type const & type) {

  gaspi_operation_t ret;

  switch (type) {

    case Allreduce::MIN: {
      ret = GASPI_OP_MIN;
      break;
    }

    case Allreduce::MAX: {
      ret = GASPI_OP_MAX;
      break;
    }

    case Allreduce::SUM: {
      ret = GASPI_OP_SUM;
      break;
    }

    default: {
      throw std::runtime_error
        (CODE_ORIGIN + "Unsupported Allreduce::Type");
    }

  }

  return ret;

}

template <typename T>
gaspi_datatype_t
getGaspiDatatypeT();

template <>
gaspi_datatype_t
getGaspiDatatypeT<int>() {
  return GASPI_TYPE_INT;
}

template <>
gaspi_datatype_t
getGaspiDatatypeT<unsigned int>() {
  return GASPI_TYPE_INT;
}

template <>
gaspi_datatype_t
getGaspiDatatypeT<float>() {
  return GASPI_TYPE_FLOAT;
}

template <>
gaspi_datatype_t
getGaspiDatatypeT<double>() {
  return GASPI_TYPE_DOUBLE;
}

template <>
gaspi_datatype_t
getGaspiDatatypeT<long>() {
  return GASPI_TYPE_LONG;
}

template <>
gaspi_datatype_t
getGaspiDatatypeT<unsigned long>() {
  return GASPI_TYPE_ULONG;
}

}

template <typename T>
T
allreduce
  ( T const & input
  , Allreduce::Type const & type
  , Context & context ) {

  T result;

  GASPI_CHECK(
    gaspi_allreduce
      ( const_cast<T * const>(&input)
      , const_cast<T * const>(&result)
      , 1
      , detail::getGaspiOperationT(type)
      , detail::getGaspiDatatypeT<T>()
      , context.group().group()
      , GASPI_BLOCK) );

  return result;
}

template <>
std::complex<float>
allreduce<std::complex<float> >
  ( std::complex<float> const & input
  , Allreduce::Type const & type
  , Context & context ) {

  if( (type == Allreduce::MIN) ||
      (type == Allreduce::MAX) ) {
    throw std::runtime_error
      (CODE_ORIGIN + "Complex data type does not support MIN or MAX");
  }

  std::complex<float> result;

  GASPI_CHECK(
    gaspi_allreduce
      ( const_cast<float * const>
          (reinterpret_cast<const float(&)[2]>(input))
      , const_cast<float * const>
          (reinterpret_cast<      float(&)[2]>(result))
      , 2
      , detail::getGaspiOperationT(type)
      , detail::getGaspiDatatypeT<float>()
      , context.group().group()
      , GASPI_BLOCK) );

  return result;
}

template <>
std::complex<double>
allreduce<std::complex<double> >
  ( std::complex<double> const & input
  , Allreduce::Type const & type
  , Context & context ) {

  if( (type == Allreduce::MIN) ||
      (type == Allreduce::MAX) ) {
    throw std::runtime_error
      (CODE_ORIGIN + "Complex data type does not support MIN or MAX");
  }

  std::complex<double> result;

  GASPI_CHECK(
    gaspi_allreduce
      ( const_cast<double * const>
          (reinterpret_cast<const double(&)[2]>(input))
      , const_cast<double * const>
          (reinterpret_cast<      double(&)[2]>(result))
      , 2
      , detail::getGaspiOperationT(type)
      , detail::getGaspiDatatypeT<double>()
      , context.group().group()
      , GASPI_BLOCK) );

  return result;
}

/// explicit template instantiation

template int allreduce<int>
  (int const&, Allreduce::Type const &, Context &);
template unsigned int allreduce<unsigned int>
  (unsigned int const&, Allreduce::Type const &, Context &);
template float allreduce<float>
  (float const&, Allreduce::Type const &, Context &);
template double allreduce<double>
  (double const&, Allreduce::Type const &, Context &);
template long allreduce<long>
  (long const&, Allreduce::Type const &, Context &);
template unsigned long allreduce<unsigned long>
  (unsigned long const&, Allreduce::Type const &, Context &);
template std::complex<float> allreduce<std::complex<float> >
  (std::complex<float> const&, Allreduce::Type const &, Context &);
template std::complex<double> allreduce<std::complex<double> >
  (std::complex<double> const&, Allreduce::Type const &, Context &);

}
}
