SECTIONS
{
    OVERLAY :
    {
        /* Move the build ID before .rodata (otherwise it moves the location of .vectors)*/
        .gnu_build_id
        {
            ext_gnu_build_id = .;
            *(.note.gnu.build-id)
        }
    }
}
INSERT BEFORE .rodata;
