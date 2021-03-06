INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

INCLUDE(compat_flags)


# Add Definitions, Compiler-Switches, etc.: -Wall -O2 -g3 ...
# MSVC compiler (cl.exe) does not accept the same switches as gcc, although preprocessor definitions in the -D form will work for both
IF(NOT MSVC)
	IF( NOT MINGW )
		#wxMSW generates > 9000 warnings with -wextra...
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wextra")
	ENDIF( NOT MINGW )
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-strict-aliasing")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wno-strict-aliasing")
ELSE(NOT MSVC)
	ADD_DEFINITIONS(-D_RC_MSVC)
ENDIF(NOT MSVC)

if(WIN32)
	add_definitions(
		-D__WXMSW__
		-D_WIN32_WINNT=0x0501
		-DBOOST_THREAD_USE_LIB
		-DSTRICT
		-DHAVE_W32API_H
	)
	set(Boost_THREADAPI win32)
endif(WIN32)

set(Boost_USE_MULTITHREADED      ON)
if( ${CMAKE_CROSSCOMPILING} )
	set(Boost_USE_STATIC_LIBS       ON)
	set(Boost_USE_STATIC_RUNTIME    ON)
endif()

FIND_PACKAGE(Boost 1.42.0 COMPONENTS thread system REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})

ADD_DEFINITIONS( -DLOCALE_INSTALL_DIR="${LOCALE_INSTALL_DIR}" )


#----------------------------------------------------------------------------------------------------
# wxWidgets lib dependency check
#----------------------------------------------------------------------------------------------------
# Here you can define, what Libraries of wxWidgets you need for your Application. You can figure out what Libraries you need here:
# http://www.wxwidgets.org/manuals/2.8/wx_librarieslist.html
# order matters!
SET(wxWidgets_USE_LIBS
		net
		aui
		xml
		qa
		richtext
		html
		adv
		core
		base
	)
# We need the Find package for wxWidgets to work, but auto-find is broken in x-compile setup
FIND_PACKAGE(wxWidgets REQUIRED ${wxWidgets_USE_LIBS})
# Did we find wxWidgets ? This condition will fail for as long as the internal Vars do not point to the proper wxWidgets Configuration.
IF(wxWidgets_FOUND)
	INCLUDE(${wxWidgets_USE_FILE})
	INCLUDE_DIRECTORIES( ${wxWidgets_INCLUDE_DIRS} )
ELSE(wxWidgets_FOUND)
	# For Convenience. If we cannot continue, inform the User.
	MESSAGE( FATAL_ERROR "wxWidgets library not found! Please install the package to continue")
ENDIF(wxWidgets_FOUND)

if (CMAKE_CROSSCOMPILING)
	message(STATUS "Crosscompiling: using static flags for wxwidgets")
	set(WXSTATIC "--static")
	#FIXME: autodetect thise flags:
	add_definitions(-DAL_LIBTYPE_STATIC)
else()
	set(WXSTATIC "")
endif()

execute_process(COMMAND ${wxWidgets_CONFIG_EXECUTABLE} "--cppflags" "${WXSTATIC}"
	OUTPUT_VARIABLE WX_CXX_FLAGS
	OUTPUT_STRIP_TRAILING_WHITESPACE )
message(STATUS "Using WX_CXX_FLAGS: ${WX_CXX_FLAGS}")

execute_process(COMMAND ${wxWidgets_CONFIG_EXECUTABLE} "--cflags" "${WXSTATIC}"
	OUTPUT_VARIABLE WX_C_FLAGS
	OUTPUT_STRIP_TRAILING_WHITESPACE )
message(STATUS "Using WX_C_FLAGS: ${WX_C_FLAGS}")

execute_process(COMMAND ${wxWidgets_CONFIG_EXECUTABLE} "--libs" "${WXSTATIC}"
	OUTPUT_VARIABLE WX_LD_FLAGS
	OUTPUT_STRIP_TRAILING_WHITESPACE )
message(STATUS "Using WX_LD_FLAGS: ${WX_LD_FLAGS}")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WX_CXX_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${WX_C_FLAGS}")


#----------------------------------------------------------------------------------------------------
# openAL libs dependency check
#----------------------------------------------------------------------------------------------------

IF( ENABLE_DEBUG_REPORT )
	ADD_DEFINITIONS( -DENABLE_DEBUG_REPORT )
	IF( MINGW )
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -gstabs")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -gstabs")
		LINK_LIBRARIES( intl)
	ENDIF( MINGW )
ENDIF( ENABLE_DEBUG_REPORT )

FIND_PACKAGE( CURL REQUIRED libcurl )
IF( NOT CURL_FOUND )
	MESSAGE( FATAL_ERROR "required library libCURL not found" )
ENDIF( NOT CURL_FOUND )

if(DEFINED PKG_CONFIG_EXECUTABLE) # override CURL_LIBRARIES if curl-config is set
	execute_process(COMMAND ${PKG_CONFIG_EXECUTABLE} "libcurl" "--libs"
		OUTPUT_VARIABLE CURL_LINKFLAGS
		OUTPUT_STRIP_TRAILING_WHITESPACE )

	SET(CURL_LINKFLAGS "${CURL_LINKFLAGS}" CACHE STRING "" FORCE)
	message(STATUS "${PKG_CONFIG_EXECUTABLE} returned ${CURL_LINKFLAGS} for linking to libcurl")

	execute_process(COMMAND ${PKG_CONFIG_EXECUTABLE} "libcurl" "--cflags-only-other"
		OUTPUT_VARIABLE CURL_DEFINITONS
		OUTPUT_STRIP_TRAILING_WHITESPACE )
	add_definitions(${CURL_DEFINITONS})
	message(STATUS "${PKG_CONFIG_EXECUTABLE} returned ${CURL_DEFINITONS} for compiling with libcurl")
endif()



ADD_DEFINITIONS( ${CURL_CFLAGS} )
#add_to_global(link_directories  ${CURL_LIB_DIRS} )
INCLUDE_DIRECTORIES( ${CURL_INCLUDE_DIR} )

#----------------------------------------------------------------------------------------------------
# libnotify dependency check
#----------------------------------------------------------------------------------------------------

IF( OPTION_NOTIFY AND NOT WIN32 )
	pkg_check_modules( LIBNOTIFY libnotify )
	pkg_check_modules( GLIB glib-2.0 )
	IF( LIBNOTIFY_FOUND )
		INCLUDE_DIRECTORIES( ${LIBNOTIFY_INCLUDE_DIRS} )
		LINK_LIBRARIES( ${LIBNOTIFY_LIBRARIES} )
		INCLUDE_DIRECTORIES( ${GLIB_INCLUDE_DIRS} )
		LINK_LIBRARIES( ${GLIB_LIBRARIES} )
		ADD_DEFINITIONS( -DHAVE_LIBNOTIFY )
	ENDIF( LIBNOTIFY_FOUND )
ENDIF( OPTION_NOTIFY AND NOT WIN32 )


set(springlobbySrc
	aui/slbook.cpp
	aui/auimanager.cpp

	hosting/addbotdialog.cpp
	hosting/autobalancedialog.cpp
	hosting/autohost.cpp
	hosting/battlemaptab.cpp
	hosting/battleoptionstab.cpp
	hosting/battleroomlistctrl.cpp
	hosting/battleroomtab.cpp
	hosting/hostbattledialog.cpp
	hosting/mainjoinbattletab.cpp
	hosting/mainsingleplayertab.cpp
	hosting/mmoptionwindows.cpp
	hosting/battleroomdownloads.cpp
	hosting/autohostmanager.cpp
	hosting/battleroommmoptionstab.cpp

	options/downloadoptionspanel.cpp
	options/mainoptionstab.cpp
	options/springoptionstab.cpp
	options/groupoptionspanel.cpp
	options/lobbyoptionstab.cpp
	options/chatoptionstab.cpp

	playback/replaylist.cpp
	playback/savegamelist.cpp

	helper/TextCompletionDatabase.cpp
	helper/slhtmlwindow.cpp
	helper/wxtextctrlhist.cpp
	helper/colorbutton.cpp

	gui/toasternotification.cpp
	gui/wxbackgroundimage.cpp
	gui/statusbar.cpp
	gui/pastedialog.cpp
	gui/notificationmanager.cpp
	gui/taskbar.cpp
	gui/contentdownloaddialog.cpp
	gui/contentsearchresultslistctrl.cpp

	utils/activitynotice.cpp
	utils/misc.cpp
	utils/tasutil.cpp
	utils/md5.c
	utils/battleevents.cpp

	channel/channelchooser.cpp
	channel/channelchooserdialog.cpp
	channel/autojoinchanneldialog.cpp
	channel/channel.cpp
	channel/channellist.cpp
	channel/channellistctrl.cpp

	battlelist/battlelist.cpp
	battlelist/battlelistctrl.cpp
	battlelist/battlelistfilter.cpp
	battlelist/battlelisttab.cpp

	downloader/prdownloader.cpp
	downloader/downloadlistctrl.cpp
	downloader/downloadsobserver.cpp

	toasterbox/ToasterBox.cpp
	toasterbox/ToasterBoxWindow.cpp
	toasterbox/ToasterBoxWindowList.cpp

	chatpanelmenu.cpp
	agreementdialog.cpp
	base64.cpp
	battle.cpp
	crc.cpp
	chatlog.cpp
	chatpanel.cpp
	connectwindow.cpp
	countrycodes.cpp
	customlistctrl.cpp
	flagimages.cpp
	ibattle.cpp
	iconimagelist.cpp
	mainchattab.cpp
	maindownloadtab.cpp
	mainwindow.cpp
	mapctrl.cpp
	mapgridctrl.cpp
	mapselectdialog.cpp
	contentsearchresult.cpp
	nicklistctrl.cpp
	offlinebattle.cpp
	
	
	selectusersdialog.cpp
	iserver.cpp
	reconnectdialog.cpp
	serverevents.cpp
	singleplayerbattle.cpp
	singleplayertab.cpp
	socket.cpp
	spring.cpp
	springlobbyapp.cpp
	springprocess.cpp
	tasserver.cpp
	ui.cpp
	user.cpp
	useractions.cpp
	userlist.cpp
	serverselector.cpp
	
	lsl/src/lsl/battle/tdfcontainer.cpp #FIXME
	springsettings/frame.cpp
)

SET(SLSharedWithSettings
	helper/wxTranslationHelper.cpp
	uiutils.cpp
	updater/versionchecker.cpp
	updater/updatehelper.cpp
	utils/controls.cpp
	utils/conversion.cpp
	utils/customdialogs.cpp
	utils/globalevents.cpp
	utils/platform.cpp
	utils/slpaths.cpp
	utils/sltipwin.cpp
	utils/uievents.cpp
	utils/curlhelper.cpp
	crashreport.cpp
	settings.cpp
	infodialog.cpp
	gui/windowattributespickle.cpp
	gui/spinctl/generic/spinctlg.cpp
	gui/spinctl/generic/spinctrldouble.cpp
	helper/slconfig.cpp
	aboutbox.cpp
	log.cpp
)

FILE( GLOB header "*.h" )
FILE( GLOB header2 "*/*.h" )
# FILE( GLOB header3 "*/*/*.h" )

#grep -lR cpp\"$ * | sed -e "s/\.h/\.cpp/g"
SET( templatesources
	hosting/battleroommmoptionstab.cxx
	customvirtlistctrl.cpp
	downloader/httpdownloader.cpp
	playback/playbackfilter.cpp
	playback/playbacklist.cpp
	playback/playbacklistctrl.cpp
	playback/playbackthread.cpp
	playback/playbacktab.cpp
	utils/stringserializer.cpp
)

IF(WIN32)
	list(APPEND SLSharedWithSettings stacktrace.c)
ELSE(WIN32)
	list(APPEND SLSharedWithSettings stacktrace.cpp)
ENDIF(WIN32)
add_library(slcommon STATIC
		${SLSharedWithSettings}
	)
target_link_libraries(slcommon
		springlobbyversion
		libsettings
		lsl-unitsync
		${wxWidgets_LIBRARIES}
		${CURL_LIBRARIES}
	)
set_target_properties(slcommon PROPERTIES
		LINK_FLAGS "${CURL_LINKFLAGS}"
	)

add_subdirectory(downloader)
add_subdirectory(utils)

set(LSLSERVER OFF CACHE BOOL "disable lsl-server build" FORCE)
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lsl/CMakeLists.txt")
	MESSAGE(FATAL_ERROR "lsl/CMakeLists.txt doesn't exist, please run \ngit submodule update --init")
else()
	add_definitions(-DHAVE_WX -DHAVE_SPRINGLOBBY)
	Add_Subdirectory(lsl)
	INCLUDE_DIRECTORIES(lsl/src)
	remove_definitions(-DHAVE_WX -DHAVE_SPRINGLOBBY)
endif()
add_subdirectory(springsettings)
add_subdirectory(sound)
add_subdirectory(json)


set_source_files_properties(  ${templatesources} PROPERTIES HEADER_FILE_ONLY 1 )

#mylink( ${EXTRA_LIB_DIRS} ${link_directories} )

IF(WIN32)
	set(springlobby_RC_FILE springlobby.rc)
ENDIF()
add_subdirectory(updater)


ADD_EXECUTABLE(springlobby WIN32 MACOSX_BUNDLE
		${springlobbySrc}
		${springlobby_RC_FILE}
	)
#the updater needs to be build for windows only
IF(WIN32)
	TARGET_LINK_LIBRARIES( springlobby
		iphlpapi
		nettle
		ws2_32
	)
	install(TARGETS springlobby RUNTIME DESTINATION .)
ELSE (WIN32)
	install(TARGETS springlobby RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin )
	option(TEST_UNITSYNC_DEADLOCKS "check unitsync for deadlocks" ON)
	#only do this on platforms with awk expected to be installed
	if(TEST_UNITSYNC_DEADLOCKS)
		ADD_CUSTOM_TARGET(test_susynclib ALL COMMAND tools/test-susynclib.awk src/lsl/src/lslunitsync/unitsync.cpp WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
	endif()
ENDIF (WIN32)

TARGET_LINK_LIBRARIES(springlobby slcommon lsl-utils lsl-unitsync pr-downloader_static json springlobbyversion libsettings
		${wxWidgets_LIBRARIES}
		${Boost_LIBRARIES}
		${CURL_LIBRARIES}
		${CURL_LINKFLAGS}
		sound
	)
#add_dependencies(springlobby version)
IF(UNIX)
	FIND_PACKAGE(X11 REQUIRED)
	TARGET_LINK_LIBRARIES(springlobby ${X11_X11_LIB})
ENDIF(UNIX)

ENABLE_TESTING()
add_subdirectory(tests)

add_analyze(${springlobbySoundsrc} ${springlobbySrc} ${SLSharedWithSettings})
#add_format(${springlobbySoundsrc} ${springlobbySrc} ${SLSharedWithSettings} ${templatesources} ${header} ${header2})

