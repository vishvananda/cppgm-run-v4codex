#!/usr/bin/env python3
"""Verify retained final-audit binaries, inputs, outputs and complete observations."""
from pathlib import Path
import json
import audit_benchmark as audit

prior = audit.prior
HERE = Path(__file__).resolve().parent


def main():
    counts = dict(campaigns=0, compiler_observations=0, runtime_observations=0, executable_checks=0)
    for name in ('audit-initial-common-performance.json', 'audit-common-performance.json',
                 'audit-affected-performance.json', 'audit-followup-performance.json'):
        data = json.loads((HERE/name).read_text()); counts['campaigns'] += 1
        for binary in data['binaries']:
            assert prior.sha(binary['path']) == binary['sha256']
        for source, sha in data.get('sources', {}).items():
            assert prior.sha(source) == sha
        for group, entry in data['inputs'].items():
            source = Path(entry['path']); assert prior.sha(source) == entry['sha256']
            if 'output_hashes' in entry:
                assert [prior.sha(source.with_suffix(suffix)) for suffix in ('.ref', '.my')] == entry['output_hashes']
            for result in entry.get('outputs', []):
                assert prior.sha(result['path']) == result['sha256']
                assert prior.sha(result['executable_path']) == result['executable_sha256']
                prior.run([result['executable_path']]); counts['executable_checks'] += 1
            rows = [row for row in data['observations'] if row['group'] == group]
            assert [row['binary'] for row in rows] == prior.ORDER
            assert all(row['wall_s'] > 0 and row['rss_kib'] > 0 for row in rows)
            counts['compiler_observations'] += len(rows)
        for runtime in data['runtime']:
            assert prior.sha(runtime['source_path']) == runtime['source_sha256']
            for result in runtime['executables']:
                assert prior.sha(result['path']) == result['sha256']
                if 'lowir_path' in result:
                    assert prior.sha(result['lowir_path']) == result['lowir_sha256']
                prior.run([result['path']]); counts['executable_checks'] += 1
            assert [row['binary'] for row in runtime['observations']] == prior.ORDER
            counts['runtime_observations'] += len(runtime['observations'])
    final = json.loads((HERE/'audit-common-performance.json').read_text())
    assert prior.sha(audit.ROOT/'dev/cppgm++') == final['binaries'][1]['sha256']
    print('PASS: '+json.dumps(counts, sort_keys=True))


if __name__ == '__main__':
    main()
