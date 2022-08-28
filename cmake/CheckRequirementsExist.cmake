
MACRO(CHECK_REQUIREMENTS_EXIST)
    include(CheckIncludeFile)
    include(CheckFunctionExists)
    include(CheckLibraryExists)
    include(CheckStdCHeaders)
    include(CheckFunctionExists)
    include(CheckStructHasMember)
    include(CheckCSourceRuns)

    TEST_BIG_ENDIAN(IS_BIG_ENDIAN)
    list(APPEND CMAKE_REQUIRED_LIBRARIES m)
    list(APPEND CMAKE_REQUIRED_LIBRARIES crypt)

    # ## MATH
    check_c_source_compiles(
        "#include <math.h> 
        int main() {
            return (int)(cbrt(0.8));
        }"
        HAVE_CBRT)

    if(HAVE_CBRT)
        Message(STATUS "cbrt() - found")
    endif()

    check_c_source_compiles(
        "#include <math.h> 
        int main() {
            return (int)(isnormal(0.8));
        }"
        HAVE_ISNORMAL)

    if(HAVE_ISNORMAL)
        Message(STATUS "isnormal() - found")
    endif()

    check_c_source_compiles(
        "#include <math.h> 
        int main() {
            return (int)(lrint(0.8));
        }"
        HAVE_LRINT)

    if(HAVE_LRINT)
        Message(STATUS "lrint() - found")
    endif()

    check_c_source_compiles(
        "#include <math.h> 
        int main() {
            return (int)(log2(0.8));
        }"
        HAVE_LOG2)

    if(HAVE_LOG2)
        Message(STATUS "log2() - found")
    endif()

    check_type_size("_Bool" SIZEOF__Bool)

    if(SIZEOF__Bool GREATER 0)
        Message(STATUS "Found _Bool")
        set(HAVE__BOOL 1)
    endif()

    # ## CRYPT
    find_path(CRYPT_INCLUDE_DIRS
        NAMES crypt.h
        PATHS /usr/include /usr/local/include /usr/local/bic/include
        NO_DEFAULT_PATH
    )
    CHECK_SYMBOL_EXISTS(crypt crypt.h CRYPT_EXISTS)

    IF(NOT CRYPT_EXISTS)
        CHECK_LIBRARY_EXISTS(crypt crypt CRYPT_INCLUDE_DIRS CRYPT_LIB_EXISTS)
    ENDIF(NOT CRYPT_EXISTS)

    if(CRYPT_EXISTS OR CRYPT_LIB_EXISTS)
        set(HAVE_CRYPT 1)
        set(HAVE_CRYPT_H 1)
        MESSAGE(STATUS "libcrypt - found")
    endif()

    # ## Strings
    check_symbol_exists(strerror_r "string.h" HAVE_STRERROR_R)

    if(HAVE_STRERROR_R)
        set(HAVE_DECL_STRERROR_R 1)
    else()
        set(HAVE_DECL_STRERROR_R 0)
    endif()

    check_symbol_exists(SIGCHLD "signal.h" HAVE_SIGCHLD)

    # ## LIBRARIES
    CHECK_LIBRARY_EXISTS(intl gettext "/usr/local/lib/" HAVE_INTL)
    CHECK_LIBRARY_EXISTS(event event_base_loop "/usr/local/lib/" HAVE_LIBEVENT)
    CHECK_LIBRARY_EXISTS(event_extra event_base_new "/usr/local/lib/" HAVE_LIBEVENT_EXTRA)
    CHECK_LIBRARY_EXISTS(event_openssl bufferevent_openssl_socket_new "/usr/local/lib/" HAVE_EVENT_OPENSSL)

    # OPENSSL
    set(OPENSSL_USE_STATIC_LIBS TRUE)
    find_package(OpenSSL REQUIRED )

    if(OPENSSL_FOUND)
        target_link_libraries(netmud OpenSSL::SSL)
        target_link_libraries(ssl_slave OpenSSL::SSL)
        target_link_libraries(info_slave OpenSSL::SSL)
        set(HAVE_SSL 1)
        set(HAVE_DH_SET0_PQG 1)
        set(HAVE_EVP_MD_DO_ALL 1)
        set(HAVE_RAND_KEEP_RANDOM_DEVICES_OPEN 1)

    else()
        message(FATAL_ERROR, "The OpenSSL library must be installed to run PennMUSH.")
    endif()

    find_package(PkgConfig REQUIRED)

    if(NOT PKGCONFIG_FOUND)
        message(FATAL_ERROR, "pkg-config is required by CMake to ensure libraries are loaded, please add this tool to your system")
    endif()

    find_package(MariaDBClient)

    if(MariaDBClient_FOUND)
        include_directories(${MariaDBClient_INCLUDE_DIRS})
        target_link_libraries(netmud MariaDBClient::MariaDBClient)

        set(HAVE_MYSQL 1)
    else()
        message(FATAL_ERROR, "The MySQL library (mysql-devel package) must be installed to run PennMUSH.")
    endif()

    find_package(ZLIB )

    if(ZLIB_FOUND)
        target_link_libraries(netmud ZLIB::ZLIB)

        set(HAVE_GZBUFFER 1)
        set(HAVE_GZVPRINTF 1)
        set(HAVE_LIBZ 1)

    else()
        message(FATAL_ERROR, "The LIBZ library must be installed to run PennMUSH.")
    endif()

    # ## FileSystem
    if(EXISTS "/dev/urandom")
        set(HAVE_DEV_URANDOM 1)
    endif()

    if(EXISTS "/usr/share/zoneinfo")
        set(HAVE_ZONEINFO 1)
        SET(TZDIR "/usr/share/zoneinfo")
    elseif(EXISTS "/usr/share/lib/zoneinfo")
        set(HAVE_ZONEINFO 1)
        SET(TZDIR "/usr/share/lib/zoneinfo")
    endif()

    # ## HEADERS
    CHECK_STDC_HEADERS()

    check_symbol_exists("fputc_unlocked" "stdio.h" HAVE_PUTC_UNLOCKED)

    set(hdrs "sys/stat.h" "sys/time.h" "sys/types.h" "sys/eventfd.h" "sys/socket.h" "arpa/inet.h"
        "libintl.h" "netdb.h" "netinet/tcp.h" "netinet/in.h" "sys/un.h" "sys/resource.h" "sys/event.h" "sys/uio.h"
        "poll.h" "sys/select.h" "sys/inotify.h" "langinfo.h" "event2/event.h" "event2/dns.h"
        "fenv.h" "sys/param.h" "syslog.h" "sys/prctl.h" "byteswap.h" "endian.h" "sys/endian.h" "pthread.h"
        "sys/ucred.h" "sys/file.h" "stdbool.h" "sys/wait.h" "stdbool.h" "stdint.h" "inttypes.h" "sys/param.h" "unistd.h"
        "memory.h")

    foreach(header ${hdrs})
        string(REPLACE / _ HDR ${header})
        string(REPLACE . _ HDR ${HDR})
        string(TOUPPER ${HDR} HDR)

        Check_Include_File(${header} HAVE_${HDR})
    endforeach()

    # ## Functions
    set(funs "getdate" "getpagesize" "getrlimit" "getrusage" "getservbyname" "gettext" "getpid" "getppid" "setitimer"
        "nl_langinfo" "setsid" "setpgid" "setpgrp" "log2" "imaxdiv" "hypot" "getuid" "geteuid" "seteuid" "getpriority"
        "setpriority" "socketpair" "sigaction" "sigprocmask" "writev" "fcntl" "flock" "poll" "kqueue" "inotify_init1" "pread" "pwrite"
        "eventfd" "pledge" "pipe2" "syslog" "fetestexcept" "feclearexcept" "fdatasync" "usleep" "fullfsync" "localtime_r" "localtime_s"
        "gmtime_r" "isnan" "malloc_usable_size" "fork" "snprintf" "strcasecmp" "strncasecmp" "isnormal" "vasprintf" "strchrnul" "strdup" "strcoll"
        "strxfrm" "sysconf" "textdomain" "vsnprintf" "waitpid" "wait3" "wait" "union_wait" "getpid" "getppid" "poll" "posix_memalign" "writev" "fcntl" "flock"
        "inotify_init1" "pread" "pwrite" "posix_fadvise" "posix_fallocate" "feclearexcept" "fetestexcept" "sha" "dh_seto_pqg" "evp_md_do_all"
        "rand_keep_random_devices_open" "prctl" "hypot" "gzbuffer" "gzvprintf" "getc_unlocked" "fgets_unlocked" "fputs_unlocked" "ffs" "eventfd"
        "pledge" "getentropy" "arc4random_buf" "pthread_atfork" "syslog" "fdatasync" "usleep" "fullfsync" "localtime_r" "gmtime_r" "sigchld" "dev_urandom"
        "zoneinfo" "uptime" "bindtextdomain"
    )

    check_struct_has_member("struct sockaddr_in6" sin6_family "netinet/in.h" HAVE_STRUCT_SOCKADDR_IN6 LANGUAGE C)

    foreach(function ${funs})
        string(REPLACE / _ fun ${function})
        string(REPLACE . _ fun ${fun})
        string(TOUPPER ${fun} fun)

        Check_Function_Exists(${function} HAVE_${fun})
    endforeach()

    check_c_source_runs("#include <ctype.h>
    int main()
    {
        if (toupper('A') == 'A')
        return 0;
        else
        return 1;
    }" TOUPPER)

    if(TOUPPER EQUAL 1)
        set(HAVE_SAFE_TOUPPER 1)
    endif()

    # ## USER ARGS
    # TODO: Add support
    set(DONT_TRANSLATE 1)
    set(INFO_SLAVE 1)
    set(SSL_SLAVE 1)
    exec_program(which ARGS uptime OUTPUT_VARIABLE UPTIME)

    if(UPTIME)
        set(HAVE_UPTIME 1)
    endif()

    # ## Language features
    set(restrict __restrict)
    set(inline __inline)
endmacro(CHECK_REQUIREMENTS_EXIST)
