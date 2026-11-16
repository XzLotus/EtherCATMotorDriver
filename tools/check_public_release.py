"""Validate an explicit public-file allowlist and its content digests."""
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
    'docs/architecture.md', 'docs/code-tour.md', 'docs/sanitization.md',
    'docs/verification.md', 'examples/host_demo.c', 'include/modbus.h',
    'include/motor_backend.h', 'include/rs485_port.h',
    'ports/host/mock_rs485.c', 'ports/host/mock_rs485.h',
    'ports/n32/board_sdk.h', 'ports/n32/can.c', 'ports/n32/can.h',
    'ports/n32/eeprom.c', 'ports/n32/eeprom.h', 'ports/n32/rs485.c',
    'ports/n32/rs485.h', 'ports/n32/qspi.c', 'ports/n32/qspi.h',
    'src/protocol/modbus.c', 'src/motor/motor_backend_redacted.c',
    'tests/test_protocol.c', 'tests/test_redacted_motor.c',
    'tools/build_and_test.py', 'tools/check_public_release.py',
}
SENSITIVE_PATTERNS = (
    rb'gh[pousr]_[A-Za-z0-9_]{30,}',
    rb'github_pat_[A-Za-z0-9_]{30,}',
    rb'-----BEGIN (?:RSA |EC |OPENSSH )?PRIVATE KEY-----',
    rb'AKIA[A-Z0-9]{16}',
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
    permitted_missing = {'public-files.json'} if args.refresh_manifest else set()
    if actual - expected or expected - actual - permitted_missing:
        raise SystemExit(f'File list mismatch: extra={sorted(actual - expected)}, '
                         f'missing={sorted(expected - actual - permitted_missing)}')
    digests = {}
    for name in sorted(ALLOWED):
        data = (ROOT / name).read_bytes()
        if b'\0' in data:
            raise SystemExit(f'Binary content requires review: {name}')
        data.decode('utf-8')
        for pattern in SENSITIVE_PATTERNS:
            if re.search(pattern, data):
                raise SystemExit(f'Potential sensitive content requires review: {name}')
        digests[name] = hashlib.sha256(data).hexdigest()
    if (ROOT / '.git').exists():
        result = subprocess.run(['git', 'ls-files', '-z'], cwd=ROOT,
                                capture_output=True, check=True)
        tracked = set(result.stdout.decode('utf-8').split('\0')) - {''}
        if tracked - expected:
            raise SystemExit(f'Unexpected tracked paths: {sorted(tracked - expected)}')
    if args.refresh_manifest:
        MANIFEST.write_text(json.dumps({'version': 1, 'files': digests}, indent=2)
                            + '\n', encoding='utf-8', newline='\n')
    saved = json.loads(MANIFEST.read_text(encoding='utf-8'))
    if saved != {'version': 1, 'files': digests}:
        saved_files = saved.get('files', {}) if isinstance(saved, dict) else {}
        changed = sorted(name for name in set(saved_files) | set(digests)
                         if saved_files.get(name) != digests.get(name))
        details = '\n'.join(f'  {name}' for name in changed)
        raise SystemExit(
            'Public file digests changed:\n'
            + (details or '  manifest version or format')
            + '\nReview the changes, then refresh the manifest:\n'
            '  python tools/check_public_release.py --refresh-manifest\n'
            'Commit public-files.json together with the reviewed changes.')
    print(f'PASS: {len(expected)} public text files; explicit allowlist and SHA-256 verified')


if __name__ == '__main__':
    main()
