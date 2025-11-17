#include "common.h"

#include "immintrin.h"

// FNV1a, seems pretty good...
u32 hash_string(String string)
{
  u32 hash = 0x811c9dc5; // Offset basis
  for (isize i = 0; i < string.count; i++)
  {
    hash ^= string.v[i];
    hash *= 0x01000193; // Prime
  }

  return hash;
}

// No standard library sutff
template<typename T>
inline void swap(T &a, T &b) {
  T t = a;
  a   = b;
  b   = t;
}

template <typename Value_T>
struct Table
{
  Arena   memory;
  isize   capacity;
  String  *keys;
  Value_T *values;
  u32     *hashes;
  u32     *distances;

  void init(isize _capacity)
  {
    Arena_Args arena_args = {};
    arena_args.reserve_size = GB(1);
    memory = __arena_make(&arena_args);

    capacity  = _capacity * 2; // Way better at lower capacity
    keys      = arena_calloc(&memory, capacity + 8, String);
    values    = arena_calloc(&memory, capacity + 8, Value_T);
    hashes    = arena_calloc(&memory, capacity + 8, u32);
    distances = arena_calloc(&memory, capacity + 8, u32);

    // Init distances with empty sentinel value
    for (isize i = 0; i < capacity; i++)
    {
      distances[i] = 0xFFFFFFFF;
    }
  }

  void clear()
  {
    for (isize i = 0; i < capacity; i++)
    {
      keys[i]      = {};
      values[i]    = {};
      hashes[i]    = 0;
      distances[i] = 0xFFFFFFFF;
    }
  }

  void free()
  {
    arena_free(&memory);
  }

  void insert(String key, Value_T value)
  {
    u32 hash  = hash_string(key);
    u32 index = hash % capacity; // can also just be a pow2 modulo

    String curr_key  = key;
    Value_T curr_val = value;
    u32 curr_hash    = hash;
    u32 curr_dist    = 0;

    while (true)
    {
      // Great, we found an empty slot, no more swaps
      if (distances[index] == 0xFFFFFFFF)
      {
        keys[index]      = curr_key;
        values[index]    = curr_val;
        hashes[index]    = curr_hash;
        distances[index] = curr_dist;
        break;
      }

      // We need to swap and take this one's place, and continue walking with that
      if (curr_dist > distances[index])
      {
        swap(keys[index],      curr_key);
        swap(values[index],    curr_val);
        swap(hashes[index],    curr_hash);
        swap(distances[index], curr_dist);
      }

      curr_dist += 1;
      index = (index + 1) & (capacity - 1); // Powers of 2 only
    }
  }

  Value_T *find(String key)
  {
    u32 hash  = hash_string(key);
    u32 index = hash % capacity;

    u32 curr_dist = 0;

    Value_T *result = NULL;

    while (true)
    {
      // Not able to find it, found an empty slot
      if (distances[index] == 0xFFFFFFFF)
      {
        break;
      }

      // Not able to find it, found a spot farther away than possible
      if (distances[index] < curr_dist)
      {
        break;
      }

      if (hashes[index] == hash && string_match(keys[index], key))
      {
        result = values + index;
        break;
      }

      curr_dist += 1;
      index = (index + 1) & (capacity - 1);
    }

    return result;
  }

  Value_T *find_avx2(String key)
  {
    u32 hash  = hash_string(key);
    u32 index = hash % capacity;

    Value_T *result = NULL;

    __m256i search_mask = _mm256_set1_epi32(hash);
    __m256i empty_mask  = _mm256_set1_epi32(0xFFFFFFFF);
    // __m256i dist_mask   = _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7);

    while (true)
    {
      __m256i check_hashes = _mm256_loadu_si256((__m256i *)&hashes[index]);
      __m256i check_dists  = _mm256_loadu_si256((__m256i *)&distances[index]);

      __m256i _matches = _mm256_cmpeq_epi32(check_hashes, search_mask);
      i32 matches = _mm256_movemask_epi8(_matches);

      while (matches)
      {
        i32 first_bit = __builtin_ctz(matches);
        i32 slot = first_bit / 4;
        slot = (index + slot) & (capacity - 1);

        if (string_match(keys[slot], key))
        {
          result = values + slot;
          return result;
        }

        matches &= matches - 1; // And clear the last match
      }

      __m256i _empty = _mm256_cmpeq_epi32(check_dists, empty_mask);
      i32 empties = _mm256_movemask_epi8(_empty);

      if (empties)
      {
        break;
      }

      index = (index + 8) & (capacity - 1);
    }

    return result;
  }
};
