#!/usr/bin/env python3

import struct
from typing import NamedTuple

import click
from serial import Serial


class Sample(NamedTuple):
    _STRUCT_FORMAT = "<Iic"

    expected_size = struct.calcsize(_STRUCT_FORMAT)

    countup: int
    countdown: int
    alphabet: str

    @classmethod
    def from_bytes(cls, data):
        countup, countdown, alphabet = struct.unpack(cls._STRUCT_FORMAT, data)
        return cls(countup, countdown, alphabet.decode('ascii'))

    def to_bytes(self):
        return struct.pack(self._STRUCT_FORMAT,
                           self.countup,
                           self.countdown,
                           self.alphabet.encode('ascii'))


def get_samples(serial, count):
    serial.write(b's')
    samples = []
    for i in range(count):
        header = serial.read(1)
        assert header == b'\x00', header

        data = serial.read(Sample.expected_size)
        samples.append(Sample.from_bytes(data))

    serial.write(b'x')
    return samples


def get_message(serial):
    serial.write(b'm')
    msg_size = int.from_bytes(serial.read(1), 'little')
    assert msg_size, msg_size

    return serial.read(msg_size).decode('ascii')


@click.group()
@click.option('--device', '-d',
              type=click.Path(exists=True, dir_okay=False),
              default='/dev/ttyACM0')
@click.pass_context
def cli(ctx, device):
    ctx.ensure_object(dict)
    ctx.obj['device'] = device


@cli.command()
@click.argument('count', type=click.IntRange(min=1), default=10)
@click.pass_context
def sample(ctx, count):
    with Serial(ctx.obj['device']) as serial:
        samples = get_samples(serial, count)

    for sample in samples:
        print(sample)

    if (samples[-1].countup - samples[0].countup) != count - 1:
        print('WARNING: non-consecutive counts')


@cli.command()
@click.pass_context
def message(ctx):
    with Serial(ctx.obj['device']) as serial:
        message = get_message(serial)

    print(message)


if __name__ == '__main__':
    cli()
