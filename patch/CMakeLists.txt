cmake_minimum_required(VERSION 3.22.1)
enable_language(CXX)
set(CMAKE_CXX_STANDARD 20) # C++20...
set(CMAKE_CXX_STANDARD_REQUIRED ON) #...is required...
set(CMAKE_CXX_EXTENSIONS OFF) #...without compiler extensions like gnu++11

#
# Set our project name
#
project(patch)

#
# Source files
#
set(patch_src
    "add_dll_ref.hpp"
    "config.hpp"
    "config_rw.hpp"
    "cryptostring.hpp"
    "debug_log.hpp"
    "gate.hpp"
    "global.hpp"
    "global_minimum.hpp"
    "hash.hpp"
    "init.cpp"
    "init.hpp"
    "json.h"
    "macro.h"
    "moduledata.hpp"
    "multi_threading.hpp"
    "mylua.hpp"
    "mywindow.hpp"
    "offset_address.hpp"
    "overwrite_resource.cpp"
    "overwrite_resource.hpp"
    "patch.cpp"
    "patch.hpp"
    "patch_access_key.hpp"
    "patch_alpha_bg.hpp"
    "patch_aviutl_wndproc_override.cpp"
    "patch_aviutl_wndproc_override.hpp"
    "patch_base.hpp"
    "patch_console.cpp"
    "patch_console.hpp"
    "patch_copybuffer_smem.hpp"
    "patch_exception_history.hpp"
    "patch_exception_log.cpp"
    "patch_exception_log.hpp"
    "patch_exception_log_dialog.cpp"
    "patch_exception_log_dialog.hpp"
    "patch_exeditwindow_sizing.hpp"
    "patch_exo_aviutlfilter.hpp"
    "patch_exo_sceneidx.hpp"
    "patch_exo_specialcolorconv.hpp"
    "patch_exo_trackminusval.cpp"
    "patch_exo_trackminusval.hpp"
    "patch_exo_trackparam.hpp"
    "patch_fast.hpp"
    "patch_fast_border.cpp"
    "patch_fast_border.hpp"
    "patch_fast_cl.hpp"
    "patch_fast_exeditwindow.cpp"
    "patch_fast_exeditwindow.hpp"
    "patch_fast_flash.cpp"
    "patch_fast_flash.hpp"
    "patch_fast_getputpixeldata.cpp"
    "patch_fast_getputpixeldata.hpp"
    "patch_fast_polortransform.cpp"
    "patch_fast_polortransform.hpp"
    "patch_fast_radiationalblur.cpp"
    "patch_fast_radiationalblur.hpp"
    "patch_fast_setting_dialog.cpp"
    "patch_fast_setting_dialog.hpp"
    "patch_fast_text.cpp"
    "patch_fast_text.hpp"
    "patch_font_dialog.hpp"
    "patch_helpful_msgbox.cpp"
    "patch_helpful_msgbox.hpp"
    "patch_ignore_media_param_reset.hpp"
    "patch_lua.cpp"
    "patch_lua.hpp"
    "patch_lua_getvalueex.cpp"
    "patch_lua_getvalueex.hpp"
    "patch_lua_rand.cpp"
    "patch_lua_rand.hpp"
    "patch_lua_randex.cpp"
    "patch_lua_randex.hpp"
    "patch_obj_lensblur.cpp"
    "patch_obj_lensblur.hpp"
    "patch_redo.cpp"
    "patch_redo.hpp"
    "patch_scroll_objdlg.hpp"
    "patch_setting_dialog_move.hpp"
    "patch_setting_dialog_wndproc_override.cpp"
    "patch_setting_dialog_wndproc_override.hpp"
    "patch_setting_gui.hpp"
    "patch_splash.cpp"
    "patch_splash.hpp"
    "patch_susie_load.cpp"
    "patch_susie_load.hpp"
    "patch_sysinfo_write.hpp"
    "patch_text_op_size.hpp"
    "patch_theme_cc.hpp"
    "patch_tra_aviutlfilter.hpp"
    "patch_undo.cpp"
    "patch_undo.hpp"
    "resource.h"
    "restorable_patch.hpp"
    "stopwatch.hpp"
    "timer.hpp"
    "util.hpp"
    "util_int.hpp"
    "util_magic.hpp"
    "util_others.cpp"
    "util_others.hpp"
    "util_pe.hpp"
    "util_resource.hpp"
    "version.hpp"
)

#
# find libraries and include
#

# find thread library
find_package(Threads REQUIRED)
# find OpenCL library
find_package(OpenCL REQUIRED)
# Get Dependencies
include(FetchContent)
FetchContent_Declare(
    aviutl_exedit_sdk
    GIT_REPOSITORY https://github.com/ePi5131/aviutl_exedit_sdk.git
    GIT_TAG 219be3ede890ab5998618d7f11d9e1987d5f9637
)
FetchContent_Declare(
    winwrap
    # GIT_REPOSITORY https://github.com/ePi5131/winwrap.git
    GIT_REPOSITORY https://github.com/yumetodo/winwrap.git
    # GIT_TAG de29f34b822f330ed2d13e6a3b2fa35c96f456ee
    GIT_TAG 012e25a669bc348d6027a5eb9bc1090fceb6085d
)
if(NOT MSVC)
FetchContent_Declare(
  fmtlib
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG b6f4ceaed0a0a24ccf575fab6c56dd50ccf6f1a9 # 8.1.1
    )
  FetchContent_MakeAvailable(aviutl_exedit_sdk winwrap fmtlib)
else()
  FetchContent_MakeAvailable(aviutl_exedit_sdk winwrap)
endif()

# find Boost
# https://cmake.org/cmake/help/v3.7/module/FindBoost.html
if(${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} LESS 3.8)
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/modules)
    include(FindBoost)
    message(STATUS "FindBoost: commit 9a8881c975ba8d814c39d52c370f3277afa80fda is used.")
endif()
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_STATIC_RUNTIME OFF)
set(Boost_USE_DEBUG_RUNTIME OFF)
add_definitions(${Boost_LIB_DIAGNOSTIC_DEFINITIONS})
find_package(Boost 1.77.0 REQUIRED)

#
# Update compiler waring flags
#
if(MSVC)
  # Force to always compile with W4
  if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  endif()
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  # Update if necessary
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-long-long -pedantic")
endif()

#
# Compile
#
add_library(patch SHARED ${patch_src})
set_target_properties(patch PROPERTIES SUFFIX ".aul")
if(MSVC)
  target_include_directories(patch
    PUBLIC ${Boost_INCLUDE_DIRS}
    PUBLIC ${OpenCL_INCLUDE_DIRS}
    PUBLIC ${aviutl_exedit_sdk_SOURCE_DIR}
    PUBLIC ${winwrap_SOURCE_DIR}
  )
else()
  message("fmtlib/fmt loaded." ${fmtlib_SOURCE_DIR})
  message(${aviutl_exedit_sdk_SOURCE_DIR})
  target_include_directories(patch
    PUBLIC ${Boost_INCLUDE_DIRS}
    PUBLIC ${OpenCL_INCLUDE_DIRS}
    PUBLIC ${aviutl_exedit_sdk_SOURCE_DIR}
    PUBLIC ${winwrap_SOURCE_DIR}
    PUBLIC ${fmtlib_SOURCE_DIR}/include
  )
endif()


#
# Link
#
target_link_libraries(patch Threads::Threads ${OpenCL_LIBRARIES})

