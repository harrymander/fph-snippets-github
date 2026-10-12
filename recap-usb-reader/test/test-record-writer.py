import os.path
import subprocess
import tempfile
from shutil import which


SIGNATURE = b'-RECAP-'
SIGNATURELEN = len(SIGNATURE)
COMPLETE_SAMPLE_SIZE = SENTINEL_SIZE = 19
AUDIO_SAMPLE_SIZE = 4
EXPECTED_SAMPLE_RATE_KHZ = 10


def run_tester(exec_path: str, outpath: str, num_samples: int) -> None:
    args = [exec_path, outpath, str(num_samples)]
    valgrind = which('valgrind')
    if valgrind:
        args = [
            valgrind,
            '--leak-check=full',
            '--error-exitcode=1',
            '--quiet',
            '--',
            *args
        ]
    subprocess.run(args, check=True)


def test_header(header: str) -> None:
    fields = {
        key: value.lstrip() for key, value in (
            line.split(':') for line in header.split('\n')
        )
    }

    def pop_assert(key: str, exp: str) -> None:
        assert key in fields, f'Header missing key: {key}'
        val = fields.pop(key)
        assert val == exp, \
            f'Wrong value for header key {key!r}: {val!r} != {exp!r}'

    pop_assert('Firmware', '5.0.0-test')
    pop_assert('Build', 'deadbeefdeadbeefdeadbeefdeadbeefdeadbeef')
    pop_assert('Sample-Rate', str(EXPECTED_SAMPLE_RATE_KHZ))
    pop_assert('VREF', '5000')
    pop_assert('Board', 'Test device')
    pop_assert('Flow', 'Test flow meter')
    pop_assert('Flow-Offset', '123456')
    pop_assert('Flow-Scale', '-120')
    pop_assert('Flow-Sign', '0')
    pop_assert('H2S-Barcode', 'H2S barcode 123')
    pop_assert('Extra', 'Hello, world!')

    assert not fields, f'Header contains unexpected extra fields: {fields}'


def test_data(data: bytes, exp_millis: int) -> None:
    # Check for signature
    assert data.startswith(SIGNATURE), 'Invalid file signature'

    # Check for major, minor record version bytes
    major = data[SIGNATURELEN]
    minor = data[SIGNATURELEN + 1]
    assert (major, minor) == (5, 0), f'Invalid version, got {major}.{minor}'

    # Check for file size field, 64-bit unsigned integer
    file_size_field = int.from_bytes(
        data[SIGNATURELEN + 2:SIGNATURELEN + 10],
        'little',
        signed=False
    )
    assert file_size_field == len(data), 'File size field does not match size'

    # Check for sample offset field, 64-bit unsigned integer
    sample_offset_field = int.from_bytes(
        data[SIGNATURELEN + 10:SIGNATURELEN + 18],
        'little',
        signed=False
    )

    # Check header string, a null-terminated UTF-8 string
    header_offset = SIGNATURELEN + 18
    header_end = data[header_offset:].find(b'\0')
    assert header_end > 0, 'Cannot find header null-terminator'
    header = data[header_offset:][:header_end]
    test_header(header.decode('utf-8'))
    header_len = len(header) + 1  # +1 for null-terminator

    # Samples begin after the header
    assert header_len + header_offset == sample_offset_field, \
        'Sample offset field is incorrect'

    # Check that the number of actual samples is correct
    total_samples_size = file_size_field - sample_offset_field - SENTINEL_SIZE
    sample_size = (  # size of a group of samples representing a single msec
        COMPLETE_SAMPLE_SIZE
        + AUDIO_SAMPLE_SIZE * (EXPECTED_SAMPLE_RATE_KHZ - 1)
    )
    assert total_samples_size % sample_size == 0, \
        'Remaining size not divisible by sample size'
    num_millis = total_samples_size // sample_size
    assert num_millis == exp_millis, \
        f'Number of millisecond sample groups {num_millis} != {exp_millis}'

    assert data[-SENTINEL_SIZE:][:4] == b'\0' * 4, 'Missing sentinel sample'


def main(exec_path: str) -> None:
    num_millis = 127

    with tempfile.TemporaryDirectory() as tmpdir:
        record_path = os.path.join(tmpdir, "record.dat")
        run_tester(exec_path, record_path, num_millis)
        with open(record_path, 'rb') as f:
            data = f.read()

    test_data(data, num_millis)
    print('All tests passed :)')


if __name__ == '__main__':
    from sys import argv

    main(argv[1])
