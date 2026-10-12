include(FetchContent)

include(imgui.cmake)
include(implot.cmake)

fetchcontent_declare(
    nfd
    GIT_REPOSITORY https://github.com/btzy/nativefiledialog-extended
    GIT_TAG fc168e8605bfa51aaec22ab0c4e46b9de665a437 # v1.3.0
)
fetchcontent_makeavailable(nfd)
set(NFD_LICENSE ${nfd_SOURCE_DIR}/LICENSE PARENT_SCOPE)

fetchcontent_declare(
    magic_enum
    GIT_REPOSITORY https://github.com/Neargye/magic_enum
    GIT_TAG e046b69a3736d314fad813e159b1c192eaef92cd # v0.9.7
)
fetchcontent_makeavailable(magic_enum)

set(IMGUI_LICENSE ${imgui_SOURCE_DIR}/LICENSE.txt PARENT_SCOPE)
set(IMPLOT_LICENSE ${implot_SOURCE_DIR}/LICENSE PARENT_SCOPE)
