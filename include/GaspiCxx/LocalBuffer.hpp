#pragma once

#include <cassert>
#include <iterator>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <vector>

namespace gaspi
{
  template <typename T> class LocalBufferIterator;

  /** A LocalBuffer is defined as a raw pointer to a contiguous array of template type T.

    The local buffer does not own the pointer, nor does it manage it's allocation.
    A LocalBuffer is only a wrapper around already allocated array that provides typed access
   */
  template <typename T>
  class LocalBuffer
  {
    public:

      /**
       * Constructor.
       *
       * @param data                  Raw pointer to an array of elements of type T.
       * @param number_of_elements    Number of elements of type T in the array.
      */
      LocalBuffer(T* const data, std::size_t number_of_elements);

      //! Copy constructor that creates a LocalBuffer pointing to the same data as the copied one.
      LocalBuffer(LocalBuffer const& );

      //! Copy assignment constructor.
      LocalBuffer& operator=(LocalBuffer const& ) = default;
      LocalBuffer() = delete;

      //! Total number of elements of the buffer.
      constexpr std::size_t get_number_elements() const;

      //! Total size of the array in bytes.
      constexpr std::size_t get_size_bytes() const;

      //! Returns the raw pointer of the array data.
      T* get_data() const;

      //! Returns the element stored by the array at a given index.
      T element_at(std::size_t) const;

      //! Create iterator pointing at the start of the array data
      LocalBufferIterator<T> begin() const;

      //! Create iterator pointing at the end of the array data
      LocalBufferIterator<T> end() const;

      //! Exception thrown when an iterator tries to access data beyond the bounds of the array buffer
      class OutOfRange : public std::exception
      {
        const char* what() const noexcept
        {
          return "LocalBuffer: index out of range";
        }
      };

    private:
      T* data;
      std::size_t number_of_elements;
  };

  template <typename T>
  LocalBuffer<T>::LocalBuffer(T* const data,  std::size_t number_of_elements)
    : data(data), number_of_elements(number_of_elements)
  { }

  template <typename T>
  LocalBuffer<T>::LocalBuffer(LocalBuffer const& other)
    : data(other.get_data()), number_of_elements(other.get_number_elements())
  {}

  template <typename T>
  constexpr std::size_t LocalBuffer<T>::get_number_elements() const
  {
    return number_of_elements;
  }

  template <typename T>
  constexpr std::size_t LocalBuffer<T>::get_size_bytes() const
  {
    return get_number_elements() * sizeof(T);
  }

  template <typename T>
  T* LocalBuffer<T>::get_data() const
  {
    return data;
  }

  template <typename T>
  T LocalBuffer<T>::element_at(std::size_t index) const
  {
    // error checking
    if (index >= get_number_elements())
    {
      throw OutOfRange();
    }
    return data[index];
  }

  template <typename T>
  LocalBufferIterator<T> LocalBuffer<T>::begin() const
  {
    return LocalBufferIterator<T>(*this, 0);
  }

  template <typename T>
  LocalBufferIterator<T> LocalBuffer<T>::end() const
  {
    return LocalBufferIterator<T>(*this, get_number_elements());
  }

  /**
   * \brief Defines bidirectional iterators for the LocalBuffer objects.
   *
   * The data is assumed to be a contiguous array in memory
   */
  template <typename T>
  class LocalBufferIterator
  {
    public:
      using iterator_category = std::bidirectional_iterator_tag;  //!< Iterator type
      using value_type = T;                                       //!< Value type returned by the iterator
      using difference_type = std::ptrdiff_t;                     //!< Type of the difference between two iterators
      using pointer = T*;                                         //!< Pointer type returned by the iterator
      using reference = T&;                                       //!< Reference type returned by the iterator

      //! Constructor based on an existing LocalBuffer and an index within the data
      explicit LocalBufferIterator(LocalBuffer<T> const& buffer, std::size_t index)
      : buffer(buffer), index(index)
      {}

      //! Copy constructor
      LocalBufferIterator(LocalBufferIterator const& it)
      : buffer(it.buffer), index(it.index)
      {}

      //! Custom assignment operator (cannot use default
      //! because the iterator contains a reference to the LocalBuffer object)
      LocalBufferIterator& operator=(LocalBufferIterator &it)
      { return {it}; }

      //! Default destructor
      ~LocalBufferIterator() = default;

      //! Dereferencing operator
      reference operator*() const { return *(buffer.get_data() + index); }
      //! Equality operator
      bool operator==(LocalBufferIterator const& other) const { return index == other.index; }
      //! Non-equality operator
      bool operator!=(LocalBufferIterator const& other) const { return !(*this == other); }

      //! Post-Increment operator
      LocalBufferIterator operator++(int)
      {
        LocalBufferIterator ret = *this;
        ++(*this);
        return ret;
      }
      //! Post-Decrement operator
      LocalBufferIterator operator--(int)
      {
        LocalBufferIterator ret = *this;
        --(*this);
        return ret;
      }
      //! Pre-Increment operator
      LocalBufferIterator& operator++()
      {
        if(index >= buffer.get_number_elements())
        {
          throw std::out_of_range("LocalBufferIterator: cannot increment");
        }
        ++index;
        return *this;
      }
      //! Pre-Decrement operator
      LocalBufferIterator& operator--()
      {
        if(index <= 0)
        {
          throw std::out_of_range("LocalBufferIterator: cannot decrement");
        }
        --index;
        return *this;
      }
      //! Advance operator
      LocalBufferIterator& operator+(std::size_t n)
      {
        if(index+n > buffer.get_volume())
        {
          throw std::out_of_range("LocalBufferIterator: cannot advance");
        }
        index += n;
        return *this;
      }
      //! Reverse advance operator
      LocalBufferIterator& operator-(std::size_t n)
      {
        if(index < n)
        {
          throw std::out_of_range("LocalBufferIterator: cannot reverse advance");
        }
        index -= n;
        return *this;
      }

    private:

      LocalBuffer<T> const& buffer;  //!< reference to buffer over which the iterator is defined
      std::size_t index;             //!< index within the buffer currently pointed to by the iterator
  };

}
