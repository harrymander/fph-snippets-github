import argparse
import re
import sys
from contextlib import nullcontext
from itertools import batched


STRUCT_NAME = "LicenseInfo"


def valid_namespace(name):
    ns = r'[a-zA-Z_][\w\d]*'
    match = re.fullmatch(rf'{ns}(?:::{ns})*', name, flags=re.ASCII)
    return match is not None


def valid_varname(name):
    match = re.fullmatch(r'[a-zA-Z_][\w\d]*', name, flags=re.ASCII)
    return match is not None


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", default="-")
    parser.add_argument("--namespace", default="recap::app")
    parser.add_argument("--varname", default="LicenseInfos")
    parser.add_argument("licenses", nargs="*")
    args = parser.parse_args()

    if not valid_namespace(args.namespace):
        raise ValueError("invalid namespace")
    if not valid_varname(args.varname):
        raise ValueError("invalid variable name")
    if args.varname == STRUCT_NAME:
        raise ValueError(f"variable name cannot be {STRUCT_NAME}")
    if len(args.licenses) % 2:
        raise ValueError("license infos must be in pairs of NAME FILE")

    licenses = [
        (name, open(file).read()) for name, file in batched(args.licenses, 2)
    ]
    licenses.sort()

    if args.output == "-":
        file = nullcontext(sys.stdout)
    else:
        file = open(args.output, "w")
    with file as file:
        generate_license_header(file, args.namespace, args.varname, licenses)


def cpp_raw_string(s):
    return f'R"~~~({s})~~~"'


def generate_license_header(file, namespace, varname, licenses):
    file.write(f"""
    #pragma once
    #include <array>

    namespace {namespace} {{

    struct {STRUCT_NAME} {{
        const char *package_name;
        const char *license_text;
    }};

    constexpr inline std::array<{STRUCT_NAME}, {len(licenses)}> {varname} =
    {{{{
    """)

    for name, text in licenses:
        file.write(f"{{ {cpp_raw_string(name)}, {cpp_raw_string(text)} }},\n")

    file.write("}};\n};\n")


if __name__ == '__main__':
    main()
