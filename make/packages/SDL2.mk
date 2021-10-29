#! External package: SDL2



LIBSDL = SDL2
LIBSDL_DIR = $(LIBDIR)$(LIBSDL)/
LIBSDL_BIN = $(LIBSDL_DIR)bin/$(OSMODE)/
LIBSDL_VERFILE = $(LIBDIR)$(LIBSDL).ver
LIBSDL_VERSION := $(shell cat $(LIBSDL_VERFILE) | cut -d'@' -f 2 | cut -d'-' -f 1)

LIBSDL_INCLUDE = $(LIBSDL_DIR)include/
LIBSDL_LINKDIR = $(LIBSDL_BIN)
LIBSDL_LINKLIB = -lSDL2
LIBSDL_LINK = -L$(LIBSDL_LINKDIR) $(LIBSDL_LINKLIB)
# on MacOS, SDL2 is a framework, so linking is complicated
ifeq ($(OSMODE),macos)
	LIBSDL_INCLUDE = $(LIBSDL_BIN)SDL2.framework/Headers/
	LIBSDL_LINKDIR = $(LIBSDL_BIN)
	LIBSDL_LINKLIB = -framework SDL2
	LIBSDL_LINK = -F$(LIBSDL_LINKDIR) $(LIBSDL_LINKLIB)
endif

LIBSDL_URL = https://www.libsdl.org/release/
LIBSDL_PKG = SDL2-$(LIBSDL_VERSION)
LIBSDL_ZIP = $(LIBSDL_PKG).zip
LIBSDL_INSTALL = \
	@printf $(C_YELLOW)"WARNING"$(C_RESET)": Unsupported platform: SDL2 must be configured manually""\n"

ifeq ($(OSMODE),other)
else ifeq ($(OSMODE),win32)
	LIBSDL_ZIP = $(LIBSDL_PKG)-win32-x86.zip
	LIBSDL_INSTALL = \
		unzip $(LIBSDL_ZIP) -d $(LIBSDL_BIN)
	LIBSDL_GETVERSIONS = \
		grep '-win32-x86.zip'
else ifeq ($(OSMODE),win64)
	LIBSDL_ZIP = $(LIBSDL_PKG)-win32-x64.zip
	LIBSDL_INSTALL = \
		unzip $(LIBSDL_ZIP) -d $(LIBSDL_BIN)
	LIBSDL_GETVERSIONS = \
		grep '-win32-x64.zip'
else ifeq ($(OSMODE),macos)
	LIBSDL_ZIP = $(LIBSDL_PKG).dmg
	LIBSDL_INSTALL = \
		listing=`hdiutil attach $(LIBSDL_ZIP) | grep Volumes` ; \
		volume=`echo "$$listing" | cut -f 3` ; \
		cp -rf "$$volume"/SDL2.framework $(LIBSDL_BIN) ; \
		hdiutil detach `echo "$$listing" | cut -f 1`
	LIBSDL_GETVERSIONS = \
		grep '.dmg'
else ifeq ($(OSMODE),linux)
	LIBSDL_ZIP = $(LIBSDL_PKG).zip
	LIBSDL_INSTALL = \
		unzip $(LIBSDL_ZIP) -d $(LIBSDL_BIN) ; \
		cd $(LIBSDL_BIN) ; \
		./configure && make && make install
	LIBSDL_GETVERSIONS = \
		grep '.zip'
else
$(error Unsupported platform: external package 'SDL2' must be configured manually)
endif



.PHONY:\
version-SDL2 # sets the desired version of the package
version-SDL2:
	@cp -f $(LIBSDL_DIR)VERSION $(LIBSDL_VERFILE)



.PHONY:\
package-SDL2 # prepares the package for building
package-SDL2:
	@printf $(C_MAGENTA)"Preparing package: $(LIBSDL)..."$(C_RESET)"\n"
	@mkdir -p $(LIBSDL_BIN)
	@curl $(LIBSDL_URL)$(LIBSDL_ZIP) --progress-bar --output $(LIBSDL_ZIP)
	@$(LIBSDL_INSTALL)
	@rm -f $(LIBSDL_ZIP)
	@printf $(C_GREEN)"SUCCESS!"$(C_RESET)"\n"



.PHONY:\
update-SDL2 # updates the package to the latest version
update-SDL2:
	@printf $(C_MAGENTA)"Checking new versions for package: $(LIBSDL)..."$(C_RESET)"\n"
	@echo "=> Current version is: $(LIBSDL_VERSION)"
	@curl --silent $(LIBSDL_URL) \
	| grep 'SDL2' \
	| cut -d'"' -f 8 \
	| $(LIBSDL_GETVERSIONS) \
	| sort --version-sort \
	| awk -v found=0 '{ if (/$(LIBSDL_VERSION)/) { found = 1 } else if (found) { print } }'
	@#$(MAKE) version-SDL2
	@#$(MAKE) package-SDL2
