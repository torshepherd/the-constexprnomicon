# Empty bits

[Source](empty-bits.cpp) · [Godbolt](https://godbolt.org/z/cxqejscT1)

The class inherits 64 distinct empty base types. On the tested target it
satisfies both `sizeof(integer) == 1` and `std::is_empty_v<integer>`.

Each base's lifetime represents a bit. Destroying the base clears the bit;
constructing it sets the bit. Clang's constant evaluator distinguishes a live
base from a dead one when `__builtin_constant_p` probes a non-static member call,
even though that member simply returns true.

The payload is supplied and changed through ordinary function arguments during
constant evaluation. Two objects of the same type can hold different values.
The template arguments identify bit positions, not the stored number.

The destructor revives the bases before their implicit destruction. The state
belongs to evaluator lifetime bookkeeping, so copying the wrapper's bytes does
not copy that state; keep the wrapper uncopied and use it only in constant
evaluation. This relies on Clang's handling of empty-base lifetime replacement.
It is an observed compiler-extension result, not a portability claim or a way
to compress runtime integers.

GCC 16.2 rejects this example. The checks on Clang cover two independent objects,
sixteen generated patterns, repeated zero/all-one writes, mutation, and
destruction.

## Try it

Recorded compiler evidence: **Clang 22.1.0 (GCC rejects the example)**, C++23.
Run from the repository root; executable names depend on your installation.

```sh
clang++-22 -std=c++23 -O2 -c tricks/empty-bits/empty-bits.cpp -o /tmp/empty-bits.o
```

See [shared provenance](../../docs/PROVENANCE.md) for known ingredients and related work.
