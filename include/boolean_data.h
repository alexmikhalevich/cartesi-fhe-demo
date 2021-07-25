// Copyright 2021 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef FULLY_HOMOMORPHIC_ENCRYPTION_TRANSPILER_DATA_BOOLEAN_DATA_H_
#define FULLY_HOMOMORPHIC_ENCRYPTION_TRANSPILER_DATA_BOOLEAN_DATA_H_

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include <bit>
#include <cstring>
#include <string>
#include <span>
#include <type_traits>
#include <vector>

template <typename T>
void Encode(const T& value, std::span<bool> out) {
  std::make_unsigned_t<T> unsigned_value;
  std::memcpy(&unsigned_value, &value, sizeof(value));
  for (int j = 0; j < 8 * sizeof(T); ++j) {
    out[j] = ((unsigned_value >> j) & 1) != 0;
  }
}

template <typename T>
T Decode(std::span<bool> value) {
  using UnsignedT = std::make_unsigned_t<T>;

  UnsignedT unsigned_value = 0;
  for (int j = 0; j < 8 * sizeof(T); j++) {
    unsigned_value |= static_cast<UnsignedT>(value[j]) << j;
  }
  T signed_value = unsigned_value;
  std::memcpy(&signed_value, &unsigned_value, sizeof(signed_value));
  return signed_value;
}

template <typename ValueType, typename Enable = void>
class EncodedValue;

template <typename ValueType>
class EncodedValue<ValueType,
                   std::enable_if_t<std::is_integral_v<ValueType> &&
                                    !std::is_same_v<ValueType, bool>>> {
 public:
  EncodedValue() : array_(8 * sizeof(ValueType), 0) {}
  EncodedValue(ValueType value) : EncodedValue() { Encode(value); }

  void Encode(const ValueType& value) { ::Encode(value, get()); }

  ValueType Decode() { return ::Decode<ValueType>(get()); }

  std::span<bool> get() { return std::span{ reinterpret_cast<bool*>(array_.data()), array_.size() }; }
  operator std::span<bool>() { return std::span{ reinterpret_cast<bool*>(array_.data()), array_.size() }; }
  operator std::span<const bool>() const { return std::span{ reinterpret_cast<const bool*>(array_.data()), array_.size() }; }

  int32_t size() { return array_.size(); }

 private:
  std::vector<unsigned char> array_;
};

// "Specialization" of EncodedValue for bools, for interface consistency with
// other native/integral types.
template <>
class EncodedValue<bool> {
 public:
  EncodedValue() : value_(false) {}
  EncodedValue(bool value) : value_(value) {}

  void Encode(bool value) { value_ = value; }

  bool Decode() { return value_; }

  std::span<bool> get() { return std::span(&value_, 1); }
  operator std::span<bool>() { return std::span(&value_, 1); }
  operator std::span<const bool>() { return std::span(&value_, 1); }

  int32_t size() { return 1; }

 private:
  bool value_;
};

// FHE representation of an array of objects of a single type, encoded as
// a bit array.
template <typename ValueType,
          std::enable_if_t<std::is_integral_v<ValueType> &&
                           !std::is_same_v<ValueType, bool>>* = nullptr>
class EncodedArray {
 public:
  EncodedArray(size_t length) : length_(length), array_(kValueWidth * length, 0) {}

  EncodedArray(std::span<const ValueType> plaintext)
      : EncodedArray(plaintext.size()) {
    Encode(plaintext);
  }

  void Encode(std::span<const ValueType> plaintext) {
    assert(plaintext.size() == length_);
    for (int j = 0; j < plaintext.size(); j++) {
      ::Encode(plaintext[j], (*this)[j]);
    }
  }

  std::vector<ValueType> Decode() {
    std::vector<ValueType> plaintext(length_);
    for (int j = 0; j < length_; j++) {
      auto subspan = (*this)[j];
      plaintext[j] = ::Decode<ValueType>(subspan);
    }
    return plaintext;
  }

  std::span<bool> get() { return std::span{ reinterpret_cast<bool*>(array_.data()), array_.size() }; }
  operator std::span<bool>() { return std::span{ reinterpret_cast<bool*>(array_.data()), array_.size() }; }
  operator std::span<const bool>() const { return std::span{ reinterpret_cast<const bool*>(array_.data()), array_.size() }; }

  std::span<bool> operator[](size_t pos) {
    auto sp = std::span{ reinterpret_cast<bool*>(array_.data()), array_.size() };
    return sp.subspan(pos * kValueWidth, kValueWidth);
  }

  size_t length() const { return length_; }
  size_t size() const { return length_; }

  size_t bit_width() const { return array_.size(); }

 private:
  using UnsignedType = std::make_unsigned_t<ValueType>;
  static constexpr size_t kValueWidth = 8 * sizeof(ValueType);

  size_t length_;
  std::vector<unsigned char> array_;
};

// Represents a string as an FheArray of the template parameter CharT.
// Corresponds to std::basic_string
template <typename CharT>
class EncodedBasicString : public EncodedArray<CharT> {
 public:
  using SizeType = typename std::basic_string<CharT>::size_type;

  using EncodedArray<CharT>::EncodedArray;

  std::basic_string<CharT> Decode() {
    auto v = EncodedArray<CharT>::Decode();
    return std::basic_string<CharT>(v.begin(), v.end());
  }
};

// Instantiates a FHE representation of a string as an array of chars, which
// are themselves arrays of bits.
// Corresponds to std::string
using EncodedString = EncodedBasicString<char>;

using EncodedInt = EncodedValue<int>;

using EncodedChar = EncodedValue<char>;

using EncodedShort = EncodedValue<short>;

#endif  // FULLY_HOMOMORPHIC_ENCRYPTION_TRANSPILER_DATA_BOOLEAN_DATA_H_
