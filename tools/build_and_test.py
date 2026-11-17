"""Build and execute the portable public samples with GCC."""
from pathlib import Path
import os
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parent.parent


def main():
    compiler = shutil.which(os.environ.get('CC', 'gcc'))
    if compiler is None:
        raise SystemExit('Install GCC and add it to PATH, or set CC to a compiler executable.')
    build = ROOT / 'build'
    build.mkdir(exist_ok=True)
    env = os.environ.copy()
    env['PATH'] = str(Path(compiler).parent) + os.pathsep + env.get('PATH', '')
    flags = ['-std=c99', '-Wall', '-Wextra', '-Wpedantic', '-Werror', '-O2', '-Iinclude']
    protocol = ['src/comms/crc16.c', 'src/comms/modbus_demo.c']
    modules = ['src/telemetry/demo_telemetry.c', 'src/drive/redacted_drive.c']
    targets = {
        'test_modbus': protocol + ['tests/test_modbus.c'],
        'test_public_modules': modules + ['tests/test_public_modules.c'],
        'host_demo': protocol + modules + ['examples/host_demo.c'],
    }
    options = {'cwd': ROOT, 'env': env, 'capture_output': True,
               'text': True, 'encoding': 'utf-8', 'errors': 'replace'}
    if sys.platform == 'win32':
        options['creationflags'] = subprocess.CREATE_NO_WINDOW
    for name, sources in targets.items():
        output = build / (name + ('.exe' if os.name == 'nt' else ''))
        for command in ([compiler, *flags, *sources, '-o', str(output.relative_to(ROOT))],
                        [str(output)]):
            result = subprocess.run(command, **options)
            sys.stdout.write(result.stdout)
            sys.stderr.write(result.stderr)
            result.check_returncode()
    print('PASS: all host targets compiled with warnings treated as errors', flush=True)


if __name__ == '__main__':
    main()
