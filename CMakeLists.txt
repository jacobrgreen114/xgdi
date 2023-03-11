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

        src/shader/glyph.vert
        src/shader/glyph.frag

        src/shader/bitmap.vert
        src/shader/bitmap.frag
)

target_include_directories(xgdi
    PUBLIC
        inc
        ../../external/DevIL/DevIL/include
)


target_link_libraries(xgdi
    PUBLIC
        rndr
        freetype
        IL
        ILU
)


target_precompile_headers(xgdi
    PUBLIC
        <muchcool/rndr.hpp>
        <ft2build.h>
)


function(compile_shader TARGET_NAME SOURCE_FILE)
    set(OUTPUT_FILE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Shaders/${TARGET_NAME}.spv)

    add_custom_command(
        OUTPUT ${OUTPUT_FILE}
        COMMAND glslc ${SOURCE_FILE} -o ${OUTPUT_FILE}
        DEPENDS ${SOURCE_FILE}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )

    add_custom_target(
        ${TARGET_NAME}
        DEPENDS ${OUTPUT_FILE}
    )
endfunction()


# compile_shader(VertexShader src/Shaders/shader.vert)
# compile_shader(FragmentShader src/Shaders/shader.frag)
#
# compile_shader(RectVertShader src/Shaders/rect.vert)
# compile_shader(RectFragShader src/Shaders/rect.frag)
#
# compile_shader(RoundRectVertShader src/Shaders/roundrect.vert)
# compile_shader(RoundRectFragShader src/Shaders/roundrect.frag)
#
# compile_shader(GlyphVertShader src/Shaders/glyph.vert)
# compile_shader(GlyphFragShader src/Shaders/glyph.frag)
#
# compile_shader(BitmapVertShader src/Shaders/bitmap.vert)
# compile_shader(BitmapFragShader src/Shaders/bitmap.frag)
#
#
# add_dependencies(xgdi
#     VertexShader
#     FragmentShader
#     RectVertShader
#     RectFragShader
#     RoundRectVertShader
#     RoundRectFragShader
#     GlyphVertShader
#     GlyphFragShader
#     BitmapVertShader
#     BitmapFragShader
# )

