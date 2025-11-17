# Done
- Main robin-hood hashing algorithm finished
    - Find
    - Insert
- Initial SIMD implementation
    - Find
- Benchmarking against std::unordered_map

| Function | Implementation  | # of operations |  Latency (TSC)    | Throughput   |
|----------|-----------------|-----------------|-------------------|--------------|
| Insert   |  ours-scalar    | 2048            | 63814 (0.0228ms)  | 0.6703 GB/s  |
| Insert   |  unorderd_map   | 2048            | 206028 (0.0735ms) | 0.2076 GB/s  |
| Find     |  ours-scalar    | 2048            | 90924 (0.0324ms)  | 0.4704 GB/s  |
| Find     |  ours-AVX2      | 2048            | 66507 (0.0237ms)  | 0.6431 GB/s  |
| Find     |  unorderd_map   | 2048            | 138201 (0.0493ms) | 0.3095 GB/s  |

- Basic virtual memory arena

# To Do
- Take more advantage of Virtual Memory arena
- SIMD implementation for insert
- Benchmark against google swiss table
-
