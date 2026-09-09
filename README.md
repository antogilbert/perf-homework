# perf homework

## intrinsics
try to convert all code using intrinsics _mm512_load & _mm512_mul (512
is the num of bits) look at intrinsics

`lscpu | grep avx512` returns nothing, so my laptop does not support avx 512.
`lscpu | grep avx2` is present, so we use _mm256 instructions

I needed to use the `u` variant of both load and store because i couldn't 
guarantee 32 byte alignment.


