#! External package: SDL2



SDL2 = SDL2
SDL2_VERSION := $(shell $(call packages_getversion,$(SDL2)))
SDL2_DIR = $(LIBDIR)$(SDL2)/
SDL2_BIN = $(SDL2_DIR)bin/$(OSMODE)/
SDL2_INCLUDE = $(SDL2_BIN)include/SDL2/
SDL2_LINKDIR = $(SDL2_BIN)bin/
SDL2_LINKLIB = -lSDL2
SDL2_LINK = -L$(SDL2_LINKDIR) $(SDL2_LINKLIB)
# on MacOS, SDL2 is a framework, so linking is complicated
ifeq ($(OSMODE),macos)
	SDL2_INCLUDE = $(SDL2_BIN)SDL2.framework/Headers/
	SDL2_LINKDIR = $(SDL2_BIN)
	SDL2_LINKLIB = -framework SDL2
	SDL2_LINK = -F$(SDL2_LINKDIR) $(SDL2_LINKLIB)
endif



SDL2_URL = https://www.libsdl.org/release/
SDL2_PKG = SDL2-$(SDL2_VERSION).zip
SDL2_PKG_INSTALL = \
	@printf $(IO_YELLOW)"WARNING"$(IO_RESET)": Unsupported platform: SDL2 must be configured manually""\n"

ifeq ($(OSMODE),other)
else ifneq ($(filter $(OSMODE), win32 win64),)
	SDL2_PKG = SDL2-devel-$(SDL2_VERSION)-mingw.tar.gz
	SDL2_PKG_INSTALL = \
		tar -xf $(SDL2_PKG) --directory=$(SDL2_DIR) ; \
		{ mv -f $(SDL2_DIR)SDL2-$(SDL2_VERSION)/* $(SDL2_DIR) && rmdir $(SDL2_DIR)SDL2-$(SDL2_VERSION) ; } ; \
		{ mkdir -p $(SDL2_DIR)bin/win32/ && mv $(SDL2_DIR)x86_64-w64-mingw32/* $(SDL2_DIR)bin/win32/ && rmdir $(SDL2_DIR)x86_64-w64-mingw32 ; } ; \
		{ mkdir -p $(SDL2_DIR)bin/win64/ && mv $(SDL2_DIR)i686-w64-mingw32/*   $(SDL2_DIR)bin/win64/ && rmdir $(SDL2_DIR)i686-w64-mingw32   ; } ;
	SDL2_GETVERSIONS = \
		grep 'mingw.tar.gz'
else ifeq ($(OSMODE),macos)
	SDL2_PKG = SDL2-$(SDL2_VERSION).dmg
	SDL2_PKG_INSTALL = \
		listing=`hdiutil attach $(SDL2_PKG) | grep Volumes` ; \
		volume=`echo "$$listing" | cut -f 3` ; \
		cp -rf "$$volume"/SDL2.framework $(SDL2_BIN) ; \
		hdiutil detach `echo "$$listing" | cut -f 1`
	SDL2_GETVERSIONS = \
		grep '.dmg'
else ifeq ($(OSMODE),linux)
	SDL2_PKG = SDL2-$(SDL2_VERSION).zip
	SDL2_PKG_INSTALL = \
		unzip $(SDL2_PKG) -d $(SDL2_BIN) ; \
		cd $(SDL2_BIN) ; \
		./configure && make && make install
	SDL2_GETVERSIONS = \
		grep '.zip'
else
$(error Unsupported platform: external package 'SDL2' must be configured manually)
endif



#! The shell command to retrieve and output list of newer versions, if any
package_SDL2_checkupdates = \
	curl --silent $(SDL2_URL) \
	| grep 'SDL2' \
	| cut -d'"' -f 8 \
	| $(SDL2_GETVERSIONS) \
	| sort --version-sort \
	| awk -v found=0 '\
	{\
		if (/$(SDL2_VERSION)/) { found = 1 }\
		else if (found) { found += 1; print; }\
	}' \
	| sed -E 's/SDL2-.*([0-9]+\.[0-9]+\.[0-9]+).*/\1/g' \



.PHONY:\
package-SDL2 #! downloads the package, according to the version number set
package-SDL2:
	@$(call packages_setversion,$(SDL2),$(SDL2_VERSION))
	@printf $(IO_CYAN)"Downloading package: $(SDL2)@$(SDL2_VERSION)..."$(IO_RESET)"\n"
	@mkdir -p $(SDL2_BIN)
	@curl $(SDL2_URL)$(SDL2_PKG) --progress-bar --output $(SDL2_PKG)
	@$(SDL2_PKG_INSTALL)
	@rm -f $(SDL2_PKG)
	@printf $(IO_GREEN)"SUCCESS!"$(IO_RESET)"\n"



.PHONY:\
update-SDL2 #! updates the package to the latest version
update-SDL2:
	@printf $(IO_CYAN)"Checking new versions for package: $(SDL2)..."$(IO_RESET)"\n"
	@echo "=> Current version is: $(SDL2_VERSION)"
	@new_versions=`$(call package_SDL2_checkupdates)` ; \
	if [ -z "$${new_versions}" ]; then \
		printf "Newest version already set.\n" ; \
	else \
		newer_version=`echo "$${new_versions}" | tail -1` ; \
		printf "Found `echo "$${new_versions}" | wc -l | xargs` new versions.\n" ; \
		printf "Newest version is '$${newer_version}'.\n" ; \
		$(MAKE) package-SDL2  SDL2_VERSION=$$newer_version ; \
	fi
