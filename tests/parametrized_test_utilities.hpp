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

    virtual std::size_t get_num_elements() = 0;
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

    void* get_data() override
    {
      return data.data();
    }

    std::size_t get_num_elements() override
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
      auto* other_data = dynamic_cast<Data const*>(&bd);
      return data == other_data->data;
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

