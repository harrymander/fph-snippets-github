#!/usr/bin/env python3

import os
import shlex
import shutil
from pathlib import Path

import click

THIS_DIR = Path(__file__).parent
MAT91LIB_SCRIPTS_DIR = THIS_DIR.joinpath("mat91lib", "sam4s", "scripts")
DEFAULT_BINARY = THIS_DIR.joinpath("build", "recap-instrumentation.elf")


def find_gdb() -> str:
    for cmd in ("gdb-multiarch", "arm-none-eabi-gdb"):
        if shutil.which(cmd):
            return cmd

    raise click.ClickException(
        "Cannot find an appropriate GDB command! "
        "Install one and add to PATH, set GDB envvar, or pass --gdb option."
    )


class Context:
    def __init__(
        self,
        gdb: str | None,
        verbose: bool,
        openocd_log: str | None,
    ):
        if gdb:
            if os.path.isabs(gdb):
                if not os.path.exists(gdb):
                    raise click.ClickException(f"File does not exist: {gdb}")
            elif not shutil.which(gdb):
                raise click.ClickException(
                    f"Command not found: {gdb}. Is it in PATH?"
                )
        else:
            gdb = find_gdb()

        self.gdb = gdb
        self.verbose = verbose
        self.openocd_log = openocd_log

    def exec_gdb_command(
        self, *, batch: bool, script: str, target: None | str = None
    ) -> None:
        args = [self.gdb]
        if batch:
            args.append("-batch")

        openocd_args = ["-c", "gdb_port pipe"]
        if self.openocd_log:
            openocd_args.extend(("-c", f"log_output {self.openocd_log}"))
        openocd_args.extend((
            "-f", str(MAT91LIB_SCRIPTS_DIR / "sam4s_stlink.cfg"),
            "-f", str(THIS_DIR.joinpath("scripts", "rtos-openocd.cfg")),
        ))

        args.extend((
            "-ex",
            f"target extended-remote | openocd {shlex.join(openocd_args)}",
            "-x", str(MAT91LIB_SCRIPTS_DIR / f"{script}.gdb"),
        ))
        if target:
            args.append(target)

        if self.verbose:
            click.echo(f"Executing: {args}")

        os.execvp(self.gdb, args)


@click.group()
@click.option("--gdb", envvar="GDB")
@click.option("--verbose", "-v", is_flag=True, default=False)
@click.option(
    "--log", "openocd_log",
    type=click.Path(dir_okay=False, writable=True),
    help="OpenOCD log file",
)
@click.pass_context
def cli(ctx, **kwargs):
    ctx.obj = Context(**kwargs)


def add_subcommand(
    script: str,
    help: str,
    *,
    batch: bool = True,
    requires_target: bool = True,
) -> None:
    if requires_target:
        @click.argument(
            "binary",
            nargs=1,
            required=False,
            type=click.Path(exists=True, dir_okay=False, readable=True),
        )
        def cmd(ctx: Context, binary: str | None):
            ctx.exec_gdb_command(
                batch=batch,
                script=script,
                target=binary or str(DEFAULT_BINARY),
            )
    else:
        def cmd(ctx: Context):
            ctx.exec_gdb_command(batch=batch, script=script)

    if requires_target:
        help = f"""
        {help}

        If BINARY is not provided, defaults to
        {DEFAULT_BINARY.relative_to(Path.cwd())}.
        """

    cli.command(name=script, help=help)(click.pass_obj(cmd))


if __name__ == "__main__":
    add_subcommand("program", "Program the target.")
    add_subcommand("debug", "Program target and open debugger.", batch=False)
    add_subcommand("reset", "Reset target.", requires_target=False)
    add_subcommand(
        "bootflash",
        "Configure target to boot from flash.",
        requires_target=False,
    )
    cli()
