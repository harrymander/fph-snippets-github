set(
    source_sans_url_base
    "https://github.com/adobe-fonts/source-sans/releases/download"
)
fetchcontent_declare(
    source_sans
    URL "${source_sans_url_base}/3.052R/TTF-source-sans-3.052R.zip"
    URL_HASH MD5=11faea909c263b00ffe17e033250b083
)
fetchcontent_makeavailable(source_sans)
set(FONT_PATH "${source_sans_SOURCE_DIR}/TTF/SourceSans3-Regular.ttf")
set(SOURCE_SANS_C_PATH ${GEN_DIR}/font.c)
generate_c_array_from_file(
    INPUT ${FONT_PATH}
    OUTPUT ${SOURCE_SANS_C_PATH}
    ARRAY_NAME recap_font_data
)

set(
    LICENSE_CHECKSUM
    56af9b9c6715597e458284a474dc118a50a4150e9d547c70f7b4a33c3e6a9328
)
file(
    DOWNLOAD
    "https://raw.githubusercontent.com/adobe-fonts/source-sans/\
refs/heads/release/LICENSE.md"
    ${CMAKE_CURRENT_BINARY_DIR}/source-sans-license.md
    EXPECTED_HASH SHA256=${LICENSE_CHECKSUM}
)
set(
    SOURCE_SANS_LICENSE
    ${CMAKE_CURRENT_BINARY_DIR}/source-sans-license.md
)
