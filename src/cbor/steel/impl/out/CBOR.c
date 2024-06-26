/*
   Copyright 2023, 2024 Microsoft Research

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/


#include "internal/CBOR.h"

#define VALIDATOR_ERROR_NOT_ENOUGH_DATA (1U)

#define VALIDATOR_ERROR_CONSTRAINT_FAILED (2U)

static uint8_t get_bitfield_gen8(uint8_t x, uint32_t lo, uint32_t hi)
{
  uint8_t op1 = (uint32_t)x << 8U - hi;
  return (uint32_t)op1 >> 8U - hi + lo;
}

static uint8_t set_bitfield_gen8(uint8_t x, uint32_t lo, uint32_t hi, uint8_t v)
{
  uint8_t op0 = 255U;
  uint8_t op1 = (uint32_t)op0 >> 8U - (hi - lo);
  uint8_t op2 = (uint32_t)op1 << lo;
  uint8_t op3 = ~op2;
  uint8_t op4 = (uint32_t)x & (uint32_t)op3;
  uint8_t op5 = (uint32_t)v << lo;
  return (uint32_t)op4 | (uint32_t)op5;
}

static uint32_t validate_recursive_error_not_enough_data = 1U;

#define MIN_SIMPLE_VALUE_LONG_ARGUMENT (32U)

#define MAX_SIMPLE_VALUE_ADDITIONAL_INFO (23U)

#define ADDITIONAL_INFO_LONG_ARGUMENT_8_BITS (24U)

#define ADDITIONAL_INFO_UNASSIGNED_MIN (28U)

#define ADDITIONAL_INFO_LONG_ARGUMENT_16_BITS (25U)

#define ADDITIONAL_INFO_LONG_ARGUMENT_32_BITS (26U)

#define ADDITIONAL_INFO_LONG_ARGUMENT_64_BITS (27U)

#define MIN_DETERMINISTIC_UINT8 (24U)

#define MIN_DETERMINISTIC_UINT16 (256U)

#define MIN_DETERMINISTIC_UINT32 (65536U)

#define MIN_DETERMINISTIC_UINT64 (4294967296ULL)

#define MIN_DETERMINISTIC_UINT8_AS_UINT64 (24ULL)

#define MIN_DETERMINISTIC_UINT16_AS_UINT64 (256ULL)

#define MIN_DETERMINISTIC_UINT32_AS_UINT64 (65536ULL)

static uint8_t read_u8(uint8_t *a)
{
  return a[0U];
}

static uint16_t read_u16(uint8_t *a)
{
  size_t pos_ = (size_t)1U;
  uint8_t last = a[pos_];
  uint8_t last1 = a[0U];
  uint16_t n = (uint16_t)last1;
  return (uint32_t)(uint16_t)last + (uint32_t)n * 256U;
}

static uint32_t read_u32(uint8_t *a)
{
  size_t pos_ = (size_t)3U;
  uint8_t last = a[pos_];
  size_t pos_1 = pos_ - (size_t)1U;
  uint8_t last1 = a[pos_1];
  size_t pos_2 = pos_1 - (size_t)1U;
  uint8_t last2 = a[pos_2];
  uint8_t last3 = a[0U];
  uint32_t n = (uint32_t)last3;
  uint32_t n0 = (uint32_t)last2 + n * 256U;
  uint32_t n1 = (uint32_t)last1 + n0 * 256U;
  return (uint32_t)last + n1 * 256U;
}

static uint64_t read_u64(uint8_t *a)
{
  size_t pos_ = (size_t)7U;
  uint8_t last = a[pos_];
  size_t pos_1 = pos_ - (size_t)1U;
  uint8_t last1 = a[pos_1];
  size_t pos_2 = pos_1 - (size_t)1U;
  uint8_t last2 = a[pos_2];
  size_t pos_3 = pos_2 - (size_t)1U;
  uint8_t last3 = a[pos_3];
  size_t pos_4 = pos_3 - (size_t)1U;
  uint8_t last4 = a[pos_4];
  size_t pos_5 = pos_4 - (size_t)1U;
  uint8_t last5 = a[pos_5];
  size_t pos_6 = pos_5 - (size_t)1U;
  uint8_t last6 = a[pos_6];
  uint8_t last7 = a[0U];
  uint64_t n = (uint64_t)last7;
  uint64_t n0 = (uint64_t)last6 + n * 256ULL;
  uint64_t n1 = (uint64_t)last5 + n0 * 256ULL;
  uint64_t n2 = (uint64_t)last4 + n1 * 256ULL;
  uint64_t n3 = (uint64_t)last3 + n2 * 256ULL;
  uint64_t n4 = (uint64_t)last2 + n3 * 256ULL;
  uint64_t n5 = (uint64_t)last1 + n4 * 256ULL;
  return (uint64_t)last + n5 * 256ULL;
}

static void write_u8(uint8_t x, uint8_t *a)
{
  uint8_t n_ = x;
  a[0U] = n_;
}

static void write_u16(uint16_t x, uint8_t *a)
{
  uint8_t lo = (uint8_t)x;
  uint16_t hi = (uint32_t)x / 256U;
  size_t pos_ = (size_t)1U;
  uint8_t n_ = (uint8_t)hi;
  a[0U] = n_;
  a[pos_] = lo;
}

static void write_u32(uint32_t x, uint8_t *a)
{
  uint8_t lo = (uint8_t)x;
  uint32_t hi = x / 256U;
  size_t pos_ = (size_t)3U;
  uint8_t lo1 = (uint8_t)hi;
  uint32_t hi1 = hi / 256U;
  size_t pos_1 = pos_ - (size_t)1U;
  uint8_t lo2 = (uint8_t)hi1;
  uint32_t hi2 = hi1 / 256U;
  size_t pos_2 = pos_1 - (size_t)1U;
  uint8_t n_ = (uint8_t)hi2;
  a[0U] = n_;
  a[pos_2] = lo2;
  a[pos_1] = lo1;
  a[pos_] = lo;
}

static void write_u64(uint64_t x, uint8_t *a)
{
  uint8_t lo = (uint8_t)x;
  uint64_t hi = x / 256ULL;
  size_t pos_ = (size_t)7U;
  uint8_t lo1 = (uint8_t)hi;
  uint64_t hi1 = hi / 256ULL;
  size_t pos_1 = pos_ - (size_t)1U;
  uint8_t lo2 = (uint8_t)hi1;
  uint64_t hi2 = hi1 / 256ULL;
  size_t pos_2 = pos_1 - (size_t)1U;
  uint8_t lo3 = (uint8_t)hi2;
  uint64_t hi3 = hi2 / 256ULL;
  size_t pos_3 = pos_2 - (size_t)1U;
  uint8_t lo4 = (uint8_t)hi3;
  uint64_t hi4 = hi3 / 256ULL;
  size_t pos_4 = pos_3 - (size_t)1U;
  uint8_t lo5 = (uint8_t)hi4;
  uint64_t hi5 = hi4 / 256ULL;
  size_t pos_5 = pos_4 - (size_t)1U;
  uint8_t lo6 = (uint8_t)hi5;
  uint64_t hi6 = hi5 / 256ULL;
  size_t pos_6 = pos_5 - (size_t)1U;
  uint8_t n_ = (uint8_t)hi6;
  a[0U] = n_;
  a[pos_6] = lo6;
  a[pos_5] = lo5;
  a[pos_4] = lo4;
  a[pos_3] = lo3;
  a[pos_2] = lo2;
  a[pos_1] = lo1;
  a[pos_] = lo;
}

static size_t jump_header_(uint8_t *a)
{
  size_t len1 = (size_t)1U;
  uint8_t v = read_u8(a);
  uint8_t *ar = a + len1;
  if (get_bitfield_gen8(v, 0U, 5U) == ADDITIONAL_INFO_LONG_ARGUMENT_8_BITS)
    if (get_bitfield_gen8(v, 5U, 8U) == CBOR_MAJOR_TYPE_SIMPLE_VALUE)
    {
      size_t len11 = (size_t)1U;
      read_u8(ar);
      return len1 + len11;
    }
    else
    {
      size_t len11 = (size_t)1U;
      read_u8(ar);
      return len1 + len11;
    }
  else if (get_bitfield_gen8(v, 0U, 5U) == ADDITIONAL_INFO_LONG_ARGUMENT_16_BITS)
  {
    size_t len11 = (size_t)2U;
    read_u16(ar);
    return len1 + len11;
  }
  else if (get_bitfield_gen8(v, 0U, 5U) == ADDITIONAL_INFO_LONG_ARGUMENT_32_BITS)
  {
    size_t len11 = (size_t)4U;
    read_u32(ar);
    return len1 + len11;
  }
  else if (get_bitfield_gen8(v, 0U, 5U) == ADDITIONAL_INFO_LONG_ARGUMENT_64_BITS)
  {
    size_t len11 = (size_t)8U;
    read_u64(ar);
    return len1 + len11;
  }
  else
  {
    size_t len11 = (size_t)0U;
    return len1 + len11;
  }
}

static size_t jump_leaf(uint8_t *a)
{
  static_assert(UINT64_MAX <= SIZE_MAX);
  size_t len1 = (size_t)1U;
  uint8_t v = read_u8(a);
  uint8_t *ar = a + len1;
  if (get_bitfield_gen8(v, 0U, 5U) == ADDITIONAL_INFO_LONG_ARGUMENT_8_BITS)
    if (get_bitfield_gen8(v, 5U, 8U) == CBOR_MAJOR_TYPE_SIMPLE_VALUE)
    {
      size_t len11 = (size_t)1U;
      uint8_t v1 = read_u8(ar);
      size_t len = len1 + len11;
      size_t res;
      if
      (
        get_bitfield_gen8(v,
          5U,
          8U)
        == CBOR_MAJOR_TYPE_BYTE_STRING
        || get_bitfield_gen8(v, 5U, 8U) == CBOR_MAJOR_TYPE_TEXT_STRING
      )
        res = (size_t)(uint64_t)v1;
      else
        res = (size_t)0U;
      size_t s2 = res;
      return len + s2;
    }
    else
    {
      size_t len11 = (size_t)1U;
      uint8_t v1 = read_u8(ar);
      size_t len = len1 + len11;
      size_t res;
      if
      (
        get_bitfield_gen8(v,
          5U,
          8U)
        == CBOR_MAJOR_TYPE_BYTE_STRING
        || get_bitfield_gen8(v, 5U, 8U) == CBOR_MAJOR_TYPE_TEXT_STRING
      )
        res = (size_t)(uint64_t)v1;
      else
        res = (size_t)0U;
      size_t s2 = res;
      return len + s2;
    }
  else if (get_bitfield_gen8(v, 0U, 5U) == ADDITIONAL_INFO_LONG_ARGUMENT_16_BITS)
  {
    size_t len11 = (size_t)2U;
    uint16_t v1 = read_u16(ar);
    size_t len = len1 + len11;
    size_t res;
    if
    (
      get_bitfield_gen8(v,
        5U,
        8U)
      == CBOR_MAJOR_TYPE_BYTE_STRING
      || get_bitfield_gen8(v, 5U, 8U) == CBOR_MAJOR_TYPE_TEXT_STRING
    )
      res = (size_t)(uint64_t)v1;
    else
      res = (size_t)0U;
    size_t s2 = res;
    return len + s2;
  }
  else if (get_bitfield_gen8(v, 0U, 5U) == ADDITIONAL_INFO_LONG_ARGUMENT_32_BITS)
  {
    size_t len11 = (size_t)4U;
    uint32_t v1 = read_u32(ar);
    size_t len = len1 + len11;
    size_t res;
    if
    (
      get_bitfield_gen8(v,
        5U,
        8U)
      == CBOR_MAJOR_TYPE_BYTE_STRING
      || get_bitfield_gen8(v, 5U, 8U) == CBOR_MAJOR_TYPE_TEXT_STRING
    )
      res = (size_t)(uint64_t)v1;
    else
      res = (size_t)0U;
    size_t s2 = res;
    return len + s2;
  }
  else if (get_bitfield_gen8(v, 0U, 5U) == ADDITIONAL_INFO_LONG_ARGUMENT_64_BITS)
  {
    size_t len11 = (size_t)8U;
    uint64_t v1 = read_u64(ar);
    size_t len = len1 + len11;
    size_t res;
    if
    (
      get_bitfield_gen8(v,
        5U,
        8U)
      == CBOR_MAJOR_TYPE_BYTE_STRING
      || get_bitfield_gen8(v, 5U, 8U) == CBOR_MAJOR_TYPE_TEXT_STRING
    )
      res = (size_t)v1;
    else
      res = (size_t)0U;
    size_t s2 = res;
    return len + s2;
  }
  else
  {
    size_t len11 = (size_t)0U;
    size_t len = len1 + len11;
    size_t res0;
    if
    (
      get_bitfield_gen8(v,
        5U,
        8U)
      == CBOR_MAJOR_TYPE_BYTE_STRING
      || get_bitfield_gen8(v, 5U, 8U) == CBOR_MAJOR_TYPE_TEXT_STRING
    )
    {
      size_t res = (size_t)(uint64_t)get_bitfield_gen8(v, 0U, 5U);
      size_t res1 = res;
      size_t res2 = res1;
      res0 = res2;
    }
    else
      res0 = (size_t)0U;
    size_t s2 = res0;
    return len + s2;
  }
}

static uint8_t read_header_major_type(uint8_t *a)
{
  uint8_t v = read_u8(a);
  uint8_t res = get_bitfield_gen8(v, 5U, 8U);
  return res;
}

static uint64_t read_header_argument_as_uint64(uint8_t *a)
{
  size_t len1 = (size_t)1U;
  uint8_t v = read_u8(a);
  uint8_t *ar = a + len1;
  if (get_bitfield_gen8(v, 0U, 5U) == ADDITIONAL_INFO_LONG_ARGUMENT_8_BITS)
    if (get_bitfield_gen8(v, 5U, 8U) == CBOR_MAJOR_TYPE_SIMPLE_VALUE)
    {
      uint8_t v1 = read_u8(ar);
      return (uint64_t)v1;
    }
    else
    {
      uint8_t v1 = read_u8(ar);
      return (uint64_t)v1;
    }
  else if (get_bitfield_gen8(v, 0U, 5U) == ADDITIONAL_INFO_LONG_ARGUMENT_16_BITS)
  {
    uint16_t v1 = read_u16(ar);
    return (uint64_t)v1;
  }
  else if (get_bitfield_gen8(v, 0U, 5U) == ADDITIONAL_INFO_LONG_ARGUMENT_32_BITS)
  {
    uint32_t v1 = read_u32(ar);
    return (uint64_t)v1;
  }
  else if (get_bitfield_gen8(v, 0U, 5U) == ADDITIONAL_INFO_LONG_ARGUMENT_64_BITS)
  {
    uint64_t v1 = read_u64(ar);
    return v1;
  }
  else
    return (uint64_t)get_bitfield_gen8(v, 0U, 5U);
}

static uint64_t read_argument_as_uint64_(uint8_t *a)
{
  uint64_t res = read_header_argument_as_uint64(a);
  return res;
}

static size_t count_remaining_data_items(uint8_t *a, size_t bound, uint32_t *perr)
{
  static_assert(UINT64_MAX <= SIZE_MAX);
  uint8_t major_type = read_header_major_type(a);
  if (major_type == CBOR_MAJOR_TYPE_ARRAY)
  {
    uint64_t arg = read_header_argument_as_uint64(a);
    return (size_t)arg;
  }
  else if (major_type == CBOR_MAJOR_TYPE_MAP)
  {
    uint64_t arg = read_header_argument_as_uint64(a);
    size_t half = (size_t)arg;
    bool overflow;
    if (half > bound)
      overflow = true;
    else
      overflow = bound - half < half;
    if (overflow)
    {
      *perr = VALIDATOR_ERROR_NOT_ENOUGH_DATA;
      return (size_t)0U;
    }
    else
      return half + half;
  }
  else if (major_type == CBOR_MAJOR_TYPE_TAGGED)
    return (size_t)1U;
  else
    return (size_t)0U;
}

static size_t validate_raw_data_item_(uint8_t *a0, size_t len, uint32_t *perr)
{
  size_t r = (size_t)0U;
  size_t r1 = (size_t)1U;
  bool r2 = true;
  while (r2)
  {
    size_t n = r1;
    if (n == (size_t)0U)
      r2 = false;
    else
    {
      size_t consumed = r;
      size_t len1 = len - consumed;
      if (n > len1)
      {
        *perr = validate_recursive_error_not_enough_data;
        r2 = false;
      }
      else
      {
        uint8_t *a = a0 + consumed;
        uint32_t r3 = 0U;
        size_t len11;
        if ((size_t)1U <= len1)
          len11 = (size_t)1U;
        else
        {
          r3 = VALIDATOR_ERROR_NOT_ENOUGH_DATA;
          len11 = (size_t)0U;
        }
        uint32_t e = r3;
        size_t v0;
        if (e == 0U)
        {
          uint8_t v = read_u8(a);
          bool ite;
          if (get_bitfield_gen8(v, 5U, 8U) == CBOR_MAJOR_TYPE_SIMPLE_VALUE)
            ite = get_bitfield_gen8(v, 0U, 5U) <= ADDITIONAL_INFO_LONG_ARGUMENT_8_BITS;
          else
            ite = true;
          if (!(ite && get_bitfield_gen8(v, 0U, 5U) < ADDITIONAL_INFO_UNASSIGNED_MIN))
          {
            *perr = VALIDATOR_ERROR_CONSTRAINT_FAILED;
            v0 = (size_t)0U;
          }
          else
          {
            uint8_t *ar = a + len11;
            size_t len2 = len1 - len11;
            if (get_bitfield_gen8(v, 0U, 5U) == ADDITIONAL_INFO_LONG_ARGUMENT_8_BITS)
              if (get_bitfield_gen8(v, 5U, 8U) == CBOR_MAJOR_TYPE_SIMPLE_VALUE)
              {
                uint32_t r4 = 0U;
                size_t len12;
                if ((size_t)1U <= len2)
                  len12 = (size_t)1U;
                else
                {
                  r4 = VALIDATOR_ERROR_NOT_ENOUGH_DATA;
                  len12 = (size_t)0U;
                }
                uint32_t e1 = r4;
                size_t v10;
                if (e1 == 0U)
                {
                  uint8_t v1 = read_u8(ar);
                  if (!(MIN_SIMPLE_VALUE_LONG_ARGUMENT <= v1))
                  {
                    *perr = VALIDATOR_ERROR_CONSTRAINT_FAILED;
                    v10 = (size_t)0U;
                  }
                  else
                  {
                    size_t len3 = len11 + len12;
                    size_t res0;
                    if
                    (
                      get_bitfield_gen8(v,
                        5U,
                        8U)
                      == CBOR_MAJOR_TYPE_BYTE_STRING
                      || get_bitfield_gen8(v, 5U, 8U) == CBOR_MAJOR_TYPE_TEXT_STRING
                    )
                    {
                      size_t res;
                      if ((size_t)(uint64_t)v1 <= len1 - len3)
                        res = (size_t)(uint64_t)v1;
                      else
                      {
                        *perr = VALIDATOR_ERROR_NOT_ENOUGH_DATA;
                        res = (size_t)0U;
                      }
                      size_t res1 = res;
                      size_t res2 = res1;
                      res0 = res2;
                    }
                    else
                    {
                      size_t res;
                      if ((size_t)0U <= len1 - len3)
                        res = (size_t)0U;
                      else
                      {
                        *perr = VALIDATOR_ERROR_NOT_ENOUGH_DATA;
                        res = (size_t)0U;
                      }
                      size_t res1 = res;
                      size_t res2 = res1;
                      res0 = res2;
                    }
                    size_t s2 = res0;
                    v10 = len3 + s2;
                  }
                }
                else
                {
                  *perr = e1;
                  v10 = (size_t)0U;
                }
                v0 = v10;
              }
              else
              {
                uint32_t r4 = 0U;
                size_t len12;
                if ((size_t)1U <= len2)
                  len12 = (size_t)1U;
                else
                {
                  r4 = VALIDATOR_ERROR_NOT_ENOUGH_DATA;
                  len12 = (size_t)0U;
                }
                uint32_t e1 = r4;
                size_t v10;
                if (e1 == 0U)
                {
                  uint8_t v1 = read_u8(ar);
                  if (!(MIN_DETERMINISTIC_UINT8 <= v1))
                  {
                    *perr = VALIDATOR_ERROR_CONSTRAINT_FAILED;
                    v10 = (size_t)0U;
                  }
                  else
                  {
                    size_t len3 = len11 + len12;
                    size_t res0;
                    if
                    (
                      get_bitfield_gen8(v,
                        5U,
                        8U)
                      == CBOR_MAJOR_TYPE_BYTE_STRING
                      || get_bitfield_gen8(v, 5U, 8U) == CBOR_MAJOR_TYPE_TEXT_STRING
                    )
                    {
                      size_t res;
                      if ((size_t)(uint64_t)v1 <= len1 - len3)
                        res = (size_t)(uint64_t)v1;
                      else
                      {
                        *perr = VALIDATOR_ERROR_NOT_ENOUGH_DATA;
                        res = (size_t)0U;
                      }
                      size_t res1 = res;
                      size_t res2 = res1;
                      res0 = res2;
                    }
                    else
                    {
                      size_t res;
                      if ((size_t)0U <= len1 - len3)
                        res = (size_t)0U;
                      else
                      {
                        *perr = VALIDATOR_ERROR_NOT_ENOUGH_DATA;
                        res = (size_t)0U;
                      }
                      size_t res1 = res;
                      size_t res2 = res1;
                      res0 = res2;
                    }
                    size_t s2 = res0;
                    v10 = len3 + s2;
                  }
                }
                else
                {
                  *perr = e1;
                  v10 = (size_t)0U;
                }
                v0 = v10;
              }
            else if (get_bitfield_gen8(v, 0U, 5U) == ADDITIONAL_INFO_LONG_ARGUMENT_16_BITS)
            {
              uint32_t r4 = 0U;
              size_t len12;
              if ((size_t)2U <= len2)
                len12 = (size_t)2U;
              else
              {
                r4 = VALIDATOR_ERROR_NOT_ENOUGH_DATA;
                len12 = (size_t)0U;
              }
              uint32_t e1 = r4;
              size_t v10;
              if (e1 == 0U)
              {
                uint16_t v1 = read_u16(ar);
                if (!(MIN_DETERMINISTIC_UINT16 <= v1))
                {
                  *perr = VALIDATOR_ERROR_CONSTRAINT_FAILED;
                  v10 = (size_t)0U;
                }
                else
                {
                  size_t len3 = len11 + len12;
                  size_t res0;
                  if
                  (
                    get_bitfield_gen8(v,
                      5U,
                      8U)
                    == CBOR_MAJOR_TYPE_BYTE_STRING
                    || get_bitfield_gen8(v, 5U, 8U) == CBOR_MAJOR_TYPE_TEXT_STRING
                  )
                  {
                    size_t res;
                    if ((size_t)(uint64_t)v1 <= len1 - len3)
                      res = (size_t)(uint64_t)v1;
                    else
                    {
                      *perr = VALIDATOR_ERROR_NOT_ENOUGH_DATA;
                      res = (size_t)0U;
                    }
                    size_t res1 = res;
                    size_t res2 = res1;
                    res0 = res2;
                  }
                  else
                  {
                    size_t res;
                    if ((size_t)0U <= len1 - len3)
                      res = (size_t)0U;
                    else
                    {
                      *perr = VALIDATOR_ERROR_NOT_ENOUGH_DATA;
                      res = (size_t)0U;
                    }
                    size_t res1 = res;
                    size_t res2 = res1;
                    res0 = res2;
                  }
                  size_t s2 = res0;
                  v10 = len3 + s2;
                }
              }
              else
              {
                *perr = e1;
                v10 = (size_t)0U;
              }
              v0 = v10;
            }
            else if (get_bitfield_gen8(v, 0U, 5U) == ADDITIONAL_INFO_LONG_ARGUMENT_32_BITS)
            {
              uint32_t r4 = 0U;
              size_t len12;
              if ((size_t)4U <= len2)
                len12 = (size_t)4U;
              else
              {
                r4 = VALIDATOR_ERROR_NOT_ENOUGH_DATA;
                len12 = (size_t)0U;
              }
              uint32_t e1 = r4;
              size_t v10;
              if (e1 == 0U)
              {
                uint32_t v1 = read_u32(ar);
                if (!(MIN_DETERMINISTIC_UINT32 <= v1))
                {
                  *perr = VALIDATOR_ERROR_CONSTRAINT_FAILED;
                  v10 = (size_t)0U;
                }
                else
                {
                  size_t len3 = len11 + len12;
                  size_t res0;
                  if
                  (
                    get_bitfield_gen8(v,
                      5U,
                      8U)
                    == CBOR_MAJOR_TYPE_BYTE_STRING
                    || get_bitfield_gen8(v, 5U, 8U) == CBOR_MAJOR_TYPE_TEXT_STRING
                  )
                  {
                    size_t res;
                    if ((size_t)(uint64_t)v1 <= len1 - len3)
                      res = (size_t)(uint64_t)v1;
                    else
                    {
                      *perr = VALIDATOR_ERROR_NOT_ENOUGH_DATA;
                      res = (size_t)0U;
                    }
                    size_t res1 = res;
                    size_t res2 = res1;
                    res0 = res2;
                  }
                  else
                  {
                    size_t res;
                    if ((size_t)0U <= len1 - len3)
                      res = (size_t)0U;
                    else
                    {
                      *perr = VALIDATOR_ERROR_NOT_ENOUGH_DATA;
                      res = (size_t)0U;
                    }
                    size_t res1 = res;
                    size_t res2 = res1;
                    res0 = res2;
                  }
                  size_t s2 = res0;
                  v10 = len3 + s2;
                }
              }
              else
              {
                *perr = e1;
                v10 = (size_t)0U;
              }
              v0 = v10;
            }
            else if (get_bitfield_gen8(v, 0U, 5U) == ADDITIONAL_INFO_LONG_ARGUMENT_64_BITS)
            {
              uint32_t r4 = 0U;
              size_t len12;
              if ((size_t)8U <= len2)
                len12 = (size_t)8U;
              else
              {
                r4 = VALIDATOR_ERROR_NOT_ENOUGH_DATA;
                len12 = (size_t)0U;
              }
              uint32_t e1 = r4;
              size_t v10;
              if (e1 == 0U)
              {
                uint64_t v1 = read_u64(ar);
                if (!(MIN_DETERMINISTIC_UINT64 <= v1))
                {
                  *perr = VALIDATOR_ERROR_CONSTRAINT_FAILED;
                  v10 = (size_t)0U;
                }
                else
                {
                  size_t len3 = len11 + len12;
                  size_t res0;
                  if
                  (
                    get_bitfield_gen8(v,
                      5U,
                      8U)
                    == CBOR_MAJOR_TYPE_BYTE_STRING
                    || get_bitfield_gen8(v, 5U, 8U) == CBOR_MAJOR_TYPE_TEXT_STRING
                  )
                  {
                    size_t res;
                    if ((size_t)v1 <= len1 - len3)
                      res = (size_t)v1;
                    else
                    {
                      *perr = VALIDATOR_ERROR_NOT_ENOUGH_DATA;
                      res = (size_t)0U;
                    }
                    size_t res1 = res;
                    size_t res2 = res1;
                    res0 = res2;
                  }
                  else
                  {
                    size_t res;
                    if ((size_t)0U <= len1 - len3)
                      res = (size_t)0U;
                    else
                    {
                      *perr = VALIDATOR_ERROR_NOT_ENOUGH_DATA;
                      res = (size_t)0U;
                    }
                    size_t res1 = res;
                    size_t res2 = res1;
                    res0 = res2;
                  }
                  size_t s2 = res0;
                  v10 = len3 + s2;
                }
              }
              else
              {
                *perr = e1;
                v10 = (size_t)0U;
              }
              v0 = v10;
            }
            else
            {
              uint32_t r4 = 0U;
              size_t len12;
              if ((size_t)0U <= len2)
                len12 = (size_t)0U;
              else
              {
                r4 = VALIDATOR_ERROR_NOT_ENOUGH_DATA;
                len12 = (size_t)0U;
              }
              uint32_t e1 = r4;
              size_t v1;
              if (e1 == 0U)
              {
                size_t len3 = len11 + len12;
                size_t res0;
                if
                (
                  get_bitfield_gen8(v,
                    5U,
                    8U)
                  == CBOR_MAJOR_TYPE_BYTE_STRING
                  || get_bitfield_gen8(v, 5U, 8U) == CBOR_MAJOR_TYPE_TEXT_STRING
                )
                {
                  size_t res;
                  if ((size_t)(uint64_t)get_bitfield_gen8(v, 0U, 5U) <= len1 - len3)
                    res = (size_t)(uint64_t)get_bitfield_gen8(v, 0U, 5U);
                  else
                  {
                    *perr = VALIDATOR_ERROR_NOT_ENOUGH_DATA;
                    res = (size_t)0U;
                  }
                  size_t res1 = res;
                  size_t res2 = res1;
                  res0 = res2;
                }
                else
                {
                  size_t res;
                  if ((size_t)0U <= len1 - len3)
                    res = (size_t)0U;
                  else
                  {
                    *perr = VALIDATOR_ERROR_NOT_ENOUGH_DATA;
                    res = (size_t)0U;
                  }
                  size_t res1 = res;
                  size_t res2 = res1;
                  res0 = res2;
                }
                size_t s2 = res0;
                v1 = len3 + s2;
              }
              else
              {
                *perr = e1;
                v1 = (size_t)0U;
              }
              v0 = v1;
            }
          }
        }
        else
        {
          *perr = e;
          v0 = (size_t)0U;
        }
        size_t consumed1 = v0;
        uint32_t err = *perr;
        if (err != 0U)
          r2 = false;
        else
        {
          size_t rem = len1 - n;
          size_t n_ = count_remaining_data_items(a, rem, perr);
          uint32_t err1 = *perr;
          bool overflow;
          if (err1 == 0U)
            overflow = n_ > rem;
          else
            overflow = true;
          if (overflow)
          {
            if (err1 == 0U)
              *perr = validate_recursive_error_not_enough_data;
            r2 = false;
          }
          else
          {
            r1 = n_ + n - (size_t)1U;
            r = consumed + consumed1;
          }
        }
      }
    }
  }
  size_t v = r;
  size_t v0 = v;
  size_t v1 = v0;
  size_t res = v1;
  return res;
}

static size_t jump_count_remaining_data_items(uint8_t *a)
{
  static_assert(UINT64_MAX <= SIZE_MAX);
  uint8_t major_type = read_header_major_type(a);
  if (major_type == CBOR_MAJOR_TYPE_ARRAY)
  {
    uint64_t arg = read_header_argument_as_uint64(a);
    return (size_t)arg;
  }
  else if (major_type == CBOR_MAJOR_TYPE_MAP)
  {
    uint64_t arg = read_header_argument_as_uint64(a);
    size_t half = (size_t)arg;
    return half + half;
  }
  else if (major_type == CBOR_MAJOR_TYPE_TAGGED)
    return (size_t)1U;
  else
    return (size_t)0U;
}

static size_t jump_raw_data_item_(uint8_t *a0)
{
  size_t r = (size_t)0U;
  size_t r1 = (size_t)1U;
  size_t n0 = r1;
  bool cond = n0 != (size_t)0U;
  while (cond)
  {
    size_t n = r1;
    size_t consumed = r;
    uint8_t *a = a0 + consumed;
    size_t consumed1 = jump_leaf(a);
    size_t n_ = jump_count_remaining_data_items(a);
    size_t new_count = n_ + n - (size_t)1U;
    r1 = new_count;
    r = consumed + consumed1;
    size_t n0 = r1;
    cond = n0 != (size_t)0U;
  }
  size_t v = r;
  size_t v0 = v;
  size_t res = v0;
  size_t res0 = res;
  return res0;
}

static uint8_t *focus_tagged(uint8_t *a)
{
  size_t res = jump_header_(a);
  size_t sz = res;
  uint8_t *res0 = a + sz;
  uint8_t *a_ = res0;
  return a_;
}

static void write_initial_byte(uint8_t t, uint8_t x, uint8_t *a)
{
  write_u8(set_bitfield_gen8(set_bitfield_gen8(0U, 0U, 5U, x), 5U, 8U, t), a);
  size_t res = (size_t)1U;
  KRML_MAYBE_UNUSED_VAR(res);
}

static size_t size_comp_simple_value(uint8_t x, size_t sz, bool *perr)
{
  size_t sz10;
  if (x <= MAX_SIMPLE_VALUE_ADDITIONAL_INFO)
  {
    size_t sz1;
    if (sz < (size_t)1U)
    {
      *perr = true;
      sz1 = sz;
    }
    else
      sz1 = sz - (size_t)1U;
    bool err = *perr;
    if (err)
      sz10 = sz1;
    else
    {
      size_t sz_;
      if (sz1 < (size_t)0U)
      {
        *perr = true;
        sz_ = sz1;
      }
      else
        sz_ = sz1 - (size_t)0U;
      size_t res = sz_;
      size_t res0 = res;
      size_t sz2 = res0;
      sz10 = sz2;
    }
  }
  else
  {
    size_t sz1;
    if (sz < (size_t)1U)
    {
      *perr = true;
      sz1 = sz;
    }
    else
      sz1 = sz - (size_t)1U;
    bool err = *perr;
    if (err)
      sz10 = sz1;
    else
    {
      size_t res;
      if (sz1 < (size_t)1U)
      {
        *perr = true;
        res = sz1;
      }
      else
        res = sz1 - (size_t)1U;
      size_t sz_ = res;
      size_t res0 = sz_;
      size_t res1 = res0;
      size_t sz2 = res1;
      sz10 = sz2;
    }
  }
  bool err = *perr;
  size_t sz_;
  if (err)
    sz_ = sz10;
  else
  {
    size_t res;
    if (sz10 < (size_t)0U)
    {
      *perr = true;
      res = sz10;
    }
    else
      res = sz10 - (size_t)0U;
    size_t sz2 = res;
    sz_ = sz2;
  }
  size_t res = sz_;
  return res;
}

static uint8_t *l2r_write_simple_value(uint8_t x, LowParse_SteelST_L2ROutput_t out)
{
  uint8_t *res0;
  if (x <= MAX_SIMPLE_VALUE_ADDITIONAL_INFO)
  {
    uint8_t *res = *out.ptr;
    size_t xlen0 = *out.len;
    size_t xlen_ = xlen0 - (size_t)1U;
    uint8_t *xptr_ = res + (size_t)1U;
    *out.ptr = xptr_;
    *out.len = xlen_;
    uint8_t *res2 = res;
    write_initial_byte(CBOR_MAJOR_TYPE_SIMPLE_VALUE, x, res2);
    uint8_t *res3 = res2;
    uint8_t *res1 = *out.ptr;
    size_t xlen = *out.len;
    size_t xlen_0 = xlen - (size_t)0U;
    uint8_t *xptr_0 = res1;
    *out.ptr = xptr_0;
    *out.len = xlen_0;
    uint8_t *res10 = res1;
    uint8_t *res11 = res10;
    uint8_t *res12 = res11;
    uint8_t *res13 = res12;
    KRML_MAYBE_UNUSED_VAR(res13);
    res0 = res3;
  }
  else
  {
    uint8_t *res = *out.ptr;
    size_t xlen0 = *out.len;
    size_t xlen_ = xlen0 - (size_t)1U;
    uint8_t *xptr_ = res + (size_t)1U;
    *out.ptr = xptr_;
    *out.len = xlen_;
    uint8_t *res2 = res;
    write_initial_byte(CBOR_MAJOR_TYPE_SIMPLE_VALUE, ADDITIONAL_INFO_LONG_ARGUMENT_8_BITS, res2);
    uint8_t *res3 = res2;
    uint8_t *res1 = *out.ptr;
    size_t xlen = *out.len;
    size_t xlen_0 = xlen - (size_t)1U;
    uint8_t *xptr_0 = res1 + (size_t)1U;
    *out.ptr = xptr_0;
    *out.len = xlen_0;
    uint8_t *res10 = res1;
    write_u8(x, res10);
    uint8_t *res11 = res10;
    uint8_t *res12 = res11;
    uint8_t *res13 = res12;
    uint8_t *res14 = res13;
    KRML_MAYBE_UNUSED_VAR(res14);
    res0 = res3;
  }
  uint8_t *res1 = *out.ptr;
  size_t xlen = *out.len;
  size_t xlen_ = xlen - (size_t)0U;
  uint8_t *xptr_ = res1;
  *out.ptr = xptr_;
  *out.len = xlen_;
  uint8_t *res10 = res1;
  uint8_t *res11 = res10;
  KRML_MAYBE_UNUSED_VAR(res11);
  uint8_t *res = res0;
  uint8_t *res2 = res;
  return res2;
}

static size_t size_comp_uint64_header(uint64_t x, size_t sz, bool *perr)
{
  if (x < MIN_DETERMINISTIC_UINT8_AS_UINT64)
  {
    size_t sz1;
    if (sz < (size_t)1U)
    {
      *perr = true;
      sz1 = sz;
    }
    else
      sz1 = sz - (size_t)1U;
    bool err = *perr;
    if (err)
      return sz1;
    else
    {
      size_t sz_;
      if (sz1 < (size_t)0U)
      {
        *perr = true;
        sz_ = sz1;
      }
      else
        sz_ = sz1 - (size_t)0U;
      size_t res = sz_;
      size_t res0 = res;
      size_t sz2 = res0;
      return sz2;
    }
  }
  else if (x < MIN_DETERMINISTIC_UINT16_AS_UINT64)
  {
    size_t sz1;
    if (sz < (size_t)1U)
    {
      *perr = true;
      sz1 = sz;
    }
    else
      sz1 = sz - (size_t)1U;
    bool err = *perr;
    if (err)
      return sz1;
    else
    {
      size_t res;
      if (sz1 < (size_t)1U)
      {
        *perr = true;
        res = sz1;
      }
      else
        res = sz1 - (size_t)1U;
      size_t sz_ = res;
      size_t res0 = sz_;
      size_t res1 = res0;
      size_t sz2 = res1;
      return sz2;
    }
  }
  else if (x < MIN_DETERMINISTIC_UINT32_AS_UINT64)
  {
    size_t sz1;
    if (sz < (size_t)1U)
    {
      *perr = true;
      sz1 = sz;
    }
    else
      sz1 = sz - (size_t)1U;
    bool err = *perr;
    if (err)
      return sz1;
    else
    {
      size_t res;
      if (sz1 < (size_t)2U)
      {
        *perr = true;
        res = sz1;
      }
      else
        res = sz1 - (size_t)2U;
      size_t sz_ = res;
      size_t res0 = sz_;
      size_t res1 = res0;
      size_t sz2 = res1;
      return sz2;
    }
  }
  else if (x < MIN_DETERMINISTIC_UINT64)
  {
    size_t sz1;
    if (sz < (size_t)1U)
    {
      *perr = true;
      sz1 = sz;
    }
    else
      sz1 = sz - (size_t)1U;
    bool err = *perr;
    if (err)
      return sz1;
    else
    {
      size_t res;
      if (sz1 < (size_t)4U)
      {
        *perr = true;
        res = sz1;
      }
      else
        res = sz1 - (size_t)4U;
      size_t sz_ = res;
      size_t res0 = sz_;
      size_t res1 = res0;
      size_t sz2 = res1;
      return sz2;
    }
  }
  else
  {
    size_t sz1;
    if (sz < (size_t)1U)
    {
      *perr = true;
      sz1 = sz;
    }
    else
      sz1 = sz - (size_t)1U;
    bool err = *perr;
    if (err)
      return sz1;
    else
    {
      size_t res;
      if (sz1 < (size_t)8U)
      {
        *perr = true;
        res = sz1;
      }
      else
        res = sz1 - (size_t)8U;
      size_t sz_ = res;
      size_t res0 = sz_;
      size_t res1 = res0;
      size_t sz2 = res1;
      return sz2;
    }
  }
}

static size_t size_comp_int64(uint64_t x, size_t sz, bool *perr)
{
  size_t sz1 = size_comp_uint64_header(x, sz, perr);
  bool err = *perr;
  size_t sz_;
  if (err)
    sz_ = sz1;
  else
  {
    size_t res;
    if (sz1 < (size_t)0U)
    {
      *perr = true;
      res = sz1;
    }
    else
      res = sz1 - (size_t)0U;
    size_t sz2 = res;
    sz_ = sz2;
  }
  size_t res = sz_;
  return res;
}

static size_t size_comp_string(uint64_t x, size_t sz, bool *perr)
{
  size_t sz1 = size_comp_uint64_header(x, sz, perr);
  bool err = *perr;
  size_t sz_;
  if (err)
    sz_ = sz1;
  else
  {
    size_t res;
    if (sz1 < (size_t)x)
    {
      *perr = true;
      res = sz1;
    }
    else
      res = sz1 - (size_t)x;
    size_t res0 = res;
    size_t sz2 = res0;
    sz_ = sz2;
  }
  size_t res = sz_;
  return res;
}

static uint8_t
*l2r_write_uint64_header(uint8_t ty, uint64_t x, LowParse_SteelST_L2ROutput_t out)
{
  if (x < MIN_DETERMINISTIC_UINT8_AS_UINT64)
  {
    uint8_t *res = *out.ptr;
    size_t xlen0 = *out.len;
    size_t xlen_ = xlen0 - (size_t)1U;
    uint8_t *xptr_ = res + (size_t)1U;
    *out.ptr = xptr_;
    *out.len = xlen_;
    uint8_t *res0 = res;
    write_initial_byte(ty, (uint8_t)x, res0);
    uint8_t *res2 = res0;
    uint8_t *res1 = *out.ptr;
    size_t xlen = *out.len;
    size_t xlen_0 = xlen - (size_t)0U;
    uint8_t *xptr_0 = res1;
    *out.ptr = xptr_0;
    *out.len = xlen_0;
    uint8_t *res10 = res1;
    uint8_t *res11 = res10;
    uint8_t *res12 = res11;
    uint8_t *res13 = res12;
    KRML_MAYBE_UNUSED_VAR(res13);
    return res2;
  }
  else if (x < MIN_DETERMINISTIC_UINT16_AS_UINT64)
  {
    uint8_t *res = *out.ptr;
    size_t xlen0 = *out.len;
    size_t xlen_ = xlen0 - (size_t)1U;
    uint8_t *xptr_ = res + (size_t)1U;
    *out.ptr = xptr_;
    *out.len = xlen_;
    uint8_t *res0 = res;
    write_initial_byte(ty, ADDITIONAL_INFO_LONG_ARGUMENT_8_BITS, res0);
    uint8_t *res2 = res0;
    uint8_t *res1 = *out.ptr;
    size_t xlen = *out.len;
    size_t xlen_0 = xlen - (size_t)1U;
    uint8_t *xptr_0 = res1 + (size_t)1U;
    *out.ptr = xptr_0;
    *out.len = xlen_0;
    uint8_t *res10 = res1;
    write_u8((uint8_t)x, res10);
    uint8_t *res11 = res10;
    uint8_t *res12 = res11;
    uint8_t *res13 = res12;
    uint8_t *res14 = res13;
    KRML_MAYBE_UNUSED_VAR(res14);
    return res2;
  }
  else if (x < MIN_DETERMINISTIC_UINT32_AS_UINT64)
  {
    uint8_t *res = *out.ptr;
    size_t xlen0 = *out.len;
    size_t xlen_ = xlen0 - (size_t)1U;
    uint8_t *xptr_ = res + (size_t)1U;
    *out.ptr = xptr_;
    *out.len = xlen_;
    uint8_t *res0 = res;
    write_initial_byte(ty, ADDITIONAL_INFO_LONG_ARGUMENT_16_BITS, res0);
    uint8_t *res2 = res0;
    uint8_t *res1 = *out.ptr;
    size_t xlen = *out.len;
    size_t xlen_0 = xlen - (size_t)2U;
    uint8_t *xptr_0 = res1 + (size_t)2U;
    *out.ptr = xptr_0;
    *out.len = xlen_0;
    uint8_t *res10 = res1;
    write_u16((uint16_t)x, res10);
    uint8_t *res11 = res10;
    uint8_t *res12 = res11;
    uint8_t *res13 = res12;
    uint8_t *res14 = res13;
    KRML_MAYBE_UNUSED_VAR(res14);
    return res2;
  }
  else if (x < MIN_DETERMINISTIC_UINT64)
  {
    uint8_t *res = *out.ptr;
    size_t xlen0 = *out.len;
    size_t xlen_ = xlen0 - (size_t)1U;
    uint8_t *xptr_ = res + (size_t)1U;
    *out.ptr = xptr_;
    *out.len = xlen_;
    uint8_t *res0 = res;
    write_initial_byte(ty, ADDITIONAL_INFO_LONG_ARGUMENT_32_BITS, res0);
    uint8_t *res2 = res0;
    uint8_t *res1 = *out.ptr;
    size_t xlen = *out.len;
    size_t xlen_0 = xlen - (size_t)4U;
    uint8_t *xptr_0 = res1 + (size_t)4U;
    *out.ptr = xptr_0;
    *out.len = xlen_0;
    uint8_t *res10 = res1;
    write_u32((uint32_t)x, res10);
    uint8_t *res11 = res10;
    uint8_t *res12 = res11;
    uint8_t *res13 = res12;
    uint8_t *res14 = res13;
    KRML_MAYBE_UNUSED_VAR(res14);
    return res2;
  }
  else
  {
    uint8_t *res = *out.ptr;
    size_t xlen0 = *out.len;
    size_t xlen_ = xlen0 - (size_t)1U;
    uint8_t *xptr_ = res + (size_t)1U;
    *out.ptr = xptr_;
    *out.len = xlen_;
    uint8_t *res0 = res;
    write_initial_byte(ty, ADDITIONAL_INFO_LONG_ARGUMENT_64_BITS, res0);
    uint8_t *res2 = res0;
    uint8_t *res1 = *out.ptr;
    size_t xlen = *out.len;
    size_t xlen_0 = xlen - (size_t)8U;
    uint8_t *xptr_0 = res1 + (size_t)8U;
    *out.ptr = xptr_0;
    *out.len = xlen_0;
    uint8_t *res10 = res1;
    write_u64(x, res10);
    uint8_t *res11 = res10;
    uint8_t *res12 = res11;
    uint8_t *res13 = res12;
    uint8_t *res14 = res13;
    KRML_MAYBE_UNUSED_VAR(res14);
    return res2;
  }
}

static uint8_t *l2r_write_int64(uint8_t ty, uint64_t x, LowParse_SteelST_L2ROutput_t out)
{
  uint8_t *res = l2r_write_uint64_header(ty, x, out);
  uint8_t *res1 = *out.ptr;
  size_t xlen = *out.len;
  size_t xlen_ = xlen - (size_t)0U;
  uint8_t *xptr_ = res1;
  *out.ptr = xptr_;
  *out.len = xlen_;
  uint8_t *res10 = res1;
  uint8_t *res11 = res10;
  KRML_MAYBE_UNUSED_VAR(res11);
  uint8_t *res0 = res;
  uint8_t *res2 = res0;
  return res2;
}

static uint8_t *focus_map_with_ghost_length(uint8_t *a)
{
  size_t res = jump_header_(a);
  size_t sz = res;
  uint8_t *res0 = a + sz;
  uint8_t *a_ = res0;
  return a_;
}

static bool deterministically_encoded_cbor_map_key_order_impl(uint8_t *a1, uint8_t *a2)
{
  size_t res0 = jump_raw_data_item_(a1);
  size_t n1 = res0;
  size_t res1 = jump_raw_data_item_(a2);
  size_t n2 = res1;
  int16_t res2;
  if (n1 == (size_t)0U)
    if (n2 == (size_t)0U)
      res2 = (int16_t)0;
    else
      res2 = (int16_t)-1;
  else if (n2 == (size_t)0U)
    res2 = (int16_t)1;
  else
  {
    uint8_t *r = a1;
    uint8_t *r1 = a2;
    size_t r2 = n1;
    size_t r3 = n2;
    int16_t r4 = (int16_t)0;
    int16_t res = r4;
    size_t na0 = r2;
    size_t nb0 = r3;
    bool cond = res == (int16_t)0 && na0 > (size_t)0U && nb0 > (size_t)0U;
    while (cond)
    {
      uint8_t *a = r;
      size_t na0 = r2;
      size_t na_ = na0 - (size_t)1U;
      size_t a_sz = (size_t)1U;
      uint8_t *b = r1;
      size_t nb0 = r3;
      size_t nb_ = nb0 - (size_t)1U;
      size_t b_sz = (size_t)1U;
      uint8_t x1 = a[0U];
      uint8_t x2 = b[0U];
      int16_t comp = (int16_t)x1 - (int16_t)x2;
      if (comp != (int16_t)0)
        r4 = comp;
      else
      {
        r2 = na_;
        r3 = nb_;
        if (na_ == (size_t)0U)
        {
          if (nb_ != (size_t)0U)
            r4 = (int16_t)-1;
        }
        else if (nb_ == (size_t)0U)
          r4 = (int16_t)1;
        else
        {
          uint8_t *a_ = a + a_sz;
          r = a_;
          uint8_t *b_ = b + b_sz;
          r1 = b_;
        }
      }
      int16_t res = r4;
      size_t na = r2;
      size_t nb = r3;
      cond = res == (int16_t)0 && na > (size_t)0U && nb > (size_t)0U;
    }
    int16_t v = r4;
    int16_t v0 = v;
    int16_t v1 = v0;
    int16_t v2 = v1;
    int16_t v3 = v2;
    int16_t res0 = v3;
    res2 = res0;
  }
  int16_t comp = res2;
  bool res = comp < (int16_t)0;
  return res;
}

static uint8_t *focus_array_with_ghost_length(uint8_t *a)
{
  size_t res = jump_header_(a);
  size_t sz = res;
  uint8_t *res0 = a + sz;
  uint8_t *a_ = res0;
  return a_;
}

static size_t validate_raw_data_item(uint8_t *a, size_t len, uint32_t *err)
{
  static_assert(UINT64_MAX <= SIZE_MAX);
  return validate_raw_data_item_(a, len, err);
}

static size_t jump_raw_data_item(uint8_t *a)
{
  return jump_raw_data_item_(a);
}

static uint8_t read_major_type(uint8_t *a)
{
  uint8_t res = read_header_major_type(a);
  return res;
}

static uint64_t read_argument_as_uint64(uint8_t *a)
{
  return read_argument_as_uint64_(a);
}

static uint64_t read_int64(uint8_t *a)
{
  return read_argument_as_uint64_(a);
}

static uint8_t read_simple_value(uint8_t *a)
{
  size_t len1 = (size_t)1U;
  uint8_t v = read_u8(a);
  uint8_t *ar = a + len1;
  uint8_t res;
  if (get_bitfield_gen8(v, 0U, 5U) == ADDITIONAL_INFO_LONG_ARGUMENT_8_BITS)
  {
    uint8_t v1 = read_u8(ar);
    res = v1;
  }
  else
    res = get_bitfield_gen8(v, 0U, 5U);
  uint8_t res0 = res;
  return res0;
}

static uint8_t *focus_string(uint8_t *a)
{
  size_t res = jump_header_(a);
  size_t sz = res;
  uint8_t *a_ = a + sz;
  return a_;
}

static cbor_map_entry
cbor_map_iterator_next_serialized(cbor_map_iterator_t i, cbor_map_iterator_t *pi)
{
  uint8_t *a;
  if (i.cbor_map_iterator_payload.tag == CBOR_Map_Iterator_Payload_Serialized)
    a = i.cbor_map_iterator_payload.case_CBOR_Map_Iterator_Payload_Serialized;
  else
    a = KRML_EABORT(uint8_t *, "unreachable (pattern matches are exhaustive in F*)");
  uint64_t len_ = i.cbor_map_iterator_length - 1ULL;
  size_t res0 = jump_raw_data_item(a);
  size_t sz_key = res0;
  uint8_t *ar = a + sz_key;
  size_t res = jump_raw_data_item(ar);
  size_t sz_value = res;
  uint8_t *a_ = ar + sz_value;
  cbor res_key = read_valid_cbor_from_buffer_with_size_strong(a, sz_key);
  cbor res_value = read_valid_cbor_from_buffer_with_size_strong(ar, sz_value);
  cbor_map_entry res1 = cbor_mk_map_entry(res_key, res_value);
  cbor_map_iterator_t
  i_ =
    {
      .cbor_map_iterator_length = len_,
      .cbor_map_iterator_payload = {
        .tag = CBOR_Map_Iterator_Payload_Serialized,
        { .case_CBOR_Map_Iterator_Payload_Serialized = a_ }
      }
    };
  *pi = i_;
  return res1;
}

static cbor_map_entry
cbor_map_iterator_next_map(cbor_map_iterator_t i, cbor_map_iterator_t *pi)
{
  cbor_map_entry *a;
  if (i.cbor_map_iterator_payload.tag == CBOR_Map_Iterator_Payload_Map)
    a = i.cbor_map_iterator_payload.case_CBOR_Map_Iterator_Payload_Map;
  else
    a = KRML_EABORT(cbor_map_entry *, "unreachable (pattern matches are exhaustive in F*)");
  uint64_t len_ = i.cbor_map_iterator_length - 1ULL;
  cbor_map_entry res = a[0U];
  cbor_map_entry *ar_ = a + (size_t)1U;
  cbor_map_iterator_t
  i_ =
    {
      .cbor_map_iterator_length = len_,
      .cbor_map_iterator_payload = {
        .tag = CBOR_Map_Iterator_Payload_Map,
        { .case_CBOR_Map_Iterator_Payload_Map = ar_ }
      }
    };
  *pi = i_;
  return res;
}

static cbor_map_iterator_t cbor_map_iterator_init_serialized(cbor a)
{
  uint64_t len = cbor_map_length(a);
  cbor_serialized s = destr_cbor_serialized(a);
  uint8_t *ar = focus_map_with_ghost_length(s.cbor_serialized_payload);
  return
    (
      (cbor_map_iterator_t){
        .cbor_map_iterator_length = len,
        .cbor_map_iterator_payload = {
          .tag = CBOR_Map_Iterator_Payload_Serialized,
          { .case_CBOR_Map_Iterator_Payload_Serialized = ar }
        }
      }
    );
}

static cbor_map_iterator_t cbor_map_iterator_init_map(cbor a)
{
  uint64_t len = cbor_map_length(a);
  cbor_map ar = destr_cbor_map(a);
  return
    (
      (cbor_map_iterator_t){
        .cbor_map_iterator_length = len,
        .cbor_map_iterator_payload = {
          .tag = CBOR_Map_Iterator_Payload_Map,
          { .case_CBOR_Map_Iterator_Payload_Map = ar.cbor_map_payload }
        }
      }
    );
}

uint8_t cbor_get_major_type(cbor a)
{
  if (a.tag == CBOR_Case_Map)
    return CBOR_MAJOR_TYPE_MAP;
  else if (a.tag == CBOR_Case_Array)
    return CBOR_MAJOR_TYPE_ARRAY;
  else if (a.tag == CBOR_Case_Tagged)
    return CBOR_MAJOR_TYPE_TAGGED;
  else if (a.tag == CBOR_Case_Simple_value)
    return CBOR_MAJOR_TYPE_SIMPLE_VALUE;
  else if (a.tag == CBOR_Case_String)
  {
    cbor_string s = cbor_destr_string(a);
    return s.cbor_string_type;
  }
  else if (a.tag == CBOR_Case_Int64)
  {
    cbor_int i = cbor_destr_int64(a);
    return i.cbor_int_type;
  }
  else
  {
    cbor_serialized s = destr_cbor_serialized(a);
    uint8_t res = read_major_type(s.cbor_serialized_payload);
    return res;
  }
}

size_t cbor_size_comp(cbor c, size_t sz, bool *perr)
{
  if (c.tag == CBOR_Case_Int64)
    return size_comp_for_int64(c, sz, perr);
  else if (c.tag == CBOR_Case_Simple_value)
    return size_comp_for_simple_value(c, sz, perr);
  else if (c.tag == CBOR_Case_String)
    return size_comp_for_string(c, sz, perr);
  else if (c.tag == CBOR_Case_Tagged)
  {
    cbor_tagged c_ = cbor_destr_tagged(c);
    size_t sz1 = size_comp_uint64_header(c_.cbor_tagged_tag, sz, perr);
    bool err1 = *perr;
    if (err1)
      return sz1;
    else
    {
      size_t sz2 = cbor_size_comp(c_.cbor_tagged_payload, sz1, perr);
      return sz2;
    }
  }
  else if (c.tag == CBOR_Case_Array)
  {
    static_assert(UINT64_MAX <= SIZE_MAX);
    cbor_array c_ = cbor_destr_array(c);
    size_t sz1 = size_comp_uint64_header(c_.cbor_array_length, sz, perr);
    bool err1 = *perr;
    if (err1)
      return sz1;
    else
    {
      size_t r = (size_t)0U;
      size_t r1 = sz1;
      size_t n0 = r;
      bool err0 = *perr;
      bool cond = !err0 && n0 < (size_t)c_.cbor_array_length;
      while (cond)
      {
        size_t n = r;
        cbor x = c_.cbor_array_payload[n];
        size_t sz11 = r1;
        size_t res = cbor_size_comp(x, sz11, perr);
        size_t sz2 = res;
        bool err0 = *perr;
        if (!err0)
          r = n + (size_t)1U;
        if (!err0)
          r1 = sz2;
        size_t n0 = r;
        bool err = *perr;
        cond = !err && n0 < (size_t)c_.cbor_array_length;
      }
      size_t v = r1;
      size_t v0 = v;
      size_t v1 = v0;
      size_t v2 = v1;
      size_t res = v2;
      size_t sz2 = res;
      return sz2;
    }
  }
  else if (c.tag == CBOR_Case_Map)
  {
    static_assert(UINT64_MAX <= SIZE_MAX);
    cbor_map c_ = destr_cbor_map(c);
    size_t sz1 = size_comp_uint64_header(c_.cbor_map_length, sz, perr);
    bool err1 = *perr;
    if (err1)
      return sz1;
    else
    {
      size_t r = (size_t)0U;
      size_t r1 = sz1;
      size_t n0 = r;
      bool err0 = *perr;
      bool cond = !err0 && n0 < (size_t)c_.cbor_map_length;
      while (cond)
      {
        size_t n = r;
        cbor_map_entry x = c_.cbor_map_payload[n];
        size_t sz11 = r1;
        size_t sz12 = cbor_size_comp(cbor_map_entry_key(x), sz11, perr);
        bool err0 = *perr;
        size_t res;
        if (err0)
          res = sz12;
        else
        {
          size_t sz2 = cbor_size_comp(cbor_map_entry_value(x), sz12, perr);
          res = sz2;
        }
        size_t sz2 = res;
        bool err1 = *perr;
        if (!err1)
          r = n + (size_t)1U;
        if (!err1)
          r1 = sz2;
        size_t n0 = r;
        bool err = *perr;
        cond = !err && n0 < (size_t)c_.cbor_map_length;
      }
      size_t v = r1;
      size_t v0 = v;
      size_t v1 = v0;
      size_t v2 = v1;
      size_t res = v2;
      size_t sz2 = res;
      return sz2;
    }
  }
  else
    return size_comp_for_serialized(c, sz, perr);
}

uint8_t *cbor_l2r_write(cbor c, LowParse_SteelST_L2ROutput_t out)
{
  if (c.tag == CBOR_Case_Int64)
    return l2r_writer_for_int64(c, out);
  else if (c.tag == CBOR_Case_Simple_value)
    return l2r_writer_for_simple_value(c, out);
  else if (c.tag == CBOR_Case_String)
    return l2r_write_cbor_string(c, out);
  else if (c.tag == CBOR_Case_Tagged)
  {
    cbor_tagged c_ = cbor_destr_tagged(c);
    uint8_t *res = l2r_write_uint64_header(CBOR_MAJOR_TYPE_TAGGED, c_.cbor_tagged_tag, out);
    cbor_l2r_write(c_.cbor_tagged_payload, out);
    return res;
  }
  else if (c.tag == CBOR_Case_Array)
  {
    static_assert(UINT64_MAX <= SIZE_MAX);
    cbor_array c_ = cbor_destr_array(c);
    uint8_t *res = l2r_write_uint64_header(CBOR_MAJOR_TYPE_ARRAY, c_.cbor_array_length, out);
    uint8_t *res1 = *out.ptr;
    size_t xlen = *out.len;
    size_t xlen_ = xlen - (size_t)0U;
    uint8_t *xptr_ = res1;
    *out.ptr = xptr_;
    *out.len = xlen_;
    uint8_t *out0 = res1;
    size_t r = (size_t)0U;
    size_t n0 = r;
    bool cond = n0 < (size_t)c_.cbor_array_length;
    while (cond)
    {
      size_t n = r;
      cbor x = c_.cbor_array_payload[n];
      uint8_t *res1 = cbor_l2r_write(x, out);
      KRML_MAYBE_UNUSED_VAR(res1);
      r = n + (size_t)1U;
      size_t n0 = r;
      cond = n0 < (size_t)c_.cbor_array_length;
    }
    uint8_t *v = out0;
    uint8_t *v0 = v;
    uint8_t *res10 = v0;
    KRML_MAYBE_UNUSED_VAR(res10);
    return res;
  }
  else if (c.tag == CBOR_Case_Map)
  {
    static_assert(UINT64_MAX <= SIZE_MAX);
    cbor_map c_ = destr_cbor_map(c);
    uint8_t *res = l2r_write_uint64_header(CBOR_MAJOR_TYPE_MAP, c_.cbor_map_length, out);
    uint8_t *res10 = *out.ptr;
    size_t xlen = *out.len;
    size_t xlen_ = xlen - (size_t)0U;
    uint8_t *xptr_ = res10;
    *out.ptr = xptr_;
    *out.len = xlen_;
    uint8_t *out0 = res10;
    size_t r = (size_t)0U;
    size_t n0 = r;
    bool cond = n0 < (size_t)c_.cbor_map_length;
    while (cond)
    {
      size_t n = r;
      cbor_map_entry x = c_.cbor_map_payload[n];
      uint8_t *res1 = cbor_l2r_write(cbor_map_entry_key(x), out);
      cbor_l2r_write(cbor_map_entry_value(x), out);
      uint8_t *res10 = res1;
      KRML_MAYBE_UNUSED_VAR(res10);
      r = n + (size_t)1U;
      size_t n0 = r;
      cond = n0 < (size_t)c_.cbor_map_length;
    }
    uint8_t *v = out0;
    uint8_t *v0 = v;
    uint8_t *res1 = v0;
    KRML_MAYBE_UNUSED_VAR(res1);
    return res;
  }
  else
    return l2r_writer_for_serialized(c, out);
}

size_t cbor_write(cbor c, uint8_t *out, size_t sz)
{
  bool r = false;
  size_t sz_ = cbor_size_comp(c, sz, &r);
  bool err = r;
  if (err)
    return (size_t)0U;
  else
  {
    uint8_t *a = out;
    size_t r1 = sz;
    uint8_t *r2 = a;
    size_t res = sz - sz_;
    LowParse_SteelST_L2ROutput_t w = { .ptr = &r2, .len = &r1 };
    cbor_l2r_write(c, w);
    size_t v = res;
    size_t v0 = v;
    return v0;
  }
}

cbor cbor_dummy = { .tag = CBOR_Case_Simple_value, { .case_CBOR_Case_Simple_value = 0U } };

cbor cbor_map_entry_key(cbor_map_entry x)
{
  return x.cbor_map_entry_key;
}

cbor cbor_map_entry_value(cbor_map_entry x)
{
  return x.cbor_map_entry_value;
}

cbor_map_entry cbor_mk_map_entry(cbor k, cbor v)
{
  return ((cbor_map_entry){ .cbor_map_entry_key = k, .cbor_map_entry_value = v });
}

cbor read_valid_cbor_from_buffer_with_size_strong(uint8_t *a, size_t alen)
{
  return
    (
      (cbor){
        .tag = CBOR_Case_Serialized,
        {
          .case_CBOR_Case_Serialized = {
            .cbor_serialized_size = alen,
            .cbor_serialized_payload = a
          }
        }
      }
    );
}

cbor_serialized destr_cbor_serialized(cbor c)
{
  if (c.tag == CBOR_Case_Serialized)
    return c.case_CBOR_Case_Serialized;
  else
  {
    KRML_HOST_EPRINTF("KaRaMeL abort at %s:%d\n%s\n",
      __FILE__,
      __LINE__,
      "unreachable (pattern matches are exhaustive in F*)");
    KRML_HOST_EXIT(255U);
  }
}

typedef struct __CBOR_SteelST_Type_Def_cbor_CBOR_SteelST_Type_Def_cbor_s
{
  cbor fst;
  cbor snd;
}
__CBOR_SteelST_Type_Def_cbor_CBOR_SteelST_Type_Def_cbor;

int16_t cbor_compare_aux(cbor a1, cbor a2)
{
  __CBOR_SteelST_Type_Def_cbor_CBOR_SteelST_Type_Def_cbor scrut = { .fst = a1, .snd = a2 };
  if (scrut.fst.tag == CBOR_Case_Serialized && scrut.snd.tag == CBOR_Case_Serialized)
  {
    cbor_serialized s1 = destr_cbor_serialized(a1);
    cbor_serialized s2 = destr_cbor_serialized(a2);
    int16_t res0;
    if (s1.cbor_serialized_size == (size_t)0U)
      if (s2.cbor_serialized_size == (size_t)0U)
        res0 = (int16_t)0;
      else
        res0 = (int16_t)-1;
    else if (s2.cbor_serialized_size == (size_t)0U)
      res0 = (int16_t)1;
    else
    {
      uint8_t *r = s1.cbor_serialized_payload;
      uint8_t *r1 = s2.cbor_serialized_payload;
      size_t r2 = s1.cbor_serialized_size;
      size_t r3 = s2.cbor_serialized_size;
      int16_t r4 = (int16_t)0;
      int16_t res = r4;
      size_t na0 = r2;
      size_t nb0 = r3;
      bool cond = res == (int16_t)0 && na0 > (size_t)0U && nb0 > (size_t)0U;
      while (cond)
      {
        uint8_t *a = r;
        size_t na0 = r2;
        size_t na_ = na0 - (size_t)1U;
        size_t a_sz = (size_t)1U;
        uint8_t *b = r1;
        size_t nb0 = r3;
        size_t nb_ = nb0 - (size_t)1U;
        size_t b_sz = (size_t)1U;
        uint8_t x1 = a[0U];
        uint8_t x2 = b[0U];
        int16_t comp = (int16_t)x1 - (int16_t)x2;
        if (comp != (int16_t)0)
          r4 = comp;
        else
        {
          r2 = na_;
          r3 = nb_;
          if (na_ == (size_t)0U)
          {
            if (nb_ != (size_t)0U)
              r4 = (int16_t)-1;
          }
          else if (nb_ == (size_t)0U)
            r4 = (int16_t)1;
          else
          {
            uint8_t *a_ = a + a_sz;
            r = a_;
            uint8_t *b_ = b + b_sz;
            r1 = b_;
          }
        }
        int16_t res = r4;
        size_t na = r2;
        size_t nb = r3;
        cond = res == (int16_t)0 && na > (size_t)0U && nb > (size_t)0U;
      }
      int16_t v = r4;
      int16_t v0 = v;
      int16_t v1 = v0;
      int16_t v2 = v1;
      int16_t v3 = v2;
      int16_t res1 = v3;
      res0 = res1;
    }
    int16_t i = res0;
    if (i < (int16_t)0)
      return (int16_t)-1;
    else if ((int16_t)0 < i)
      return (int16_t)1;
    else
      return (int16_t)0;
  }
  else
    return (int16_t)2;
}

size_t size_comp_for_serialized(cbor c, size_t sz, bool *perr)
{
  cbor_serialized c_ = destr_cbor_serialized(c);
  if (c_.cbor_serialized_size > sz)
  {
    *perr = true;
    return sz;
  }
  else
    return sz - c_.cbor_serialized_size;
}

uint8_t *l2r_writer_for_serialized(cbor c, LowParse_SteelST_L2ROutput_t out)
{
  cbor_serialized c_ = destr_cbor_serialized(c);
  uint8_t *res = *out.ptr;
  size_t xlen = *out.len;
  size_t xlen_ = xlen - c_.cbor_serialized_size;
  uint8_t *xptr_ = res + c_.cbor_serialized_size;
  *out.ptr = xptr_;
  *out.len = xlen_;
  uint8_t *res0 = res;
  uint8_t *p_src = c_.cbor_serialized_payload;
  uint8_t *p_dst = res0;
  memcpy(p_dst, p_src, c_.cbor_serialized_size * sizeof (uint8_t));
  return res0;
}

cbor cbor_constr_array(cbor *a, uint64_t len)
{
  return
    (
      (cbor){
        .tag = CBOR_Case_Array,
        { .case_CBOR_Case_Array = { .cbor_array_length = len, .cbor_array_payload = a } }
      }
    );
}

cbor_array cbor_destr_array(cbor a)
{
  if (a.tag == CBOR_Case_Array)
    return a.case_CBOR_Case_Array;
  else
  {
    KRML_HOST_EPRINTF("KaRaMeL abort at %s:%d\n%s\n",
      __FILE__,
      __LINE__,
      "unreachable (pattern matches are exhaustive in F*)");
    KRML_HOST_EXIT(255U);
  }
}

uint64_t cbor_array_length(cbor a)
{
  if (a.tag == CBOR_Case_Array)
  {
    cbor_array a_ = cbor_destr_array(a);
    return a_.cbor_array_length;
  }
  else
  {
    cbor_serialized s = destr_cbor_serialized(a);
    uint64_t res = read_argument_as_uint64(s.cbor_serialized_payload);
    return res;
  }
}

static cbor cbor_array_index_case_array(cbor a, size_t i)
{
  cbor_array ar = cbor_destr_array(a);
  return ar.cbor_array_payload[i];
}

static cbor cbor_array_index_case_serialized(cbor a, size_t i)
{
  cbor_serialized s = destr_cbor_serialized(a);
  uint8_t *a1 = focus_array_with_ghost_length(s.cbor_serialized_payload);
  uint8_t *e = a1;
  bool cont = (size_t)0U < i;
  bool r = cont;
  size_t r1 = (size_t)0U;
  uint8_t *r2 = e;
  while (r)
  {
    uint8_t *e1 = r2;
    size_t res = jump_raw_data_item(e1);
    size_t sz = res;
    uint8_t *res0 = e1 + sz;
    uint8_t *res1 = res0;
    uint8_t *e_ = res1;
    size_t n1 = r1;
    size_t n_ = n1 + (size_t)1U;
    bool cont_ = n_ < i;
    r1 = n_;
    r = cont_;
    r2 = e_;
  }
  uint8_t *v1 = r2;
  uint8_t *v10 = v1;
  uint8_t *v11 = v10;
  uint8_t *e0 = v11;
  uint8_t *res0 = e0;
  uint8_t *elt = res0;
  size_t res = jump_raw_data_item(elt);
  size_t sz = res;
  cbor res1 = read_valid_cbor_from_buffer_with_size_strong(elt, sz);
  return res1;
}

cbor cbor_array_index(cbor a, size_t i)
{
  if (a.tag == CBOR_Case_Array)
    return cbor_array_index_case_array(a, i);
  else
    return cbor_array_index_case_serialized(a, i);
}

cbor_array_iterator_t
cbor_dummy_array_iterator =
  {
    .cbor_array_iterator_length = 0ULL,
    .cbor_array_iterator_payload = {
      .tag = CBOR_Array_Iterator_Payload_Array,
      { .case_CBOR_Array_Iterator_Payload_Array = NULL }
    }
  };

static cbor_array_iterator_t cbor_array_iterator_init_array(cbor a)
{
  uint64_t len = cbor_array_length(a);
  cbor_array ar = cbor_destr_array(a);
  return
    (
      (cbor_array_iterator_t){
        .cbor_array_iterator_length = len,
        .cbor_array_iterator_payload = {
          .tag = CBOR_Array_Iterator_Payload_Array,
          { .case_CBOR_Array_Iterator_Payload_Array = ar.cbor_array_payload }
        }
      }
    );
}

static cbor_array_iterator_t cbor_array_iterator_init_serialized(cbor a)
{
  uint64_t len = cbor_array_length(a);
  cbor_serialized s = destr_cbor_serialized(a);
  uint8_t *ar = focus_array_with_ghost_length(s.cbor_serialized_payload);
  return
    (
      (cbor_array_iterator_t){
        .cbor_array_iterator_length = len,
        .cbor_array_iterator_payload = {
          .tag = CBOR_Array_Iterator_Payload_Serialized,
          { .case_CBOR_Array_Iterator_Payload_Serialized = ar }
        }
      }
    );
}

cbor_array_iterator_t cbor_array_iterator_init(cbor a)
{
  if (a.tag == CBOR_Case_Array)
    return cbor_array_iterator_init_array(a);
  else
    return cbor_array_iterator_init_serialized(a);
}

bool cbor_array_iterator_is_done(cbor_array_iterator_t i)
{
  return i.cbor_array_iterator_length == 0ULL;
}

static cbor cbor_array_iterator_next_array(cbor_array_iterator_t i, cbor_array_iterator_t *pi)
{
  cbor *a;
  if (i.cbor_array_iterator_payload.tag == CBOR_Array_Iterator_Payload_Array)
    a = i.cbor_array_iterator_payload.case_CBOR_Array_Iterator_Payload_Array;
  else
    a = KRML_EABORT(cbor *, "unreachable (pattern matches are exhaustive in F*)");
  uint64_t len_ = i.cbor_array_iterator_length - 1ULL;
  cbor res = a[0U];
  cbor *ar_ = a + (size_t)1U;
  cbor_array_iterator_t
  i_ =
    {
      .cbor_array_iterator_length = len_,
      .cbor_array_iterator_payload = {
        .tag = CBOR_Array_Iterator_Payload_Array,
        { .case_CBOR_Array_Iterator_Payload_Array = ar_ }
      }
    };
  *pi = i_;
  return res;
}

static cbor
cbor_array_iterator_next_serialized(cbor_array_iterator_t i, cbor_array_iterator_t *pi)
{
  uint8_t *a;
  if (i.cbor_array_iterator_payload.tag == CBOR_Array_Iterator_Payload_Serialized)
    a = i.cbor_array_iterator_payload.case_CBOR_Array_Iterator_Payload_Serialized;
  else
    a = KRML_EABORT(uint8_t *, "unreachable (pattern matches are exhaustive in F*)");
  uint64_t len_ = i.cbor_array_iterator_length - 1ULL;
  size_t res0 = jump_raw_data_item(a);
  size_t sz = res0;
  uint8_t *a_ = a + sz;
  cbor res = read_valid_cbor_from_buffer_with_size_strong(a, sz);
  cbor_array_iterator_t
  i_ =
    {
      .cbor_array_iterator_length = len_,
      .cbor_array_iterator_payload = {
        .tag = CBOR_Array_Iterator_Payload_Serialized,
        { .case_CBOR_Array_Iterator_Payload_Serialized = a_ }
      }
    };
  *pi = i_;
  return res;
}

cbor cbor_array_iterator_next(cbor_array_iterator_t *pi)
{
  cbor_array_iterator_t i1 = *pi;
  if (i1.cbor_array_iterator_payload.tag == CBOR_Array_Iterator_Payload_Array)
    return cbor_array_iterator_next_array(i1, pi);
  else
    return cbor_array_iterator_next_serialized(i1, pi);
}

cbor *cbor_read_array(cbor input, cbor *a0, uint64_t len)
{
  if (input.tag == CBOR_Case_Array)
  {
    cbor_array res = cbor_destr_array(input);
    return res.cbor_array_payload;
  }
  else
  {
    cbor_serialized s = destr_cbor_serialized(input);
    uint8_t *a = focus_array_with_ghost_length(s.cbor_serialized_payload);
    static_assert(UINT64_MAX <= SIZE_MAX);
    size_t n0_as_sz = (size_t)len;
    size_t r = (size_t)0U;
    uint8_t *r1 = a;
    size_t n = r;
    bool cond = n < n0_as_sz;
    while (cond)
    {
      uint8_t *r2 = r1;
      size_t res = jump_raw_data_item(r2);
      size_t sz = res;
      uint8_t *r_ = r2 + sz;
      r1 = r_;
      size_t n = r;
      size_t n_ = n + (size_t)1U;
      r = n_;
      cbor c = read_valid_cbor_from_buffer_with_size_strong(r2, sz);
      a0[n] = c;
      size_t n0 = r;
      cond = n0 < n0_as_sz;
    }
    return a0;
  }
}

static bool is_CBOR_Case_String(cbor c)
{
  if (c.tag == CBOR_Case_String)
    return true;
  else
    return false;
}

cbor_string cbor_destr_string(cbor c)
{
  if (is_CBOR_Case_String(c))
    if (c.tag == CBOR_Case_String)
      return c.case_CBOR_Case_String;
    else
    {
      KRML_HOST_EPRINTF("KaRaMeL abort at %s:%d\n%s\n",
        __FILE__,
        __LINE__,
        "unreachable (pattern matches are exhaustive in F*)");
      KRML_HOST_EXIT(255U);
    }
  else
  {
    cbor_serialized cs = destr_cbor_serialized(c);
    uint8_t typ = read_major_type(cs.cbor_serialized_payload);
    uint64_t len = read_argument_as_uint64(cs.cbor_serialized_payload);
    uint8_t *lpayload = focus_string(cs.cbor_serialized_payload);
    uint8_t *payload = lpayload;
    return
      (
        (cbor_string){
          .cbor_string_type = typ,
          .cbor_string_length = len,
          .cbor_string_payload = payload
        }
      );
  }
}

size_t size_comp_for_string(cbor c, size_t sz, bool *perr)
{
  static_assert(UINT64_MAX <= SIZE_MAX);
  cbor_string c_ = cbor_destr_string(c);
  size_t res = size_comp_string(c_.cbor_string_length, sz, perr);
  return res;
}

uint8_t *l2r_write_cbor_string(cbor c, LowParse_SteelST_L2ROutput_t out)
{
  static_assert(UINT64_MAX <= SIZE_MAX);
  cbor_string c_ = cbor_destr_string(c);
  uint8_t *res = l2r_write_uint64_header(c_.cbor_string_type, c_.cbor_string_length, out);
  size_t len = (size_t)c_.cbor_string_length;
  uint8_t *res1 = *out.ptr;
  size_t xlen = *out.len;
  size_t xlen_ = xlen - len;
  uint8_t *xptr_ = res1 + len;
  *out.ptr = xptr_;
  *out.len = xlen_;
  uint8_t *res_pl = res1;
  uint8_t *res_pl_a = res_pl;
  uint8_t *p_src = c_.cbor_string_payload;
  uint8_t *p_dst = res_pl_a;
  memcpy(p_dst, p_src, len * sizeof (uint8_t));
  return res;
}

cbor cbor_constr_string(uint8_t typ, uint8_t *a, uint64_t len)
{
  return
    (
      (cbor){
        .tag = CBOR_Case_String,
        {
          .case_CBOR_Case_String = {
            .cbor_string_type = typ,
            .cbor_string_length = len,
            .cbor_string_payload = a
          }
        }
      }
    );
}

uint8_t cbor_destr_simple_value(cbor c)
{
  if (c.tag == CBOR_Case_Simple_value)
    return c.case_CBOR_Case_Simple_value;
  else if (c.tag == CBOR_Case_Serialized)
  {
    cbor_serialized cs = c.case_CBOR_Case_Serialized;
    uint8_t c_ = read_simple_value(cs.cbor_serialized_payload);
    return c_;
  }
  else
  {
    KRML_HOST_EPRINTF("KaRaMeL abort at %s:%d\n%s\n",
      __FILE__,
      __LINE__,
      "unreachable (pattern matches are exhaustive in F*)");
    KRML_HOST_EXIT(255U);
  }
}

size_t size_comp_for_simple_value(cbor c, size_t sz, bool *perr)
{
  uint8_t c_ = cbor_destr_simple_value(c);
  size_t res = size_comp_simple_value(c_, sz, perr);
  return res;
}

uint8_t *l2r_writer_for_simple_value(cbor c, LowParse_SteelST_L2ROutput_t out)
{
  uint8_t c_ = cbor_destr_simple_value(c);
  uint8_t *res = l2r_write_simple_value(c_, out);
  return res;
}

cbor cbor_constr_simple_value(uint8_t value)
{
  return ((cbor){ .tag = CBOR_Case_Simple_value, { .case_CBOR_Case_Simple_value = value } });
}

cbor_int cbor_destr_int64(cbor c)
{
  if (c.tag == CBOR_Case_Int64)
    return c.case_CBOR_Case_Int64;
  else if (c.tag == CBOR_Case_Serialized)
  {
    cbor_serialized cs = c.case_CBOR_Case_Serialized;
    uint8_t typ = read_major_type(cs.cbor_serialized_payload);
    uint64_t value = read_int64(cs.cbor_serialized_payload);
    return ((cbor_int){ .cbor_int_type = typ, .cbor_int_value = value });
  }
  else
  {
    KRML_HOST_EPRINTF("KaRaMeL abort at %s:%d\n%s\n",
      __FILE__,
      __LINE__,
      "unreachable (pattern matches are exhaustive in F*)");
    KRML_HOST_EXIT(255U);
  }
}

size_t size_comp_for_int64(cbor c, size_t sz, bool *perr)
{
  cbor_int c_ = cbor_destr_int64(c);
  size_t res = size_comp_int64(c_.cbor_int_value, sz, perr);
  return res;
}

uint8_t *l2r_writer_for_int64(cbor c, LowParse_SteelST_L2ROutput_t out)
{
  cbor_int c_ = cbor_destr_int64(c);
  uint8_t *res = l2r_write_int64(c_.cbor_int_type, c_.cbor_int_value, out);
  return res;
}

cbor cbor_constr_int64(uint8_t ty, uint64_t value)
{
  return
    (
      (cbor){
        .tag = CBOR_Case_Int64,
        { .case_CBOR_Case_Int64 = { .cbor_int_type = ty, .cbor_int_value = value } }
      }
    );
}

static cbor_tagged0 destr_cbor_tagged0(cbor a)
{
  if (a.tag == CBOR_Case_Tagged)
    return a.case_CBOR_Case_Tagged;
  else
  {
    KRML_HOST_EPRINTF("KaRaMeL abort at %s:%d\n%s\n",
      __FILE__,
      __LINE__,
      "unreachable (pattern matches are exhaustive in F*)");
    KRML_HOST_EXIT(255U);
  }
}

cbor_tagged cbor_destr_tagged(cbor a)
{
  if (a.tag == CBOR_Case_Tagged)
  {
    cbor_tagged0 r = destr_cbor_tagged0(a);
    cbor pl = *r.cbor_tagged0_payload;
    return ((cbor_tagged){ .cbor_tagged_tag = r.cbor_tagged0_tag, .cbor_tagged_payload = pl });
  }
  else
  {
    cbor_serialized s = destr_cbor_serialized(a);
    uint64_t tag = read_argument_as_uint64(s.cbor_serialized_payload);
    uint8_t *s_ = focus_tagged(s.cbor_serialized_payload);
    size_t res = jump_raw_data_item(s_);
    size_t sz = res;
    cbor pl = read_valid_cbor_from_buffer_with_size_strong(s_, sz);
    return ((cbor_tagged){ .cbor_tagged_tag = tag, .cbor_tagged_payload = pl });
  }
}

cbor cbor_constr_tagged(uint64_t tag, cbor *a)
{
  return
    (
      (cbor){
        .tag = CBOR_Case_Tagged,
        { .case_CBOR_Case_Tagged = { .cbor_tagged0_tag = tag, .cbor_tagged0_payload = a } }
      }
    );
}

cbor_map destr_cbor_map(cbor a)
{
  if (a.tag == CBOR_Case_Map)
    return a.case_CBOR_Case_Map;
  else
  {
    KRML_HOST_EPRINTF("KaRaMeL abort at %s:%d\n%s\n",
      __FILE__,
      __LINE__,
      "unreachable (pattern matches are exhaustive in F*)");
    KRML_HOST_EXIT(255U);
  }
}

uint64_t cbor_map_length(cbor a)
{
  if (a.tag == CBOR_Case_Map)
  {
    cbor_map a_ = destr_cbor_map(a);
    return a_.cbor_map_length;
  }
  else
  {
    cbor_serialized s = destr_cbor_serialized(a);
    uint64_t res = read_argument_as_uint64(s.cbor_serialized_payload);
    return res;
  }
}

cbor_map_iterator_t
cbor_dummy_map_iterator =
  {
    .cbor_map_iterator_length = 0ULL,
    .cbor_map_iterator_payload = {
      .tag = CBOR_Map_Iterator_Payload_Map,
      { .case_CBOR_Map_Iterator_Payload_Map = NULL }
    }
  };

cbor cbor_constr_map(cbor_map_entry *a, uint64_t len)
{
  return
    (
      (cbor){
        .tag = CBOR_Case_Map,
        { .case_CBOR_Case_Map = { .cbor_map_length = len, .cbor_map_payload = a } }
      }
    );
}

cbor_map_iterator_t cbor_map_iterator_init(cbor a)
{
  if (a.tag == CBOR_Case_Map)
    return cbor_map_iterator_init_map(a);
  else
    return cbor_map_iterator_init_serialized(a);
}

bool cbor_map_iterator_is_done(cbor_map_iterator_t i)
{
  return i.cbor_map_iterator_length == 0ULL;
}

cbor_map_entry cbor_map_iterator_next(cbor_map_iterator_t *pi)
{
  cbor_map_iterator_t i1 = *pi;
  if (i1.cbor_map_iterator_payload.tag == CBOR_Map_Iterator_Payload_Map)
    return cbor_map_iterator_next_map(i1, pi);
  else
    return cbor_map_iterator_next_serialized(i1, pi);
}

static cbor_read_t cbor_read_(uint8_t *a, size_t sz)
{
  uint8_t *a_ = a;
  uint32_t r = 0U;
  size_t sz_ = validate_raw_data_item(a_, sz, &r);
  uint32_t err = r;
  cbor_read_t v;
  if (err == 0U)
  {
    uint8_t *rem_ = a_ + sz_;
    uint8_t *rem = rem_;
    cbor c = read_valid_cbor_from_buffer_with_size_strong(a_, sz_);
    v =
      (
        (cbor_read_t){
          .cbor_read_is_success = true,
          .cbor_read_payload = c,
          .cbor_read_remainder = rem,
          .cbor_read_remainder_length = sz - sz_
        }
      );
  }
  else
    v =
      (
        (cbor_read_t){
          .cbor_read_is_success = false,
          .cbor_read_payload = cbor_dummy,
          .cbor_read_remainder = a,
          .cbor_read_remainder_length = sz
        }
      );
  cbor_read_t res = v;
  return res;
}

cbor_read_t cbor_read(uint8_t *a, size_t sz)
{
  return cbor_read_(a, sz);
}

cbor_read_t cbor_read_deterministically_encoded(uint8_t *a, size_t sz)
{
  static_assert(UINT64_MAX <= SIZE_MAX);
  cbor_read_t res = cbor_read_(a, sz);
  if (!res.cbor_read_is_success)
    return res;
  else
  {
    cbor_serialized s = destr_cbor_serialized(res.cbor_read_payload);
    bool r = true;
    uint8_t *r1 = s.cbor_serialized_payload;
    size_t r2 = (size_t)1U;
    bool r3 = true;
    bool cont = r3;
    bool cond;
    if (cont)
    {
      bool cont_ = r;
      if (cont_)
        cond = cont;
      else
      {
        r3 = false;
        cond = false;
      }
    }
    else
      cond = cont;
    while (cond)
    {
      size_t n = r2;
      size_t n_pred = n - (size_t)1U;
      uint8_t *a1 = r1;
      bool res1 = r;
      if (res1)
      {
        uint8_t res20 = read_header_major_type(a1);
        uint8_t major_type = res20;
        bool res21;
        if (major_type == CBOR_MAJOR_TYPE_MAP)
        {
          uint64_t n64 = read_argument_as_uint64_(a1);
          size_t n1 = (size_t)n64;
          size_t res20 = jump_header_(a1);
          size_t sz1 = res20;
          uint8_t *res22 = a1 + sz1;
          uint8_t *ac = res22;
          bool res23;
          if (n1 < (size_t)2U)
            res23 = true;
          else
          {
            size_t n2 = n1 - (size_t)1U;
            uint8_t *r4 = ac;
            size_t r5 = n2;
            bool r6 = true;
            bool res2 = r6;
            size_t n210 = r5;
            bool cond = res2 && n210 > (size_t)0U;
            while (cond)
            {
              uint8_t *a2 = r4;
              size_t s1 = jump_raw_data_item_(a2);
              uint8_t *ar = a2 + s1;
              size_t s2 = jump_raw_data_item_(ar);
              size_t res2 = s1 + s2;
              size_t sz1 = res2;
              uint8_t *res20 = a2 + sz1;
              uint8_t *a21 = res20;
              r4 = a21;
              size_t n210 = r5;
              size_t n2_ = n210 - (size_t)1U;
              r5 = n2_;
              bool res21 = deterministically_encoded_cbor_map_key_order_impl(a2, a21);
              bool res22 = res21;
              r6 = res22;
              bool res23 = r6;
              size_t n21 = r5;
              cond = res23 && n21 > (size_t)0U;
            }
            bool v = r6;
            bool v0 = v;
            bool v1 = v0;
            bool res20 = v1;
            res23 = res20;
          }
          res21 = res23;
        }
        else
          res21 = true;
        r = res21;
      }
      size_t res10 = jump_leaf(a1);
      size_t sz1 = res10;
      uint8_t *res11 = a1 + sz1;
      uint8_t *a2 = res11;
      r1 = a2;
      size_t nl = jump_count_remaining_data_items(a1);
      size_t n_ = nl + n_pred;
      r2 = n_;
      r3 = n_ != (size_t)0U;
      bool cont = r3;
      bool ite;
      if (cont)
      {
        bool cont_ = r;
        if (cont_)
          ite = cont;
        else
        {
          r3 = false;
          ite = false;
        }
      }
      else
        ite = cont;
      cond = ite;
    }
    bool v = r;
    bool res1 = v;
    bool res10 = res1;
    bool res11 = res10;
    bool test = res11;
    if (test)
      return res;
    else
      return
        (
          (cbor_read_t){
            .cbor_read_is_success = false,
            .cbor_read_payload = res.cbor_read_payload,
            .cbor_read_remainder = res.cbor_read_remainder,
            .cbor_read_remainder_length = res.cbor_read_remainder_length
          }
        );
  }
}

