set(FONTAWESOME_VERSION 6.4.2)
set(
    FONTAWESOME_URL_BASE
    "https://use.fontawesome.com/releases/v${FONTAWESOME_VERSION}"
)
set(
    FONTAWESOME_URL
    "fontawesome-free-${FONTAWESOME_VERSION}-desktop.zip"
)
fetchcontent_declare(
    fontawesome
    URL "${FONTAWESOME_URL_BASE}/${FONTAWESOME_URL}"
    URL_HASH MD5=ba00ea72ef921fc739c8229fb6ec2ded
)
fetchcontent_makeavailable(fontawesome)
set(
    FA_FONT_PATH
    "${fontawesome_SOURCE_DIR}/otfs/Font Awesome 6 Free-Solid-900.otf"
)
set(FA_FONT_C_PATH ${GEN_DIR}/fontawesome.c)
generate_c_array_from_file(
    INPUT ${FA_FONT_PATH}
    OUTPUT ${FA_FONT_C_PATH}
    ARRAY_NAME fontawesome_ttf_data
)
