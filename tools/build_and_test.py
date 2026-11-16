"""Build and execute the portable portfolio demo and behavioral tests."""
from pathlib import Path
import os
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parent.parent


def main():
    compiler = shutil.which(os.environ.get('CC', 'gcc'))
    if compiler is None:
        raise SystemExit('Install GCC and add it to PATH, or set CC to a C compiler executable.')
    build = ROOT / 'build'
    build.mkdir(exist_ok=True)
    env = os.environ.copy()
    env['PATH'] = str(Path(compiler).parent) + os.pathsep + env.get('PATH', '')
    flags = ['-std=c99', '-Wall', '-Wextra', '-Wpedantic', '-Werror', '-O2',
             '-Iinclude', '-Iports/host']
    shared = ['src/protocol/modbus.c', 'ports/host/mock_rs485.c']
    motor = ['src/motor/motor_backend_redacted.c']
    targets = {
        'test_protocol': shared + ['tests/test_protocol.c'],
        'test_redacted_motor': motor + ['tests/test_redacted_motor.c'],
        'host_demo': shared + motor + ['examples/host_demo.c'],
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
    print('PASS: all portable targets compiled with warnings treated as errors', flush=True)


if __name__ == '__main__':
    main()
