FetchContent_Declare(
	curl
	GIT_REPOSITORY	https://github.com/curl/curl.git
	GIT_TAG			curl-8_6_0
	SOURCE_DIR		${CMAKE_CURRENT_SOURCE_DIR}/build/_deps/curl
	GIT_PROGRESS	true
	GIT_SHALLOW		true
)

FetchContent_GetProperties(curl)
if(NOT curl_POPULATED)
	FetchContent_Populate(curl)
	
	if(USE_SHARED_LIBS)
		set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
		set(LLVM_USE_CRT_DEBUG MDd CACHE STRING "" FORCE)
		set(LLVM_USE_CRT_MINSIZEREL MD CACHE STRING "" FORCE)
		set(LLVM_USE_CRT_RELEASE MD CACHE STRING "" FORCE)
		set(LLVM_USE_CRT_RELWITHDEBINFO MD CACHE STRING "" FORCE)
		set(OPENSSL_USE_STATIC_LIBS OFF CACHE BOOL "" FORCE)
		if(MSVC)
			set(CURL_STATIC_CRT OFF CACHE BOOL "" FORCE)
			set(OPENSSL_MSVC_STATIC_RT OFF CACHE BOOL "" FORCE)
		endif()
	else()
		add_definitions(-DCURL_STATICLIB)
		set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
		set(LLVM_USE_CRT_DEBUG MTd CACHE STRING "" FORCE)
		set(LLVM_USE_CRT_MINSIZEREL MT CACHE STRING "" FORCE)
		set(LLVM_USE_CRT_RELEASE MT CACHE STRING "" FORCE)
		set(LLVM_USE_CRT_RELWITHDEBINFO MT CACHE STRING "" FORCE)
		set(OPENSSL_USE_STATIC_LIBS ON CACHE BOOL "" FORCE)
		if(MSVC)
			set(CURL_STATIC_CRT ON CACHE BOOL "" FORCE)
			set(OPENSSL_MSVC_STATIC_RT ON CACHE BOOL "" FORCE)
		endif()
	endif()

	set(HTTP_ONLY ON CACHE BOOL "" FORCE)
	set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
	set(BUILD_CURL_EXE OFF CACHE BOOL "" FORCE)
	set(BUILD_LIBCURL_DOCS OFF CACHE BOOL "" FORCE)
	set(CURL_BROTLI OFF CACHE BOOL "" FORCE)
	set(CURL_CA_FALLBACK OFF CACHE BOOL "" FORCE)
	set(CURL_DISABLE_ALTSVC ON CACHE BOOL "" FORCE)
	set(CURL_DISABLE_COOKIES ON CACHE BOOL "" FORCE)
	set(CURL_DISABLE_CRYPTO_AUTH OFF CACHE BOOL "" FORCE)
	set(CURL_DISABLE_DICT ON CACHE BOOL "" FORCE)
	set(CURL_DISABLE_DOH ON CACHE BOOL "" FORCE)
	set(CURL_DISABLE_FILE OFF CACHE BOOL "" FORCE)
	set(CURL_DISABLE_FTP ON CACHE BOOL "" FORCE)
	set(CURL_DISABLE_GETOPTIONS ON CACHE BOOL "" FORCE)
	set(CURL_DISABLE_GOPHER ON CACHE BOOL "" FORCE)
	set(CURL_DISABLE_HSTS ON CACHE BOOL "" FORCE)
	set(CURL_DISABLE_HTTP OFF CACHE BOOL "" FORCE)
	set(CURL_DISABLE_HTTP_AUTH OFF CACHE BOOL "" FORCE)
	set(CURL_DISABLE_IMAP ON CACHE BOOL "" FORCE)
	set(CURL_DISABLE_LDAP ON CACHE BOOL "" FORCE)
	set(CURL_DISABLE_LDAPS ON CACHE BOOL "" FORCE)
	set(CURL_DISABLE_LIBCURL_OPTION ON CACHE BOOL "" FORCE)
	set(CURL_DISABLE_MIME ON CACHE BOOL "" FORCE)
	set(CURL_DISABLE_MQTT ON CACHE BOOL "" FORCE)
	set(CURL_DISABLE_NETRC ON CACHE BOOL "" FORCE)
	set(CURL_DISABLE_NTLM ON CACHE BOOL "" FORCE)
	set(CURL_DISABLE_OPENSSL_AUTO_LOAD_CONFIG OFF CACHE BOOL "" FORCE)
	set(CURL_DISABLE_PARSEDATE ON CACHE BOOL "" FORCE)
	set(CURL_DISABLE_POP3 ON CACHE BOOL "" FORCE)
	set(CURL_DISABLE_PROGRESS_METER ON CACHE BOOL "" FORCE)
	set(CURL_DISABLE_PROXY ON CACHE BOOL "" FORCE)
	set(CURL_DISABLE_RTSP ON CACHE BOOL "" FORCE)
	set(CURL_DISABLE_SHUFFLE_DNS ON CACHE BOOL "" FORCE)
	set(CURL_DISABLE_SMB ON CACHE BOOL "" FORCE)
	set(CURL_DISABLE_SMTP ON CACHE BOOL "" FORCE)
	set(CURL_DISABLE_SOCKETPAIR ON CACHE BOOL "" FORCE)
	set(CURL_DISABLE_TELNET ON CACHE BOOL "" FORCE)
	set(CURL_DISABLE_TFTP ON CACHE BOOL "" FORCE)
	set(CURL_DISABLE_VERBOSE_STRINGS ON CACHE BOOL "" FORCE)
	set(CURL_ENABLE_EXPORT_TARGET ON CACHE BOOL "" FORCE)
	set(CURL_ENABLE_SSL ON CACHE BOOL "" FORCE)
	set(CURL_HIDDEN_SYMBOLS ON CACHE BOOL "" FORCE)
	set(CURL_LTO OFF CACHE BOOL "" FORCE)
	set(CURL_USE_BEARSSL OFF CACHE BOOL "" FORCE)
	set(CURL_USE_GSSAPI OFF CACHE BOOL "" FORCE)
	set(CURL_USE_LIBPSL OFF CACHE BOOL "" FORCE)
	set(CURL_USE_LIBSSH OFF CACHE BOOL "" FORCE)
	set(CURL_USE_LIBSSH2 OFF CACHE BOOL "" FORCE)
	set(CURL_USE_MBEDTLS OFF CACHE BOOL "" FORCE)
	set(CURL_USE_NSS OFF CACHE BOOL "" FORCE)
	set(CURL_USE_OPENSSL ON CACHE BOOL "" FORCE)
	set(CURL_USE_SCHANNEL OFF CACHE BOOL "" FORCE)
	set(CURL_USE_WOLFSSL OFF CACHE BOOL "" FORCE)
	set(CURL_WERROR OFF CACHE BOOL "" FORCE)
	set(CURL_ZLIB AUTO CACHE STRING "" FORCE)
	set(CURL_ZSTD OFF CACHE BOOL "" FORCE)
	
	if(NOT CMAKE_DEBUG_POSTFIX)
	  set(CMAKE_DEBUG_POSTFIX _debug)
	endif()
	if(NOT CMAKE_RELEASE_POSTFIX)
	  set(CMAKE_RELEASE_POSTFIX)
	endif()
	if(NOT CMAKE_MINSIZEREL_POSTFIX)
	  set(CMAKE_MINSIZEREL_POSTFIX _minsizerel)
	endif()
	if(NOT CMAKE_RELWITHDEBINFO_POSTFIX)
	  set(CMAKE_RELWITHDEBINFO_POSTFIX _reldeb)
	endif()

	add_definitions(-DUSE_OPENSSL)
	
	##EXCLUDE_FROM_ALL reject install for this target
	add_subdirectory(${curl_SOURCE_DIR} EXCLUDE_FROM_ALL)	

	if(USE_SHARED_LIBS)
		set(CURL_LIBRARIES libcurl_shared)
		set_target_properties(libcurl_shared PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${FINAL_BIN_DIR}")
		set_target_properties(libcurl_shared PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${FINAL_BIN_DIR}")
		set_target_properties(libcurl_shared PROPERTIES RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${FINAL_BIN_DIR}")
		set_target_properties(libcurl_shared PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${FINAL_BIN_DIR}")
  		if (TARGET libcurl_object)
			set_target_properties(libcurl_shared PROPERTIES FOLDER 3rdparty/Shared/curl)
			set_target_properties(libcurl_object PROPERTIES FOLDER 3rdparty/Shared/curl)
			set_target_properties(libcurl_object PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${FINAL_BIN_DIR}")
			set_target_properties(libcurl_object PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${FINAL_BIN_DIR}")
			set_target_properties(libcurl_object PROPERTIES RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${FINAL_BIN_DIR}")
			set_target_properties(libcurl_object PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${FINAL_BIN_DIR}")
		else()
			set_target_properties(libcurl_shared PROPERTIES FOLDER 3rdparty/Shared)
  		endif()
		if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
			target_compile_options(libcurl_shared PRIVATE -Wno-everything) # disable all warnings, since im not maintaining this lib
			if (TARGET libcurl_object)
				target_compile_options(libcurl_object PRIVATE -Wno-everything) # disable all warnings, since im not maintaining this lib
			endif()
		endif()
	else()
		set(CURL_LIBRARIES libcurl_static)
		set_target_properties(libcurl_static PROPERTIES FOLDER 3rdparty/Static)
  		if (TARGET libcurl_object)
			set_target_properties(libcurl_object PROPERTIES FOLDER 3rdparty/Static)
		endif()
		if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
			target_compile_options(libcurl_static PRIVATE -Wno-everything) # disable all warnings, since im not maintaining this lib
	  		if (TARGET libcurl_object)
				target_compile_options(libcurl_object PRIVATE -Wno-everything) # disable all warnings, since im not maintaining this lib
			endif()
		endif()
	endif()

	set(CURL_INCLUDE_DIR ${curl_SOURCE_DIR}/include)
	include_directories(${CURL_INCLUDE_DIR}/curl)
endif()
