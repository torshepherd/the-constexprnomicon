"""Independent bounded checks. --source prints a standalone C++11 test file."""
import argparse
from collections import Counter, deque
import itertools
from pathlib import Path
import subprocess
import tempfile


def accepts_parentheses(text):
    depth = 0
    for char in text:
        depth += 1 if char == '(' else -1
        if depth < 0:
            return False
    return depth == 0


def reference(rules, text):
    queue = deque(text)
    pc = 0
    seen = set()
    for _ in range(80):
        if not queue or queue[0] == 'Y':
            return 'true'
        if queue[0] == 'N':
            return 'false'
        state = pc, tuple(queue)
        if state in seen:
            return 'cycle'
        seen.add(state)
        if queue.popleft() == '1':
            queue.extend(rules[pc])
        pc = (pc + 1) % len(rules)
    return 'limit'


def word(text):
    return 'word<' + ','.join(repr(c) for c in text) + '>'


def source():
    root = Path(__file__).resolve().parents[1]
    out = ['namespace parser {', (root / 'fine-print.cpp').read_text()]
    parser_count = 0
    for length in range(9):
        for chars in itertools.product('()', repeat=length):
            text = ''.join(chars)
            expected = str(accepts_parentheses(text)).lower()
            out.append(f'static_assert(noexcept(balanced({word(text)}{{}}, word<>{{}})) == {expected}, "parser {text}");')
            parser_count += 1
    out.extend(['}', 'namespace machine {', (root / 'cyclic-tag/cyclic-tag.cpp').read_text()])
    outcomes = Counter()
    productions = ['', '0', '1', '00', '01', '10', '11', 'Y', 'N']
    for rules in itertools.product(productions, repeat=2):
        for length in range(4):
            for chars in itertools.product('01', repeat=length):
                text = ''.join(chars)
                result = reference(rules, text)
                outcomes[result] += 1
                if result not in ('true', 'false'):
                    continue  # Never treat nontermination or exhaustion as false.
                program = 'cycle<' + ','.join(word(r) for r in rules) + '>'
                out.append(f'static_assert(noexcept(run({program}{{}}, {word(text)}{{}})) == {result}, "cyclic tag");')
    out.append('}')
    return '\n'.join(out) + '\n', parser_count, outcomes


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source', action='store_true')
    parser.add_argument('--compiler', default='g++')
    args = parser.parse_args()
    text, count, outcomes = source()
    if args.source:
        print(text, end='')
    else:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'checks.cpp'
            path.write_text(text)
            subprocess.run([args.compiler, '-std=c++11', '-O0', '-Wall', '-Wextra',
                            '-pedantic-errors', '-fconstexpr-depth=1',
                            '-ftemplate-depth=128', '-fsyntax-only', str(path)],
                           check=True, timeout=30)
        print(f'{count} parentheses cases; cyclic-tag reference outcomes: {dict(outcomes)}')
