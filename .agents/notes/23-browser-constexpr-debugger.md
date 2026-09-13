# Browser constexpr debugger: prior art and feasibility

September 13, 2026. Research and discussion only; no WebAssembly compiler build
or new application was attempted in this session.

Tor asked:

> Let’s just research the following then discuss. Is there any way we could compile GCC with our introspection tools to web assembly so there could be an interactive page where GCC dumps a json file or something of our steps, then the web page could be similar in vibe to godbolt but just compile in wasm locally?
>
> Let’s just start with prior art search maybe?

And clarified:

> My thinking here btw is we could extend this concept to make an interactive web demo of the constexpr caching through memoization trick too. And all of the other builtin constant p.
>
> Like a web debugger for contexpr in a way?

The research checkout was fast-forwarded from the previous instrumentation
commit to `eccad4657d0c06482233c80e8cb74b6a7c8cb30c`, preserving the concurrent
constantness-alternatives documentation and examples.

## Closest prior art

- **[WasmBolt](https://github.com/Anutosh21/WasmBolt)** is directly relevant to
  the proposed user experience. Its source README describes Clang/LLVM/LLD
  embedded in an Emscripten application, source editing, AST/IR/assembly output,
  Graphviz views, browser files, draggable panes, and local compilation.
  The author's [LLVM RFC](https://discourse.llvm.org/t/rfc-embeddable-llvm-tool-drivers-for-long-lived-hosts/91754)
  was created September 8, 2026. It discusses problems with repeated tool
  invocation and ownership of global compiler state. This is LLVM prior art,
  not evidence that GCC-specific cache tricks work on Clang. The source README
  and RFC were read; the live deployment was not exercised.
- **[@horang-corp/avr-gcc-wasm 0.2.0](https://github.com/horang-corp/avr-gcc-wasm)**
  publishes a real GCC C++ frontend as `cc1plus.wasm`, together with AVR
  binutils. Its [published README](https://cdn.jsdelivr.net/npm/@horang-corp/avr-gcc-wasm@0.2.0/README.md)
  describes direct frontend invocation, MEMFS, and a fresh Worker per build.
  It reports an approximately 14 MB stripped frontend and 40.8 MB peak frontend
  linear memory for its firmware workload; those are the author's measurements,
  not estimates or measurements for our compiler. It avoids the GCC driver
  because of process dependencies such as `vfork`. The repository tree at
  `e3a563f765b041623734991125d5640c7e56053e` includes binaries and asset-generation
  scripts, but no compiler-source port/build recipe was identified. The read
  metadata did not establish the GCC release. This is strong feasibility
  precedent, not a verified reusable modern x86-64-target GCC build.
- **[binji/wasm-clang](https://github.com/binji/wasm-clang)**, from a CppCon 2019
  talk, runs Clang and LLD compiled to WASI with an in-memory filesystem and a
  dedicated worker. It includes C++ headers/libraries and x86/Wasm assembly
  exploration. Its README calls it alpha demoware and links build notes.
- **[Emception](https://github.com/jprendes/emception)** runs Emscripten's
  C/C++ toolchain in the browser, with published build scripts. Its `llvm-box`
  combines several LLVM tools to share code and reduce download size. Useful
  build/packaging precedent, though substantially broader than a constexpr
  observer needs.
- **[CLion's Constexpr Debugger](https://www.jetbrains.com/help/clion/constexpr-debugger.html)**
  supplies direct debugger UX precedent: stepping, backward stepping, locals,
  call stack, template arguments, and failure inspection. Introduced in 2025.3;
  documentation updated August 7, 2026 still lists unsupported constructs and
  no breakpoints/run-to-cursor. This is CLion's evaluator and does not establish
  observability of GCC's actual memoization table.

Additional leads retained:

- [romdev](https://github.com/monteslu/romdev) publishes m68k/MIPS GCC tools as
  Wasm packages. The inspected m68k package is C-only (`cc1`, not `cc1plus`).
  Its old package README points to root `BUILDING.md`/`scripts/versions.json`,
  which returned 404. Current files moved under `packages/romdevtools/scripts`.
  [build-m68k-toolchain.sh](https://github.com/monteslu/romdev/blob/main/packages/romdevtools/scripts/build-m68k-toolchain.sh)
  pins GCC 14.2 but builds the native C toolchain and labels Wasm stage 2
  forthcoming. Do not cite this script as a complete reproduced Wasm port.
- [saarraz/static-print](https://github.com/saarraz/static-print) patches GCC
  7.1 to add compile-time printing. A Stack Overflow constexpr-debugging lead
  resolved to this, not to a full stepping debugger.
- [Metashell](https://github.com/metashell/metashell) is adjacent compile-time
  metaprogramming-shell prior art, not proof of a GCC constexpr heap/cache view.
- GCC WebAssembly-backend work (e.g.
  [feedab1e/gcc-wasm](https://github.com/feedab1e/gcc-wasm)) concerns the output
  target. It is a different issue from compiling GCC itself to run in Wasm.
  General Emscripten tutorials and old forum opinions about porting difficulty
  do not establish whether our particular GCC frontend build works.

## Feasibility assessment

The architecture is credible. The unverified milestone is a reproducible
modern GCC C++ frontend built for a Wasm host while retaining an x86-64 target,
with our hooks and the expected behavior of each spell.

[GCC distinguishes build, host, and target](https://gcc.gnu.org/onlinedocs/gccint/Configure-Terms.html).
For this project the browser could run a wasm32-hosted compiler whose target
still has eight-byte pointers. The symbolic Astral heap remains allocation
identities and wide integer offsets; it does not require a 65-bit Wasm memory.
This is an architectural inference, not a tested cross-build result. Host
memory/stack limits and compiler portability issues still require measurement.

The initial proposed compiler entry point is patched `cc1plus` with
`-fsyntax-only`, as in our native capture. This avoids needing to execute the
user program, or supplying an assembler/linker. It does not automatically
strip all GCC backend code from the binary. Header-free spells can be the
first input; examples using the library need matching target headers.

Emscripten already supplies the plumbing:
[MEMFS and read/write APIs](https://emscripten.org/docs/api_reference/Filesystem-API.html),
[stderr callbacks](https://emscripten.org/docs/api_reference/module.html), and
[independent module instances](https://emscripten.org/docs/compiling/Modularized-Output.html).
Write the editor source into the virtual filesystem, invoke the compiler in
a Worker, collect JSON-lines output, then inspect it locally. A JSON output
file is also possible. Stream events in batches if desired; a separate download
is not needed for the viewer to consume them.

## Broader debugger design, proposed rather than implemented

The observer should have one event model with views for call stacks, local
values, heap/subobject lifetimes, memoization, and constantness probes.

- **Cache:** record actual keys, lookup hits/misses, depth-based cache bypass,
  in-progress entries, stored results/failures, and reuse. GCC 13.3's
  `cxx_eval_call_expression` hashes function identity, argument bindings, and
  manifest-constant-evaluation mode. The current instrumentation capture used
  `-fconstexpr-cache-depth=0`; that setting must not carry into cache demos.
- **Probes:** record the attempted expression, evaluation context, returned
  `__builtin_constant_p` result, and the observed failure/limit when available.
  [GCC documents](https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html) that zero
  means GCC could not establish constantness under the active options, not a
  universal proof that the expression can never be constant.
- **Lifetimes:** show active/inactive union members and live/dead subobjects,
  where each compiler exposes them. The actual current heap logger suppresses
  events before `heap_vars` is nonempty, so it cannot simply be reused unchanged
  to observe heap-free cache/lifetime tricks.
- **Scope:** preserve events across all constant evaluations in one translation
  unit, including failed/speculative attempts and optional folding. A single
  source statement can correspond to multiple attempts, or a cached result
  with no new execution of its body. Source stepping must show that honestly.
- **Replay first:** compile to a trace, then step forward/backward over that
  captured history. True live pause/resume could follow, but requires additional
  suspension machinery such as Emscripten
  [Asyncify](https://emscripten.org/docs/porting/asyncify.html). No assertion that
  simply calling `main()` repeatedly or keeping the runtime alive makes GCC
  safely reentrant.
- **Observer effect:** watch views should read captured compiler state. Calling
  the evaluator again to compute a watch could warm exactly the cache being
  demonstrated. The logger must not add target expressions, alter constexpr
  operation accounting, or change the flags being studied. Trace volume needs
  caps and preferably deltas/interned nodes rather than whole-heap snapshots
  on every recursive call.
- **Run boundaries:** use a fresh instance/Worker for each whole compilation,
  preserving caches inside that run. Browser-cached compiler bytes can be reused
  independently of GCC's semantic memoization state. This also supplies a clear
  cancellation and memory-reclamation boundary.

GCC alone will not reproduce the whole collection. The current Empty bits
README records Clang 22.1 success and GCC rejection; its C++26 query variant
also fails the GCC trunk value checks. The eventual shared viewer could accept
compiler-specific event adapters: GCC first for Astral heap and memoization,
Clang for lifetime-based constructions. Existing browser Clang ports help with
hosting, but their internals still need explicit instrumentation.

The bounded next experiment, if requested after discussion, is to establish
the reproducible GCC frontend port, run a tiny ordinary constant evaluation,
then verify Astral heap and the memoization counter against native results.
No UI implementation, compiler download/build, deployment, or live-debugger
promise is implied by this prior-art research.

## Optional follow-up: compiler activity timeline

Tor added:

> Awesome. I’d love to extend the research notes a tiny bit… to say that it would be awesome if we could include in this debugger a “perfetto-inspired” timeline trace of compiler activity… will be neat for showing why certain compile time programs are so slow and some faster?
>
> This is not essential to the core concept though at all

A Perfetto-inspired, zoomable timeline could show durations and nested compiler
activity, with source-linked evaluation attempts, cache hits/misses, and repeated
work. Comparing traces of different implementations could help explain why one
compile-time program is slower than another. Keep measured durations distinct
from operation counts and account for tracing overhead when comparing timings.
This is an optional exploration and teaching feature, not a prerequisite for
the core debugger or the initial Wasm proof of concept.
