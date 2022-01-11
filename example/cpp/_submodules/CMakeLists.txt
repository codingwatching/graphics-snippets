
include_directories(
	glew_opengl_4_6/include
)

# glew
# see [Automatic Code Generation](http://glew.sourceforge.net/advanced.html)
add_library(
	glew STATIC
	glew_opengl_4_6/include/gl/glew.h
	glew_opengl_4_6/include/gl/wglew.h
	glew_opengl_4_6/include/gl/eglew.h
	glew_opengl_4_6/include/gl/glxew.h
	glew_opengl_4_6/src/glew.c
	glew_opengl_4_6/src/glewinfo.c
)
target_compile_definitions(
	glew PUBLIC
	-DWIN32 -D_LIB -DWIN32_LEAN_AND_MEAN -DGLEW_STATIC
)

# freetype
add_subdirectory(freetype)