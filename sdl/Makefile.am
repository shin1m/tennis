bin_PROGRAMS = test_xml_reader test_collada viewer gears tennis
AM_CPPFLAGS = $(LIBXML_CFLAGS) $(SDL2_CFLAGS) -I .
AM_CXXFLAGS =
AM_LDFLAGS =
if DEBUG
AM_CXXFLAGS += -g
else
AM_CPPFLAGS += -DNDEBUG
AM_CXXFLAGS += -O3
endif
if PROFILE
AM_CXXFLAGS += -pg
AM_LDFLAGS += -pg
endif
LDADD = $(SDL2_LIBS) -lGLESv2 -lGLU $(SDL2_IMAGE_LIBS) $(SDL2_TTF_LIBS) $(SDL2_MIXER_LIBS)
GL_SOURCES = \
	xanadu/matrix \
	xanadu/transform \
	xanadu/tuple \
	xanadu/vector \
	portable.h \
	portable.cc \
	gl/core.h \
	gl/shaders.h \
	gl/image.h \
	gl.cc
COLLADA_SOURCES = $(GL_SOURCES) \
	xml_reader.h \
	collada.h \
	collada.cc
test_xml_reader_LDADD = $(LIBXML_LIBS)
test_xml_reader_SOURCES = xml_reader.h test_xml_reader.cc
test_collada_LDADD = $(LDADD) $(LIBXML_LIBS)
test_collada_SOURCES = $(COLLADA_SOURCES) test_collada.cc
viewer_LDADD = $(LDADD) $(LIBXML_LIBS)
viewer_SOURCES = $(COLLADA_SOURCES) sdl_core.h viewer.cc
gears_SOURCES = $(GL_SOURCES) sdl_core.h gears.cc
tennis_LDADD = $(LDADD) $(LIBXML_LIBS)
tennis_SOURCES = $(COLLADA_SOURCES) \
	placement.h \
	sdl_core.h \
	ball.h \
	ball.cc \
	player.h \
	player.cc \
	stage.h \
	stage.cc \
	computer.h \
	computer.cc \
	main.h \
	main.cc
