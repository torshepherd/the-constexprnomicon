"""Generate small SAT instances; cross-check subsumption against brute force.

Python constructs test inputs and expected answers, not the C++ solving step.
Each generated C++ query has two constrained NON-TEMPLATE member declarations.
There are no C++ function definitions or template-recursive solving helpers.
"""
import itertools
import json
import random
import subprocess
import sys
import urllib.request


def truth(n, clauses):
    return any(all(any(bool(mask & (1 << (abs(lit) - 1))) == (lit > 0)
                       for lit in clause) for clause in clauses)
               for mask in range(1 << n))


def emit(number, n, clauses):
    def atom(lit):
        return f'{"P" if lit > 0 else "N"}{abs(lit)}<T>'
    atoms = '\n'.join(f'template<class> concept {sign}{i} = true;'
                      for i in range(1, n + 1) for sign in ('P', 'N'))
    bad = ' || '.join(f'(P{i}<T> && N{i}<T>)' for i in range(1, n + 1))
    formula = ' && '.join('(' + ' || '.join(map(atom, c)) + ')'
                          for c in clauses) or 'true'
    expected = str(truth(n, clauses)).lower()
    return f'''namespace case_{number} {{
{atoms}
template<class T> concept Bad = {bad};
template<class T> concept F = {formula};
template<class T> struct Oracle {{
    void solve() requires Bad<T>;
    void solve() requires (F<T> && true);
}};
template<class T> concept SAT = !requires(Oracle<T> o) {{ o.solve(); }};
static_assert(F<int> && Bad<int>);
static_assert(SAT<int> == {expected});
}}
'''


clauses2 = [(1,), (-1,), (2,), (-2,), (1, 2), (1, -2), (-1, 2), (-1, -2)]
cases = [(2, [c for i, c in enumerate(clauses2) if mask & (1 << i)])
         for mask in range(256)]
rng = random.Random(20260908)
for n in (3, 4):
    for _ in range(64):
        clauses = [tuple(rng.choice((-1, 1)) * i for i in
                         rng.sample(range(1, n + 1), rng.randint(1, min(n, 3))))
                   for _ in range(rng.randint(1, 8))]
        cases.append((n, clauses))

# Repetition, a tautology, an explicit contradiction, equivalent Bad,
# and a disconnected variable. In particular, the guard must resolve F=Bad.
cases.extend([(2, [(1, -1)]), (2, [(1,), (-1,)]),
              (2, [(1,), (1,), (2, -2)]),
              (2, [(1, 2), (1, -2), (-1, 2), (-1, -2)])])
source = '\n'.join(emit(i, n, c) for i, (n, c) in enumerate(cases))
source += '''
namespace equivalence_control {
template<class> concept A = true;
template<class> concept NA = true;
template<class T> concept Bad = A<T> && NA<T>;
template<class T> struct Oracle {
    void solve() requires Bad<T>;
    void solve() requires (Bad<T> && true);
};
template<class T> concept SAT = !requires(Oracle<T> o) { o.solve(); };
static_assert(!SAT<int>);
}
'''

if '--source' in sys.argv:
    print(source)
    raise SystemExit
print(json.dumps({'cases': len(cases) + 1,
                  'satisfiable': sum(truth(n, c) for n, c in cases),
                  'unsatisfiable': sum(not truth(n, c) for n, c in cases) + 1,
                  'source_bytes': len(source)}), flush=True)
if len(sys.argv) == 1:
    result = subprocess.run(['g++', '-x', 'c++', '-std=c++20', '-Wall', '-Wextra',
                             '-pedantic-errors', '-fsyntax-only', '-'],
                            input=source, text=True, capture_output=True, timeout=40)
    print(json.dumps({'compiler': 'local g++ 13.3', 'code': result.returncode,
                      'stderr': result.stderr}))
    raise SystemExit(result.returncode)
compiler = sys.argv[1]
payload = {'source': source, 'lang': 'c++', 'options': {
    'userArguments': '-std=c++20 -O2 -Wall -Wextra -pedantic-errors',
    'compilerOptions': {'skipAsm': True}, 'filters': {'execute': False}}}
request = urllib.request.Request(f'https://godbolt.org/api/compiler/{compiler}/compile',
    data=json.dumps(payload).encode(),
    headers={'Accept': 'application/json', 'Content-Type': 'application/json'})
with urllib.request.urlopen(request, timeout=55) as response:
    result = json.load(response)
print(json.dumps({'compiler': compiler, 'code': result.get('code'),
                  'stderr': result.get('stderr', [])}))
