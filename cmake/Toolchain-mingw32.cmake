# the name of the target operating system
SET(CMAKE_SYSTEM_NAME Windows)

# only use static libs for linking
set(CMAKE_FIND_LIBRARY_SUFFIXES  .a )
#set(CMAKE_SHARED_LIBRARY_PREFIX "")
set(CMAKE_SHARED_MODULE_SUFFIX "")

# which compilers to use for C and C++
SET(CMAKE_C_COMPILER /usr/bin/i586-mingw32msvc-gcc)
SET(CMAKE_CXX_COMPILER /usr/bin/i586-mingw32msvc-g++)
SET(CMAKE_RC_COMPILER /usr/bin/i586-mingw32msvc-windres)
SET( MINGWLIBS /opt/mingw32/ CACHE INTERNAL MINGWLIB_DUMMMY_DIR FORCE  )
SET( BUILD_SHARED_LIBS OFF CACHE INTERNAL MINGWLIB_DUMMMY_DIR FORCE  )
SET(CMAKE_FIND_ROOT_PATH /opt/mingw32 )
SET( wxWidgets_CONFIG_EXECUTABLE /opt/mingw32/bin/wx-config )
SET( wxWidgets_LIBRARIES
	wx_baseu_net-2.8-i586-pc-mingw32.dll
	wx_mswu_core-2.8-i586-pc-mingw32.dll
	wx_baseu-2.8-i586-pc-mingw32.dll
	wx_mswu_adv-2.8-i586-pc-mingw32.dll
	wx_mswu_aui-2.8-i586-pc-mingw32.dll
	wx_mswu_html-2.8-i586-pc-mingw32.dll
	wx_mswu_gl-2.8-i586-pc-mingw32.dll
	wx_baseu_xml-2.8-i586-pc-mingw32.dll
	wx_mswu_qa-2.8-i586-pc-mingw32.dll
	wx_mswu_richtext-2.8-i586-pc-mingw32.dll
	wx_mswu_xrc-2.8-i586-pc-mingw32.dll)
SET( wxWidgets_INCLUDE_DIRS /opt/mingw32/include/wx-2.8/ /opt/mingw32/lib/wx/include/i586-pc-mingw32-msw-unicode-release-2.8/)
SET( wxWidgets_LIB_DIR /opt/mingw32/lib )
SET( wxWidgets_CONFIGURATION mswu )
SET( CMAKE_VERBOSE_MAKEFILE OFF )
SET( wxWidgets_RC_DIR /opt/mingw32/include/wx-2.8 )
#otherwise cmake finds linux lib for win...
SET( ENV{OPENALDIR} /opt/mingw32/ )
SET( OPENAL_LIBRARY OpenAL32 )
SET( OPENAL_INCLUDE_DIR /opt/mingw32/include/AL )
SET( Boost_LIBRARIES
	boost_thread-gcc44-mt-1_41
	boost_filesystem-gcc44-mt-1_41
	boost_system-gcc44-mt-1_41 )
SET( boost_LIB_DIR /opt/mingw32/lib )
SET( boost_INCLUDE_DIR /opt/mingw32/include )
SET( WINDRES /usr/bin/i586-mingw32msvc-windres )
link_directories( /opt/mingw32/lib )
INCLUDE_DIRECTORIES(/opt/mingw32/include  )
INCLUDE_DIRECTORIES(/opt/mingw32/include/drmingw/include  )

SET( PKG_CONFIG_EXECUTABLE /opt/mingw32/bin/pkg-config )
ADD_DEFINITIONS( -DCURL_STATICLIB -DWXUSINGDLL -D__WXMSW__ -mthreads)

SET( LOCALE_INSTALL_DIR "${CMAKE_BINARY_DIR}/locale" CACHE STRING
	"message catalogs will installed here" FORCE )
SET( CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}" CACHE STRING
	"install prefix" FORCE )
SET( CURL_CFLAGS "-I/opt/mingw32/include" )
SET( CURL_STATIC_LIBRARY_DIRS "/opt/mingw32/lib")
SET( CURL_STATIC_LDFLAGS "-L/opt/mingw32/lib;-lcurl;-lws2_32")
SET( CURL_INCLUDE_DIR "/opt/mingw32/include")
