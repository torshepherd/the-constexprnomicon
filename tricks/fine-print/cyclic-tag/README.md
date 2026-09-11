# A cyclic tag machine in exception specifications

[The source](cyclic-tag.cpp) is a generic interpreter made from three binary
transition/halting overloads, plus two optional Boolean-output halts. All return
`void`; none has a body. It uses the [Fine print mechanism](../).

## The machine

A cyclic tag system has a queue of bits and a nonempty ring of production words.
Each step removes the queue's first bit. If that bit was 1, append the active
production to the queue; if it was 0, append nothing. Advance to the next
production, wrapping around. An empty queue halts.

Our C++ encoding maps these operations directly:

| Machine component | C++ representation or operation |
| --- | --- |
| Queue | `word<'1','0',...>` |
| Production ring, with current rule first | `cycle<word<...>, word<...>, ...>` |
| Read and delete first bit | Match `word<'0', Tail...>` or `word<'1', Tail...>`. |
| Append current production | Form `word<Tail..., Append...>`. |
| Advance the ring | Form `cycle<Rest..., word<Append...>>`. |
| Continue | Query the next configuration inside the selected exception specification. |
| Halt on empty queue | The terminal overload declares `noexcept`. |

The two transition declarations are:

```cpp
template<char... Append, class... Rest, char... Tail>
void run(cycle<word<Append...>, Rest...>, word<'0', Tail...>)
    noexcept(noexcept(run(cycle<Rest..., word<Append...>>{}, word<Tail...>{})));

template<char... Append, class... Rest, char... Tail>
void run(cycle<word<Append...>, Rest...>, word<'1', Tail...>)
    noexcept(noexcept(run(cycle<Rest..., word<Append...>>{}, word<Tail..., Append...>{})));
```

Changing programs means changing the type-pack of production words. The
interpreter declarations remain fixed; there is no pre-unrolled execution trace.
The empty class templates contain no recursive definitions or member machinery.

For the ring `[11, empty, empty]`, starting with `11`:

| Step | Active production index | Queue |
| --- | --- | --- |
| 0 | 0 | `11` |
| 1 | 1 | `111` |
| 2 | 2 | `11` |
| 3 | 0 | `1` |
| 4 | 1 | `11` |
| 5 | 2 | `1` |
| 6 | 0 | empty: halt |

The queue grows, previously appended bits are consumed, and the first production
is reused. The source needs no declaration for steps 1 through 6.

## Reading a Boolean answer

In the binary model, successful empty-queue termination yields true. This is not
a total test of whether arbitrary programs halt: a nonterminating computation
does not become false.

Two extra terminal overloads allow programs to emit distinct Boolean results:

```cpp
template<class... Rules, char... Tail>
void run(cycle<Rules...>, word<'Y', Tail...>) noexcept;
template<class... Rules, char... Tail>
void run(cycle<Rules...>, word<'N', Tail...>) noexcept(false);
```

`Y` and `N` are optional **halting markers** here; the ordinary data alphabet is
`0` and `1`. Cook's paper uses Y/N to name its two ordinary bits; we have renamed
those to 1/0. Do not confuse that notation with this output extension.

With productions `[Y, N]`, input `1` returns true; input `01` advances past the
first rule, appends N using the second, and returns false. Neither throws an
exception. The final specification is just the result channel.

## Computational-power claim

The allowed operations are the same explicit combination as in Fine print:
function-template deduction/overload selection, pack substitution, and lazy
exception-specification propagation. They provide selection, evolving memory,
and recurrence without intermediate value extraction or another evaluator.
This meets the project's [roughly-Turing criteria](../../../docs/Pedantics.md)
for that combined mechanism.

There is also a standard-model argument: restricting the source to binary
queues and productions gives the transition rules of cyclic tag systems, whose
universality Cook establishes in section 2.2 of
[Universality in Elementary Cellular Automata](https://content.wolfram.com/sites/13/2023/02/15-1-1.pdf).
Thus the declaration scheme can simulate that universal model, subject to
compiler resources. The two small example programs themselves are not claimed
to be universal, and no general Turing-machine-to-tag compiler or arbitrary
output-tape decoder is supplied here.

This is a semantic translation argument plus bounded compiler evidence, not a
claim that a finite compiler can run forever or decide nontermination. Output
here is a terminal Boolean. No claim of a complete *single-feature* machine is
made by calling this combined construction roughly-Turing.

## Reproduce and boundaries

```sh
g++ -std=c++11 -O0 -Wall -Wextra -pedantic-errors -fconstexpr-depth=1 -fsyntax-only tricks/fine-print/cyclic-tag/cyclic-tag.cpp
python3 tricks/fine-print/experiments/check.py
```

GCC 13.3, GCC 16.2, and Clang 22.1.0 pass the source and all 940 terminating
generated cyclic cases. See the [parent matrix](../README.md#reproduce) for
exact modes, exclusions, and the independent checker.

Use a nonempty production ring and the declared alphabet. Empty production
*words* are supported; an empty production *ring* cannot process a nonempty
queue. Unknown symbols are ill-formed. Each configuration occupies compiler
template state: there is no zero-storage or efficient-interpreter claim.

The [controls](../experiments/controls.cpp) distinguish a finite false result,
a self-dependent cycle, growing recursion, unknown input, and a missing program.
Only the first yields a valid false query. The earlier
[two-tag sketch](../experiments/two-tag.cpp) is retained as research history;
its productions were written as separate overloads, whereas this interpreter
accepts the production ring as data.
