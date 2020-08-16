

find_path(STEAMWORKS_INCLUDE_DIR
	NAMES steam_api.h
	PATH_SUFFIXES "public/steam"
	PATHS ${STEAMWORKS_SDK}
	)

if (APPLE)
	find_library(STEAMWORKS_LIBRARY
		NAMES steam_api
		PATH_SUFFIXES "redistributable_bin/osx32"
		PATHS ${STEAMWORKS_SDK}
		)
elseif(Windows)
	if (BIT_32)
		find_library(STEAMWORKS_LIBRARY
			NAMES steam_api
			PATH_SUFFIXES "redistributable_bin"
			PATHS ${STEAMWORKS_SDK}
			)
	else()
		find_library(STEAMWORKS_LIBRARY
			NAMES steam_api64
			PATH_SUFFIXES "redistributable_bin/win64"
			PATHS ${STEAMWORKS_SDK}
			)
	endif()
else()
	if (BIT_32)
		find_library(STEAMWORKS_LIBRARY
			NAMES steam_api
			PATH_SUFFIXES "redistributable_bin/linux32"
			PATHS ${STEAMWORKS_SDK}
			)
	else ()
		find_library(STEAMWORKS_LIBRARY
			NAMES steam_api
			PATH_SUFFIXES "redistributable_bin/linux64"
			PATHS ${STEAMWORKS_SDK}
			)
	endif()
endif()

find_package_handle_standard_args(STEAMWORKS
	DEFAULT_MSG
	STEAMWORKS_LIBRARY
	STEAMWORKS_INCLUDE_DIR
	)
