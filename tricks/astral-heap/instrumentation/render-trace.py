#!/usr/bin/env python3
"""Render observed ASTRAL lines; do not simulate GCC or invent missing entries.

The compact viewer is specific to trace-example.cpp's one-field byte realms.
The raw JSON-lines trace is the authoritative record for other C++ object shapes.
"""
import argparse
import gzip
import json
from pathlib import Path


def nodes(tree):
    if not isinstance(tree, dict):
        return
    yield tree
    for entry in tree.get('entries', []):
        yield from nodes(entry.get('index'))
        yield from nodes(entry.get('value'))
    for operand in tree.get('operands', []):
        yield from nodes(operand)


def byte_array(tree):
    # Require the source member named bytes and then its actual constructor.
    for node in nodes(tree):
        for entry in node.get('entries', []):
            idx, val = entry.get('index') or {}, entry.get('value') or {}
            if idx.get('kind') == 'field_decl' and idx.get('name') == 'bytes' and val.get('kind') == 'constructor':
                assert val['type_kind'] == 'array_type'
                assert not val.get('truncated')
                items = []
                for e in val['entries']:
                    assert e['index']['kind'] == e['value']['kind'] == 'integer_cst'
                    items.append({'index': e['index']['decimal'], 'value': e['value']['decimal']})
                return {'node':val['node'], 'count':val['entry_count'],
                        'no_clearing':val['no_clearing'], 'entries':items}


def pointer(tree, ids):
    # Exact shape observed in trace-example.cpp, not a general pointer decoder.
    assert tree['kind'] == 'pointer_plus_expr'
    base, offset = tree['operands']
    assert offset['kind'] == 'integer_cst'
    decls = [n['decl_uid'] for n in nodes(base) if n.get('kind') == 'var_decl' and n['decl_uid'] in ids]
    assert len(decls) == 1
    return {'allocation':decls[0], 'offset':offset['decimal'], 'tree':tree}


def compact(events):
    # This workload must be one evaluator run; don't merge repeated evaluations.
    assert len({e['evaluation'] for e in events}) == 1
    pointers, output = {}, []
    initial = None
    for event in events:
        heap = [{'id':h['id'], 'bytes':h['logical_bytes'], 'alive':h['alive'],
                 'initialized':h['value'] is not None, 'array':byte_array(h['value'])}
                for h in event['heap']]
        subject = event.get('subject') or {}
        name = subject.get('name')
        if event['event'] == 'bind' and name in ('p','q'):
            pointers[name] = pointer(event['value'], {h['id'] for h in heap})
        read = None
        if event['event'].startswith('array-read-'):
            # Suppress reads of the small worlds[] pointer array from the view.
            if not any(h['array'] and h['array']['node'] == subject.get('node') for h in heap):
                continue
            read = {'node':subject['node'], 'index':event['value']['decimal'],
                    'explicit':event['event']=='array-read-explicit'}
        elif event['event'] in ('store','bind'):
            if name not in ('p','q','untouched') and subject.get('decl_uid') not in {h['id'] for h in heap}:
                continue
        elif event['event'] == 'before-delete':
            continue
        output.append({'event':event['event']+(f' {name}' if name in ('p','q','untouched') else ''),
                       'line':event['line'], 'evaluation':event['evaluation'], 'heap':heap,
                       'pointers':dict(pointers), 'read':read})
        if name == 'q' and event['event']=='bind': initial = len(output)-1
    assert initial is not None
    return {'initial':initial, 'events':output}


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('trace', type=Path)
    parser.add_argument('output', type=Path)
    parser.add_argument('--fragment', type=Path)
    args = parser.parse_args()
    opener = gzip.open if args.trace.suffix == '.gz' else open
    with opener(args.trace, 'rt') as source:
        lines = source.read().splitlines()
    assert all(not line.strip() or line.startswith('ASTRAL ') for line in lines), 'Unexpected compiler diagnostics; inspect the raw log'
    events = [json.loads(line[7:]) for line in lines if line.startswith('ASTRAL ')]
    data = compact(events)
    literal = json.dumps(data, separators=(',',':')).replace('<','\\u003c')
    fragment = Path(__file__).with_name('viewer.fragment.html').read_text().replace('__TRACE_DATA__',literal)
    assert len(fragment.encode()) < 1_000_000
    if args.fragment:
        args.fragment.write_text(fragment)
    # Standalone defaults; the inline fragment inherits the conversation theme.
    css = '''html{color-scheme:light dark;--foreground:CanvasText;--background:Canvas;--viz-series-1:Highlight;--font-size-base:16px}body{background:Canvas;color:CanvasText;font:16px system-ui;margin:24px auto;padding:0 16px;max-width:850px}.viz-row{display:flex;align-items:center;gap:10px;flex-wrap:wrap}.btn{font:inherit;padding:8px 14px;min-height:44px}.form-label{display:block;margin:12px 0}.form-range{display:block;width:100%}.table{width:100%;border-collapse:collapse}.table td,.table th{text-align:left;padding:10px 6px;border-bottom:1px solid GrayText}.text-small{font-size:.8em}.tabular-nums{font-variant-numeric:tabular-nums}details{margin:16px 0}pre{font-size:.85em}@media(max-width:500px){body{margin:12px auto}.table td:first-child{max-width:130px}}'''
    args.output.write_text('<!doctype html><html lang="en"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>GCC Astral heap trace</title><style>'+css+'</style><body>'+fragment+'</body></html>')
    print(json.dumps({'raw_events':len(events),'shown_events':len(data['events']), 'initial':data['initial'], 'report_bytes':args.output.stat().st_size}))


if __name__ == '__main__':
    main()
