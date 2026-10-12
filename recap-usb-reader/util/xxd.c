/**
 * A very lightweight portable equivalent of xxd.
 *
 * Reads from input file and prints a C file to stdout with a const uint8_t array and const size_t
 * size variable.
 *
 * Takes 1 to 3 arguments:
 *
 * xxd <input file>
 *   Read from input file, array variable has name "data" and size is "data_size".
 *
 * xxd <input file> <array name>
 *   Reads from input file and stores array in variable named <array name>. Size variable is
 *   <array name> appended with "_size".
 *
 * xxd <input file> <array name> <size name>
 *   Reads from input file and stores array in variable named <array name> and size in variable
 *   named <size name>.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define BUF_SIZE 256

static int xxd(FILE *fin, FILE *fout, const char *arrname, const char *sizename)
{
#define write_error()                                                                              \
    do {                                                                                           \
        fputs("Error writing to file\n", stderr);                                                  \
        return 1;                                                                                  \
    } while (0)
#define read_error()                                                                               \
    do {                                                                                           \
        fputs("Error writing to file\n", stderr);                                                  \
        return 1;                                                                                  \
    } while (0)

    if (fprintf(
            fout,
            "#include <stdint.h>\n"
            "#include <stddef.h>\n"
            "const uint8_t %s[] = {",
            arrname
        )
        < 0)
    {
        write_error();
    }
    while (!feof(fin)) {
        uint8_t buf[BUF_SIZE];
        const size_t r = fread(buf, 1, sizeof(buf), fin);
        if (ferror(fin)) {
            read_error();
        }

        for (size_t i = 0; i < r; i++) {
            if (fprintf(fout, "0x%02x,", buf[i]) < 0) {
                write_error();
            }
        }
    }
    if (fputs("};\n", fout) == EOF) {
        write_error();
    }
    if (sizename) {
        if (fprintf(fout, "const size_t %s = sizeof(%s);\n", sizename, arrname) < 0) {
            write_error();
        }
    } else if (fprintf(fout, "const size_t %s_size = sizeof(%s);\n", arrname, arrname) < 0) {
        write_error();
    }
    return 0;
}

int main(int argc, char *argv[])
{
    FILE *file;
    int ret;

    if (!argv[1]) {
        fputs("Error: missing input file path\n", stderr);
        fprintf(stderr, "Usage: %s <input file> [array-name [size-name]]\n", argv[0]);
        return 1;
    }
    file = fopen(argv[1], "rb");
    if (!file) {
        fputs("Error opening file\n", stderr);
        return 1;
    }
    ret = xxd(file, stdout, argc > 2 ? argv[2] : "data", argc > 3 ? argv[3] : NULL);
    fclose(file);
    return ret;
}
