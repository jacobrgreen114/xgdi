

add_library(xgdi
  STATIC
    src/XGDI.cpp
    src/DrawingContext.cpp
    src/Font.cpp
    src/FormattedText.cpp
    src/Bitmap.cpp
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


function(CompileShader TARGET_NAME SOURCE_FILE)
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


CompileShader(VertexShader src/Shaders/shader.vert)
CompileShader(FragmentShader src/Shaders/shader.frag)

CompileShader(RectVertShader src/Shaders/rect.vert)
CompileShader(RectFragShader src/Shaders/rect.frag)

CompileShader(RoundRectVertShader src/Shaders/roundrect.vert)
CompileShader(RoundRectFragShader src/Shaders/roundrect.frag)

CompileShader(GlyphVertShader src/Shaders/glyph.vert)
CompileShader(GlyphFragShader src/Shaders/glyph.frag)

CompileShader(BitmapVertShader src/Shaders/bitmap.vert)
CompileShader(BitmapFragShader src/Shaders/bitmap.frag)


add_dependencies(xgdi
    VertexShader
    FragmentShader
    RectVertShader
    RectFragShader
    RoundRectVertShader
    RoundRectFragShader
    GlyphVertShader
    GlyphFragShader
    BitmapVertShader
    BitmapFragShader
)


