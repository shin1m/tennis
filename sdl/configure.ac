AC_INIT([tennis-sdl], [201507])
AM_INIT_AUTOMAKE([foreign nostdinc dist-bzip2 no-dist-gzip])
AC_CONFIG_SRCDIR([main.cc])

PKG_CHECK_MODULES([LIBXML], [libxml-2.0 >= 2.7])
PKG_CHECK_MODULES([SDL2], [sdl2 >= 2.0])
PKG_CHECK_MODULES([SDL2_IMAGE], [SDL2_image >= 2.0])
PKG_CHECK_MODULES([SDL2_TTF], [SDL2_ttf >= 2.0])
PKG_CHECK_MODULES([SDL2_MIXER], [SDL2_mixer >= 2.0])

if test "${CXXFLAGS+set}" != set; then
	CXXFLAGS=
fi
AC_PROG_CXX

AC_C_INLINE
AC_TYPE_PID_T
AC_TYPE_SIZE_T

AC_ARG_ENABLE(
	[debug],
	AS_HELP_STRING([--enable-debug], [turn on debugging]),
	[case "${enableval}" in
	  yes) debug=true ;;
	  no) debug=false ;;
	  *) AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
	 esac],
	[debug=false]
)
AM_CONDITIONAL([DEBUG], [test x$debug = xtrue])
AC_ARG_ENABLE(
	[profile],
	AS_HELP_STRING([--enable-profile], [turn on profiling]),
	[case "${enableval}" in
	  yes) profile=true ;;
	  no) profile=false ;;
	  *) AC_MSG_ERROR([bad value ${enableval} for --enable-profile]) ;;
	 esac],
	[profile=false]
)
AM_CONDITIONAL([PROFILE], [test x$profile = xtrue])

AC_CONFIG_HEADERS([configure.h])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
