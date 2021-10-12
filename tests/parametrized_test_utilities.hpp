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
 * parameterized_test_utilities.hpp
 *
 */

#include <algorithm>
#include <cmath>
#include <vector>

using DataSize = std::size_t;
using ElementType = std::string;

template<template <typename> class Factory>
auto select_factory_by_type(ElementType element_type)
{
  if (element_type == "int") return &Factory<int>::factory;
  if (element_type == "float") return &Factory<float>::factory;
  if (element_type == "double") return &Factory<double>::factory;

  throw std::runtime_error("[select_factory_type] Unknown element type \"" + element_type + "\"");
}

class BaseData
{
  public:
    virtual ~BaseData() {};
    virtual void* get_data() = 0;

    virtual std::size_t get_num_elements() const = 0;
    virtual void fill(double value) = 0;
    virtual void fill_from_list(std::vector<double> const& values) = 0;
    virtual void fill_from_list_and_scale(std::vector<double> const& values, std::size_t scaling_factor) = 0;

    bool operator==(BaseData const& bd) const
    {
      return is_equal(bd);
    }

  private:
    virtual bool is_equal(BaseData const& bd) const = 0;
};

template<typename T>
class Data : public BaseData
{
  public:
    Data(DataSize size)
    : data(size)
    {}
    Data(Data const&) = default;
    ~Data() = default;

    T const& operator[](std::size_t idx) const
    {
      return data[idx];
    }

    void* get_data() override
    {
      return data.data();
    }

    std::size_t get_num_elements() const override
    {
      return data.size();
    }

    void fill(double value) override
    {
      std::fill(data.begin(), data.end(), convert_value_to_expected_type(value));
    }

    void fill_from_list(std::vector<double> const& values) override
    {
      assert(data.size() == values.size());
      std::transform(values.begin(), values.end(), data.begin(),
                    [&](double value) { return convert_value_to_expected_type(value);});
    }

    void fill_from_list_and_scale(std::vector<double> const& values,
                                  std::size_t scaling_factor) override
    {
      fill_from_list(values);
      std::transform(data.begin(), data.end(), data.begin(),
                    [&](T value) { return value * convert_value_to_expected_type(scaling_factor);});
    }

  private:
    bool is_equal(BaseData const& bd) const override
    {
      auto const epsilon = 1e-2;
      auto* other_data = dynamic_cast<Data const*>(&bd);

      if (get_num_elements() != other_data->get_num_elements())
      {
        return false;
      }

      for (std::size_t i = 0UL; i < get_num_elements(); ++i)
      {
        if (std::abs(data[i]-(*other_data)[i]) > epsilon)
        {
          std::cout << "Data element at position " << i
                    << " does not match: " << data[i] << " [actual]"
                    << " != " << (*other_data)[i] << " [expected]" << std::endl;
          return false;
        }
      }
      return true;
    }

    template <typename U>
    using ReturnTypeIntegral = typename std::enable_if_t<std::is_integral<U>::value, U>;
    template <typename U>
    using ReturnTypeFloat = typename std::enable_if_t<std::is_same<U, float>::value, U>;
    template <typename U>
    using ReturnTypeDouble = typename std::enable_if_t<std::is_same<U, double>::value, U>;


    template<typename U = T>
    ReturnTypeIntegral<U> convert_value_to_expected_type(double value)
    {
      return static_cast<T>(std::trunc(value));
    }

    template<typename U = T>
    ReturnTypeFloat<U> convert_value_to_expected_type(double value)
    {
      return static_cast<T>(value);
    }
    template<typename U = T>
    ReturnTypeDouble<U> convert_value_to_expected_type(double value)
    {
      return value;
    }

    std::vector<T> data;
};

template<typename T>
class DataFactory
{
  public:
    static auto factory(DataSize size)
    {
      std::unique_ptr<BaseData> d = std::make_unique<Data<T>>(size);
      return d;
    }
};

