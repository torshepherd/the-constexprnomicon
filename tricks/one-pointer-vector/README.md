# One-pointer vector

[Source](one-pointer-vector.cpp) · [Godbolt sketch](https://godbolt.org/z/MscKzxdjf)

Each slot is a union of an empty marker and a value. The evaluator tracks which
member is active, so the vector itself stores just one pointer.

The compact version probes the `char empty` member, rather than trying to
recognize an arbitrary `T` as constant. This also works for the tested structs
and pointers. A reserved empty slot always follows the occupied prefix:
`size()` scans to that slot; `capacity()` continues through empty slots until
the allocation ends, then subtracts the reserved slot.

Growing from size `n` allocates `2 * n + 2` slots, of which `2 * n + 1` are
usable. Capacity therefore grows through 1, 3, 7, ... . The final empty slot is
an implementation sentinel; no value of `T` is reserved.

Assigning a new union with an active value fills a slot. Activating `empty`
pops it. The compiler supplies the missing tags, bounds, and lifetimes.

The sketch expects trivially copyable/destructible element types suitable for
these union operations. Checks cover integers, doubles, booleans, small
aggregates, pointers (including null), and a class with a converting constructor.
It does not attempt general container semantics: keep the owning vector
uncopied, pop only when nonempty, and index live elements. Queries scan linearly.
Allocation and deletion happen within the same constant evaluation.

The Godbolt sketch uses C++20 and a shorter demonstration. The repository keeps
the fuller checks and has been checked in C++23.

GCC 13.3 rejects the unchanged source with constant-expression diagnostics for
local variables; use the documented GCC 16.2 or Clang 22.1.0 configurations.
The [cleanup verification](../../.agents/notes/16-repository-cleanup.md#verification)
records that additional local check.

## Try it

Recorded compiler evidence: **GCC 16.2 and Clang 22.1.0**, C++23.
Run from the repository root; executable names depend on your installation.

```sh
g++-16 -std=c++23 -O2 -c tricks/one-pointer-vector/one-pointer-vector.cpp -o /tmp/one-pointer-vector.o
```

See [shared provenance](../../docs/PROVENANCE.md) for known ingredients and related work.
