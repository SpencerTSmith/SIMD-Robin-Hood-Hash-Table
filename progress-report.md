# Done
- Main robin-hood hashing algorithm finished
    - Find
    - Insert
- Initial SIMD implementation
    - Find
- Benchmarking against std::unordered_map
- Basic virtual memory arena

| Function | Implementation  | # of operations |  Latency (TSC)    | Throughput   |
|----------|-----------------|-----------------|-------------------|--------------|
| Insert   |  ours-scalar    | 2048            | 63814 (0.0228ms)  | 0.6703 GB/s  |
| Insert   |  unorderd_map   | 2048            | 206028 (0.0735ms) | 0.2076 GB/s  |
| Find     |  ours-scalar    | 2048            | 90924 (0.0324ms)  | 0.4704 GB/s  |
| Find     |  ours-AVX2      | 2048            | 66507 (0.0237ms)  | 0.6431 GB/s  |
| Find     |  unorderd_map   | 2048            | 138201 (0.0493ms) | 0.3095 GB/s  |


# To Do
- Take more advantage of Virtual Memory arena
    - 2 level indexing, split backing arrays into page sized parts only commit sections as needed by table
        - Would serve as early out too... check if top page sized bucket is even mapped
- SIMD implementation for insert
- Benchmark against google swiss table
- Better SIMD Implementations for find
- Experiment with different memory layouts
    - Currently each piece of data is stored in a separate array.
    - But currently that pollutes the cache a bit when loading hashes, as we also need to load from the distances array for checking if we haven't found it yet.
    - Perhaps can store distances and hashes together interleaved as we are checking both quite frequently
