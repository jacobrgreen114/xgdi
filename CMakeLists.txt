# Copyright (C) 2022-2023 Jacob R. Green
# All Rights Reserved.

message("Modules ${CMAKE_MODULE_PATH}")

find_package(Ember MODULE REQUIRED)


add_library(xgdi
        src/xgdi.cpp
        src/datatypes.cpp
        src/drawing_context.cpp
        src/font.cpp
        src/formatted_text.cpp
        src/bitmap.cpp
)

target_shaders(xgdi
    SPIRV
        src/shader/rect.vert
        src/shader/rect.frag

        src/shader/roundrect.vert
        src/shader/roundrect.frag

        src/shader/bitmap.vert
        src/shader/bitmap.frag

        src/shader/glyph.vert
        src/shader/glyph.frag

        src/shader/glyph_sdf.vert
        src/shader/glyph_sdf.frag
)

target_include_directories(xgdi
    PUBLIC
        inc
        ../../external/DevIL/DevIL/include
)


target_link_libraries(xgdi
    PUBLIC
        rndr
        freetype-pp
        IL
        ILU
)


target_precompile_headers(xgdi
    PUBLIC
        <muchcool/rndr.hpp>
        <freetype/freetype.hpp>
)

install(TARGETS xgdi)
