# Pedantics

Tor's instructions for this project's computational-power experiments. The
quotations below preserve his messages verbatim, with Markdown blockquote
markers added. They are the source of the rules; the editorial status note at
the end is separate. Read this before classifying an isolated technique as
roughly-Turing.

## The original definition — verbatim

> Now, I have known for a while that template argument deduction and overload resolution are each independently Turing-complete. Or rather, TAD is but OVL is not quite - there was something about recursion that didn’t quite work for me.
>
> Is this new technique you found “roughly Turing”? You might want to update the notes with my personal definition of roughly-Turing: has either recursion or iteration and memory.

## Isolation, extraction, and allowed operations — verbatim

> Also, now that I'm thinking about it, this whole thing about roughly Turing definition and stuff like that, I feel like this should live in a new file called Pedantics.md, because I like to be a bit pedantic about these things. So one thing that I like to be pedantic about is that I like to try to think about a technique in isolation. So what I mean by that is, if I am looking at, for example, how to make class template argument deduction Turing complete, or roughly Turing complete rather, then what I really care about is: can we do every single point of the program using only class template argument deduction? So, for example, if you can store things to a template argument using deduction, but you have to recover it by actually calling, like, a function, like dot value, or using a member accessor, then that actually, in my mind, is considered cheating, and we can't consider that a win for CTAD. Now, my working example of CTAD-only programming, which I believe used merge sort in CTAD-only, actually included this cheat. So it wasn't quite real, because it had that. But I think recently I found a way that we can both lift values and recover values all using CTAD, and then only use another technique to get, like, the final value out of the program. But I guess that's a topic for another day. And maybe you could just start by, like, writing all this stuff down in Markdown files in the working notes. It would be great if you could write down some of the stuff that I'm saying here mostly verbatim, as these are instructions, and I really don't want them to get translated or, you know, rephrased into your words.
>
> Applied to the thing that you just showed me, I would say that this is considered cheating. You are doing conditioning using if constexpr. That's a separate technique, right? So the actual technique itself, in this case, the hidden copy thing, is not actually the thing that's driving the selection. Now, note the thing that you showed me, overload resolution. You could say, for example, hidden copy plus overload resolution. If you define that to be the technique, then I think you can do this without cheating, right? Assuming that you can get recursion. But if you're talking about, like, specialization, then, you know, that's yet another technique. More cheating. Do you see what I mean by this? Like, I like the mental game of defining this minimal set of operations that I'm allowed, and seeing if there is a machine in there. And so it's kind of like a self-measure of, like, can we express all of the things using a single technique? Not by, like, mixing and matching.

## Selection / conditional logic — verbatim

> Also, of course, roughly-Turing has to have a method of selection/conditional logic

## Current status — editorial, not a quotation

The combined criterion is **(recursion or iteration) + memory + selection /
conditional logic**, with all three expressed within the chosen allowed
operations. The quoted instructions distinguish intermediate value recovery
from observing the final result.

- The earlier affirmative assessment of the copy-driven factorial is withdrawn
  as a claim about an isolated technique. The program uses `if constexpr` for
  decisions, member reads and constexpr arithmetic for transitions, and
  return-type deduction and template instantiation for recursion. Its compiler
  results still stand. See the [corrected assessment](09-copy-gate.md#roughly-turing-assessment).
- “Hidden copy plus overload resolution” is a proposed explicit choice of
  allowed techniques. A complete machine, including recursion within that
  choice, has not been demonstrated here. Adding specialization or another
  mechanism would change the allowed set rather than silently solve it.
- Tor's earlier CTAD example, recalled as merge sort, used internal value
  extraction that he now identifies as cheating. His possible newer route for
  lifting and recovering values entirely through CTAD is for a future session;
  no source or verification for it was supplied here. Preserve the distinction
  he draws between intermediate extraction and observing the final result.

The “secret equality” [research direction](09-copy-gate.md#follow-up-the-compilers-secret-equality)
remains open under these instructions.
