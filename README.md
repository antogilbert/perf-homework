# perf homework

## intrinsics
try to convert all code using intrinsics _mm512_load & _mm512_mul (512
is the num of bits) look at intrinsics

`lscpu | grep avx512` returns nothing, so my laptop does not support avx 512.
`lscpu | grep avx2` is present, so we use _mm256 instructions

I needed to use the `u` variant of both load and store because i couldn't 
guarantee 32 byte alignment.

##
perf record -g

the sample was probably too small so everything was inlined

let's put all together in gbench or quick-bench.com and re run it

use aligned_alloc next time and do not use the 'u' version of the calls

### FUCK YOU MOMENTS
aligned_alloc's alignment parameter is in BITS
intrinsincs load/store's alignment is in BYTES

Setting up variable size in benchmark with the difference above,
Off by one block
