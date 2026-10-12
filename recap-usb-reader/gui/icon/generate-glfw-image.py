#!/usr/bin/env python3

import argparse
import re
from pathlib import Path


def parse_args():
    argparser = argparse.ArgumentParser()
    argparser.add_argument('input')
    argparser.add_argument('output')
    argparser.add_argument('--varname')
    argparser.add_argument('--glfw-include-path', default='<GLFW/glfw3.h>')
    return argparser.parse_args()


def pixel_data(image):
    from itertools import product

    pixels = image.load()
    for y, x in product(range(image.height), range(image.width)):
        yield from pixels[x, y]


def main(
    image_path: Path,
    output_path: Path,
    varname: str | None,
    include_path: str,
):
    from PIL import Image

    image = Image.open(image_path).convert('RGBA')
    varname = re.sub(r'[^\w]', '_', varname or image_path.stem, flags=re.ASCII)
    if varname[0].isdigit():
        varname = f'_{varname}'

    output_path.write_text(f"""#include {include_path}
static unsigned char {varname}__pixels[] = {{
    {", ".join(map(str, pixel_data(image)))}
}};
GLFWimage {varname} = {{
    .width = {image.width},
    .height = {image.height},
    .pixels = {varname}__pixels,
}};
""")


if __name__ == '__main__':
    args = parse_args()
    main(
        Path(args.input),
        Path(args.output),
        args.varname,
        args.glfw_include_path)
