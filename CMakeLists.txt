cmake_policy(SET CMP0048 NEW)
cmake_policy(SET CMP0076 NEW)
cmake_policy(SET CMP0075 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0048 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0076 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0075 NEW)
project(netmud VERSION 1.8.8 DESCRIPTION PennMUSH)
cmake_minimum_required(VERSION 3.0)

# SET (CMAKE_C_COMPILER_WORKS 1)
# SET (CMAKE_CXX_COMPILER_WORKS 1)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_HOME_DIRECTORY}/cmake/")
set(CMAKE_C_COMPILER_ID "GNU")
set(CMAKE_SOURCE_DIR "${CMAKE_HOME_DIRECTORY}/src/")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_HOME_DIRECTORY}/src/")
add_executable(netmud)
add_executable(ssl_slave)
add_executable(info_slave)
set(DCMAKE_EXPORT_COMPILE_COMMANDS 1)

# set(CMAKE_CXX_COMPILER_ID "GNU")
set_property(GLOBAL PROPERTY C_STANDARD 17)

target_sources(info_slave PUBLIC src/info_slave.c src/strdup.c src/sig.c src/wait.c src/mysocket.c)

target_sources(ssl_slave PUBLIC src/ssl_slave.c src/strdup.c src/sig.c src/wait.c src/myssl.c src/pcg_basic.c src/mysocket.c)

target_sources(netmud PUBLIC
    src/access.c
    src/atr_tab.c
    src/attrib.c
    src/boolexp.c
    src/bsd.c
    src/bufferq.c
    src/charconv.c
    src/chunk.c
    src/cJSON.c
    src/cmdlocal.c
    src/cmds.c
    src/command.c
    src/compress.c
    src/conf.c
    src/connlog.c
    src/cque.c
    src/create.c
    src/db.c
    src/destroy.c
    src/extchat.c
    src/extmail.c
    src/filecopy.c
    src/flaglocal.c
    src/flags.c
    src/funcrypt.c
    src/function.c
    src/fundb.c
    src/funjson.c
    src/funlist.c
    src/funlocal.c
    src/funmath.c
    src/funmisc.c
    src/funstr.c
    src/funtime.c
    src/funufun.c
    src/game.c
    src/hash_function.c
    src/help.c
    src/htab.c
    src/intmap.c
    src/local.c
    src/lock.c
    src/log.c
    src/look.c
    src/malias.c
    src/map_file.c
    src/markup.c
    src/match.c
    src/memcheck.c
    src/move.c
    src/mycrypt.c
    src/mymalloc.c
    src/mysocket.c
    src/myrlimit.c
    src/myssl.c
    src/notify.c
    src/odbc.c
    src/odbc_mushdb.c
    src/parse.c
    src/pcg_basic.c
    src/player.c
    src/plyrlist.c
    src/predicat.c
    src/privtab.c
    src/info_master.c
    src/ptab.c
    src/rob.c
    src/services.c
    src/set.c
    src/sig.c
    src/sort.c
    src/speech.c
    src/spellfix.c
    src/sql.c
    src/sqlite3.c
    src/ssl_master.c
    src/strdup.c
    src/strtree.c
    src/strutil.c
    src/tables.c
    src/testframework.c
    src/timer.c
    src/tz.c
    src/uint.c
    src/unparse.c
    src/utf_impl.c
    src/utils.c
    src/version.c
    src/wait.c
    src/warnings.c
    src/websock.c
    src/wild.c
    src/wiz.c
    src/combat.c
    src/space_alert.c
    src/space_ansi.c
    src/space_balance.c
    src/space_border.c
    src/space_conf.c
    src/space_crypt.c
    src/space_error.c
    src/space_format.c
    src/space_get.c
    src/space_iterate.c
    src/space_log.c
    src/space_main.c
    src/space_misc.c
    src/space_nebula.c
    src/space_notify.c
    src/space_output.c
    src/space_put.c
    src/space_read.c
    src/space_report.c
    src/space_set.c
    src/space_status.c
    src/space_unparse.c
    src/space_utils.c
    src/space_variables.c
    src/space_write.c
    src/space_json.c
)
include(TestBigEndian)
include(CheckTypeSize)
include(CheckFunctionExists)
include(CheckTypeSize)
include(UsePkgConfig)
include(CheckSymbolExists)

include(CheckRequirementsExist)
include(FindMariaDBClient)

include(CheckIncludeFiles)
include(CheckCSourceCompiles)
include(CheckMathFunc)
include(ProcessPennFiles)


# ## check types

# Generic libraries
set(SETPGRP_VOID 1)

include(ExternalProject)

set(PCRE2_VERSION "pcre2-10.39")
set(PCRE2_TARBALL "${PCRE2_VERSION}.tar.gz")
set(PCRE2_SRC ${CMAKE_HOME_DIRECTORY}/build/${PCRE2_VERSION})
set(PCRE2_BIN ${CMAKE_HOME_DIRECTORY}/build/pcre2)

ExternalProject_Add(pcre2
    SOURCE_DIR ${PCRE2_SRC}
    BINARY_DIR ${PCRE2_BIN}/build/pcre2-bin
    INSTALL_DIR ${CMAKE_HOME_DIRECTORY}/3rdParty/pcre2
    DOWNLOAD_DIR ${CMAKE_HOME_DIRECTORY}/build
    URL "https://github.com/PhilipHazel/pcre2/releases/download/${PCRE2_VERSION}/${PCRE2_TARBALL}"
    CONFIGURE_COMMAND ${PCRE2_SRC}/configure --disable-shared --with-pic --prefix=${CMAKE_HOME_DIRECTORY}/3rdParty/pcre2 --enable-jit=auto
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

add_dependencies(netmud pcre2)
add_dependencies(info_slave file_toucher)
add_dependencies(ssl_slave file_toucher)

target_link_libraries(ssl_slave -lm -ldl -levent_core -levent_extra -levent_openssl)
target_link_libraries(info_slave -lm -ldl -levent_core -levent_extra -levent_openssl)
target_link_libraries(netmud -lm -ldl -lcrypt ${CMAKE_HOME_DIRECTORY}/3rdParty/pcre2/lib/libpcre2-8.a)
target_include_directories(ssl_slave PRIVATE ${CMAKE_HOME_DIRECTORY}/3rdParty/pcre2/include 
PRIVATE ${CMAKE_HOME_DIRECTORY}/hdrs 
PRIVATE ${CMAKE_HOME_DIRECTORY})

target_include_directories(info_slave PRIVATE ${CMAKE_HOME_DIRECTORY}/3rdParty/pcre2/include 
PRIVATE ${CMAKE_HOME_DIRECTORY}/hdrs 
PRIVATE ${CMAKE_HOME_DIRECTORY})

target_include_directories(netmud PRIVATE ${CMAKE_HOME_DIRECTORY}/3rdParty/pcre2/include 
PRIVATE ${CMAKE_HOME_DIRECTORY}/hdrs 
PRIVATE ${CMAKE_HOME_DIRECTORY} 
)

target_precompile_headers(netmud PUBLIC "${CMAKE_HOME_DIRECTORY}/config.h" PUBLIC "${CMAKE_HOME_DIRECTORY}/confmagic.h")
target_precompile_headers(ssl_slave PUBLIC "${CMAKE_HOME_DIRECTORY}/config.h" PUBLIC "${CMAKE_HOME_DIRECTORY}/confmagic.h")
target_precompile_headers(info_slave PUBLIC "${CMAKE_HOME_DIRECTORY}/config.h" PUBLIC "${CMAKE_HOME_DIRECTORY}/confmagic.h")

target_compile_options(info_slave PRIVATE -O2 -Wno-unknown-pragmas -Wall -Wextra -Wno-comment -Wno-char-subscripts -funsigned-char -DSLAVE -DINFO_SLAVE)
target_compile_options(ssl_slave PRIVATE -O2 -Wno-unknown-pragmas -Wall -Wextra -Wno-comment -Wno-char-subscripts -funsigned-char -DSLAVE -DSSL_SLAVE)
target_compile_options(netmud PRIVATE -g -O2 -pipe -Og -march=native -Wall -Wextra -Wpedantic -Wno-comment -Wno-char-subscripts -funsigned-char -ggdb -DPROFILING)
CHECK_REQUIREMENTS_EXIST()

CONFIGURE_FILE(${CMAKE_HOME_DIRECTORY}/config.h.in ${CMAKE_HOME_DIRECTORY}/config.h)

add_custom_target(file_toucher
    COMMAND ${CMAKE_COMMAND} -E touch_nocreate ${CMAKE_HOME_DIRECTORY}/src/mysocket.c
    COMMENT "Rebuilding mysocket.c"
)

add_custom_command(TARGET ssl_slave

    # Run after all other rules within the target have been executed
    POST_BUILD
    COMMAND rm -f game/netmush
    COMMAND rm -f game/info_slave
    COMMAND rm -f game/ssl_slave
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_HOME_DIRECTORY}/src/netmud ${CMAKE_HOME_DIRECTORY}/game/netmush
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_HOME_DIRECTORY}/src/info_slave ${CMAKE_HOME_DIRECTORY}/game/info_slave
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_HOME_DIRECTORY}/src/ssl_slave ${CMAKE_HOME_DIRECTORY}/game/ssl_slave
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_HOME_DIRECTORY}/build/cmds.h ${CMAKE_HOME_DIRECTORY}/hdrs/cmds.h
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_HOME_DIRECTORY}/build/cmds.h ${CMAKE_HOME_DIRECTORY}/hdrs/funs.h
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_HOME_DIRECTORY}/build/cmds.h ${CMAKE_HOME_DIRECTORY}/hdrs/switches.h
    COMMENT "Installing Links"
    
)

PennMUSH_MKCMDS()
PennMUSH_MKSWITCHES()
