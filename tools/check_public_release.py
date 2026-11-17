"""Check every public file; hash executable source/config while allowing doc edits."""
from pathlib import Path
import argparse
import hashlib
import json
import re
import subprocess

ROOT = Path(__file__).resolve().parent.parent
MANIFEST = ROOT / 'public-files.json'
ALLOWED = {
    '.gitattributes', '.gitignore', '.github/workflows/verify.yml', 'README.md',
    'docs/architecture.md', 'docs/source-provenance.md', 'docs/sanitized-scope.md',
    'docs/verification.md', 'include/demo_modbus.h', 'include/demo_telemetry.h',
    'include/redacted_drive.h', 'src/comms/crc16.c', 'src/comms/modbus_demo.c',
    'src/telemetry/demo_telemetry.c', 'src/drive/redacted_drive.c',
    'reference/haawking_sci_rx.c', 'examples/host_demo.c', 'tests/test_modbus.c',
    'tests/test_public_modules.c', 'tools/build_and_test.py', 'tools/check_public_release.py',
}
SENSITIVE_PATTERNS = (
    rb'gh[pousr]_[A-Za-z0-9_]{30,}', rb'github_pat_[A-Za-z0-9_]{30,}',
    rb'-----BEGIN (?:RSA |EC |OPENSSH )?PRIVATE KEY-----', rb'AKIA[A-Z0-9]{16}',
    rb'[A-Za-z]:[\\/](?:Users|Projects)[\\/]',
)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--refresh-manifest', action='store_true')
    args = parser.parse_args()
    expected = ALLOWED | {'public-files.json'}
    actual = set()
    for path in ROOT.rglob('*'):
        parts = path.relative_to(ROOT).parts
        if any(p in {'.git', 'build', '__pycache__'} for p in parts):
            continue
        if path.is_symlink():
            raise SystemExit(f'Symlink requires review: {path.relative_to(ROOT)}')
        if path.is_file():
            actual.add(path.relative_to(ROOT).as_posix())
    missing = expected - actual - ({'public-files.json'} if args.refresh_manifest else set())
    if actual - expected or missing:
        raise SystemExit(f'File list mismatch: extra={sorted(actual - expected)}, missing={sorted(missing)}')
    digests = {}
    for name in sorted(actual):
        data = (ROOT / name).read_bytes()
        if b'\0' in data:
            raise SystemExit(f'Binary content requires review: {name}')
        data.decode('utf-8')
        if any(re.search(pattern, data) for pattern in SENSITIVE_PATTERNS):
            raise SystemExit(f'Potential sensitive content requires review: {name}')
        if name in ALLOWED and Path(name).suffix != '.md':
            digests[name] = hashlib.sha256(data).hexdigest()
    if (ROOT / '.git').exists():
        result = subprocess.run(['git', 'ls-files', '-z'], cwd=ROOT,
                                capture_output=True, check=True)
        tracked = set(result.stdout.decode('utf-8').split('\0')) - {''}
        if tracked - expected:
            raise SystemExit(f'Unexpected tracked files: {sorted(tracked - expected)}')
    current = {'version': 1, 'source_and_config_sha256': digests}
    if args.refresh_manifest:
        MANIFEST.write_text(json.dumps(current, indent=2) + '\n', encoding='utf-8', newline='\n')
    saved = json.loads(MANIFEST.read_text(encoding='utf-8'))
    if saved != current:
        old = saved.get('source_and_config_sha256', {})
        changed = sorted(name for name in set(old) | set(digests) if old.get(name) != digests.get(name))
        raise SystemExit('Source/config digests changed: ' + ', '.join(changed)
                         + '\nReview changes, then run: python tools/check_public_release.py --refresh-manifest'
                         + '\nCommit public-files.json together with the reviewed changes.')
    print(f'PASS: {len(expected)} public text files; {len(digests)} source/config digests verified')


if __name__ == '__main__':
    main()
