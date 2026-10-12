if(UNIX)
    if(NOT (IMGUI_BACKEND_OPENGL_FOUND AND IMGUI_BACKEND_GLFW_FOUND))
        message(FATAL_ERROR "Missing OpenGL and GLFW")
    endif()
    target_sources(recapctl-gui PUBLIC glfw_opengl3.cpp)
    target_link_libraries(recapctl-gui PUBLIC
        imgui::backend::glfw
        imgui::backend::opengl
    )
    target_compile_options(
        recapctl-gui
        PUBLIC -Wall -Wextra $<$<CONFIG:DEBUG>:-Og>
    )
elseif(WIN32 AND MSVC)
    target_sources(recapctl-gui PUBLIC win32_dx12.cpp)
    target_link_libraries(recapctl-gui PUBLIC imgui::backend::win32-native)
    set_target_properties(recapctl-gui PROPERTIES WIN32_EXECUTABLE true)
    target_compile_options(recapctl-gui PUBLIC /nologo /W4)
    target_compile_definitions(recapctl-gui PUBLIC UNICODE _UNICODE)
else()
    message(SEND_ERROR "Unsupported platform")
endif()
