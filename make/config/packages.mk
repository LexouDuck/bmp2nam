#! This file stores project-specific variables relating to package management



#! Define the list of dependency packages for this project
PACKAGES = \
	libccc \
	SDL2 \



#! The makefile variable name for each packages (typically uppercase)
PACKAGES_VAR = \
	LIBCCC \
	LIBSDL \



PACKAGES_VERSION = $(addsuffix _VERSION, $(PACKAGES_VAR))
PACKAGES_DIR     = $(addsuffix _DIR,     $(PACKAGES_VAR))
PACKAGES_BIN     = $(addsuffix _BIN,     $(PACKAGES_VAR))
PACKAGES_INCLUDE = $(addsuffix _INCLUDE, $(PACKAGES_VAR))
PACKAGES_LINKDIR = $(addsuffix _LINKDIR, $(PACKAGES_VAR))
PACKAGES_LINKLIB = $(addsuffix _LINKLIB, $(PACKAGES_VAR))
PACKAGES_LINK    = $(addsuffix _LINK,    $(PACKAGES_VAR))



#! This variable sets the command which is used to download external packages
DOWNLOAD = curl --progress-bar
