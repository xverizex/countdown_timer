include (FindPkgConfig)

pkg_check_modules (OBS "libobs")

add_library (countdown_binary SHARED
	cb.c
	)

target_include_directories (countdown_binary PUBLIC
	${OBS_INCLUDE_DIRS}
	)

target_link_libraries (countdown_binary PUBLIC
	${OBS_LIBRARIES}
	)
