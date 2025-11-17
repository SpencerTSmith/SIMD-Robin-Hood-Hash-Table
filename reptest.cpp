#define COMMON_IMPLEMENTATION
#include "common.h"

#include "benchmark/repetition_test.c"
#include "benchmark/platform_timing.c"
#include "robin-hood.cpp"

struct vec3
{
  f32 x, y, z;
};

struct Entity
{
  u64  flags;
  vec3 position;
  vec3 scale;
  vec3 rotation;

  u64 resource_handle0;
  u64 resource_handle1;
};

struct Operation_Parameters
{
  String *keys;
  Entity *values;
  usize  count;
};

#include <unordered_map>

// CPP BULLLLLSHIIITT
namespace std
{
template<>
struct hash<String>
{
  size_t operator()(const String& s) const noexcept
  {
    return hash_string(s);
  }
};
template<>
struct equal_to<String>
{
  bool operator()(const String& a, const String& b) const noexcept
  {
      return string_match(a, b);
  }
};
}

static
void insert_unordered_map(Repetition_Tester *tester, Operation_Parameters *params)
{
  std::unordered_map<String, Entity> map;
  map.reserve(params->count);

  while (repetition_tester_is_testing(tester))
  {
    repetition_tester_begin_time(tester);
    for (usize i = 0; i < params->count; i++)
    {
      String key = params->keys[i];
      Entity val = params->values[i];

      map.emplace(key, val);
    }
    repetition_tester_close_time(tester);

    repetition_tester_count_bytes(tester, params->count * 8);

    map.clear();
  }
}

static
void insert_ours(Repetition_Tester *tester, Operation_Parameters *params)
{
  Table<Entity> table = {};
  table.init(params->count);

  while (repetition_tester_is_testing(tester))
  {
    repetition_tester_begin_time(tester);
    for (usize i = 0; i < params->count; i++)
    {
      String key = params->keys[i];
      Entity val = params->values[i];
      table.insert(key, val);
    }
    repetition_tester_close_time(tester);

    repetition_tester_count_bytes(tester, params->count * 8);

    table.clear();
  }

  table.free();
}

static
void find_unordered_map(Repetition_Tester *tester, Operation_Parameters *params)
{
  std::unordered_map<String, Entity> map;
  map.reserve(params->count);

  while (repetition_tester_is_testing(tester))
  {
    for (usize i = 0; i < params->count; i++)
    {
      String key = params->keys[i];
      Entity val = params->values[i];

      map.emplace(key, val);
    }

    repetition_tester_begin_time(tester);
    for (usize i = 0; i < params->count; i++)
    {
      auto it = map.find(params->keys[i]);
      it->second.position.x += 1.0;
    }
    repetition_tester_close_time(tester);

    repetition_tester_count_bytes(tester, params->count * 8);

    map.clear();
  }
}

static
void find_ours_scalar(Repetition_Tester *tester, Operation_Parameters *params)
{
  Table<Entity> table = {};
  table.init(params->count);

  while (repetition_tester_is_testing(tester))
  {
    for (usize i = 0; i < params->count; i++)
    {
      String key = params->keys[i];
      Entity val = params->values[i];
      table.insert(key, val);
    }

    repetition_tester_begin_time(tester);
    for (usize i = 0; i < params->count; i++)
    {
      Entity *found = table.find(params->keys[i]);
      found->position.x += 1.0;
    }
    repetition_tester_close_time(tester);

    repetition_tester_count_bytes(tester, params->count * 8);

    table.clear();
  }

  table.free();
}

static
void find_ours_avx2(Repetition_Tester *tester, Operation_Parameters *params)
{
  Table<Entity> table = {};
  table.init(params->count);

  while (repetition_tester_is_testing(tester))
  {
    for (usize i = 0; i < params->count; i++)
    {
      String key = params->keys[i];
      Entity val = params->values[i];
      table.insert(key, val);
    }

    repetition_tester_begin_time(tester);
    for (usize i = 0; i < params->count; i++)
    {
      Entity *found = table.find_avx2(params->keys[i]);
      found->position.x += 1.0;
    }
    repetition_tester_close_time(tester);

    repetition_tester_count_bytes(tester, params->count * 8);

    table.clear();
  }

  table.free();
}

Operation_Entry test_entries[] =
{
  {String("FIND - ours (AVX2)"),     find_ours_avx2},
  {String("FIND - ours (scalar)"),   find_ours_scalar},
  {String("FIND - unordered_map"),   find_unordered_map},
  {String("INSERT - ours"),          insert_ours},
  {String("INSERT - unordered_map"), insert_unordered_map},
};

String random_string(Arena *arena)
{
  String result = {};
  result.count = 16;
  result.v = arena_calloc(arena, result.count, u8);

  char table[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789";

  for (isize i = 0; i < result.count; i++)
  {
    result.v[i] = table[rand() % STATIC_ARRAY_COUNT(table)];
  }

  return result;
}

Entity random_entity()
{
  Entity entity = {};
  entity.position = {(f32)rand(), (f32)rand(), (f32)rand()};

  return entity;
}

Operation_Parameters make_random_test_paramters(Arena *arena, u32 count)
{
  Operation_Parameters result = {};
  result.count  = count;
  result.keys   = arena_calloc(arena, count, String);
  result.values = arena_calloc(arena, count, Entity);
  for (u32 i = 0; i < count; i++)
  {
    result.keys[i] = random_string(arena);
    result.values[i] = random_entity();
  }

  return result;
}

int main(int arg_count, char **args)
{
  if (arg_count != 3)
  {
    printf("Usage: %s [seconds_to_try_for_min] [item_count]\n", args[0]);
    return 1;
  }

  u32 seconds_to_try_for_min = atoi(args[1]);
  u32 item_count = atoi(args[2]);

  Arena_Args arena_args = {};
  arena_args.reserve_size = GB(1);
  Arena scratch = __arena_make(&arena_args);

  Operation_Parameters params = make_random_test_paramters(&scratch, item_count);

  u64 cpu_timer_frequency = estimate_cpu_timer_freq();

  Repetition_Tester testers[STATIC_ARRAY_COUNT(test_entries)] = {0};

  for (isize i = 0; i < STATIC_ARRAY_COUNT(test_entries); i++)
  {
    Repetition_Tester *tester = &testers[i];
    Operation_Entry *entry = &test_entries[i];

    printf("\n--- %.*s ---\n", String_Format(entry->name));
    printf("                                                          \r");
    repetition_tester_new_wave(tester, params.count * 8, cpu_timer_frequency, seconds_to_try_for_min);

    entry->function(tester, &params);
  }
}
