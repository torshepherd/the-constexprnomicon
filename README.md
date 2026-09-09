# The Constexprnomicon

A grimoire of forbidden C++, conjured by human and machine.

## Standalone tricks

| Spell | The trick |
| --- | --- |
| [One-pointer vector](tricks/one-pointer-vector/) | Union lifetimes hide size and capacity behind one pointer. |
| [Empty bits](tricks/empty-bits/) | An empty class carries 64 writable logical bits. |
| [GCC memoization](tricks/gcc-memoization/) | Turn the compiler's cache into a counter, dictionary, or mutable vector—even one you can sort. |
| [Copy gate](tricks/copy-gate/) | A copy constructor sorts a template argument; overload resolution admits only already-sorted words. |
| [Last rites](tricks/last-rites/) | Temporary destructors run backpropagation: the semicolon differentiates an expression. |
| [False idols](tricks/false-idols/) | Overload ambiguity decides SAT, though every atomic constraint is literally true. |
| [Chromatic aberration](tricks/chromatic-aberration/) | Empty-base optimization colors a graph; byte offsets are colors. |
| [Seance](tricks/seance/) | A false concept leaves a runtime ghost in a program with an empty `main`. |
| [Astral heap](tricks/astral-heap/) | Eight-byte pointers address 32 exbibytes of live constexpr storage. |

Each folder contains the source and a walkthrough, including compiler requirements
and commands. Sources stand alone; most use `static_assert` checks, while Seance
also needs execution. These are experiments, with compiler-specific caveats.

## Roughly-Turing-complete features

Can one language mechanism supply selection, memory, and recurrence? See the
[machine investigations](docs/MACHINES.md) and [Pedantics](docs/Pedantics.md).
No complete isolated machine is currently demonstrated by the examples tracked there.

---

By Tor Shepherd and OpenAI's Codex. An independent companion to
[sorcery-cpp](https://github.com/torshepherd/sorcery-cpp), starting from
[Static Antics](https://github.com/torshepherd/static_antics).
[Provenance](docs/PROVENANCE.md) · [Collaborating](AGENTS.md)
