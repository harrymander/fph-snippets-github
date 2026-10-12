#!/usr/bin/env python3

import argparse
import json
import os
import os.path
import shlex
import sys
from shutil import which
from typing import Any


def elog(*s: str) -> None:
    print(f'{sys.argv[0]}:', *s, file=sys.stderr)


class CommandWriter:
    def __init__(self, cc: str, path_rename: tuple[str, str] | None):
        self.cc = cc
        self.cwd = os.getcwd()
        self.path_rename = path_rename

    def _convert_path(self, path: str) -> str:
        if not self.path_rename:
            return path

        old_name, new_name = self.path_rename
        if os.path.abspath(path) and path.startswith(old_name):
            return f'{new_name}{path[len(old_name):]}'

        return path

    def output_object(
        self, src: str, cflags: list[str], objdir: str,
    ) -> dict[str, Any]:
        objfile = os.path.join(
            objdir,
            f'{os.path.basename(src).removesuffix(".c")}.o'
        )
        return {
            "directory": self._convert_path(self.cwd),
            "file": self._convert_path(os.path.realpath(src)),
            "arguments": [
                self.cc, '-o', self._convert_path(objfile), src, *cflags
            ],
            "output": self._convert_path(objfile),
        }


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument('src', nargs='+')
    parser.add_argument('--objdir', required=True)
    parser.add_argument('--cc', default='cc')
    parser.add_argument('--cflags', required=True)
    return parser.parse_args()


def main() -> None:
    args = parse_args()

    path_rename_var = os.getenv('COMPILE_COMMANDS_PATH_RENAME')
    if path_rename_var:
        names = path_rename_var.split(':', maxsplit=1)
        for name in names:
            if not os.path.isabs(name):
                elog(f"error: '{name}' is not an absolute path")
                sys.exit(1)

        path_rename = (names[0], names[1])
    else:
        path_rename = None

    cflags = shlex.split(args.cflags)
    objdir = os.path.realpath(args.objdir)
    cc = which(args.cc)
    if not cc:
        elog(f"error: '{args.cc}' not found")
        sys.exit(1)

    writer = CommandWriter(cc, path_rename)
    output = json.dumps(
        [writer.output_object(s, cflags, objdir) for s in args.src],
        indent=2
    )
    print(output)


if __name__ == '__main__':
    main()
