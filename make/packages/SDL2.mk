#! External package: SDL2



LIBSDL = SDL2
LIBSDL_VERSION = 2.0.16
LIBCCC_VERFILE = $(LIBDIR)$(LIBSDL).ver
LIBSDL_DIR = $(LIBDIR)$(LIBSDL)/
LIBSDL_BIN = $(LIBSDL_DIR)bin/$(OSMODE)/

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
LIBSDL_ZIP = SDL2-$(LIBSDL_VERSION)
ifeq ($(OSMODE),other)
else ifeq ($(OSMODE),win32)
	LIBSDL_ZIP = $(LIBSDL_PKG)-win32-x86.zip
else ifeq ($(OSMODE),win64)
	LIBSDL_ZIP = $(LIBSDL_PKG)-win32-x64.zip
else ifeq ($(OSMODE),macos)
	LIBSDL_ZIP = $(LIBSDL_PKG).dmg
else ifeq ($(OSMODE),linux)
	LIBSDL_ZIP = $(LIBSDL_PKG).zip
else
$(error Unsupported platform: external package 'SDL2' must be configured manually)
endif



.PHONY:\
package-SDL2 # prepares the package for building
package-SDL2:
	@mkdir -p $(LIBSDL_BIN)
ifeq ($(OSMODE),other)
	@printf $(C_YELLOW)"WARNING"$(C_RESET)": Unsupported platform: SDL2 must be configured manually""\n"
else ifeq ($(OSMODE),win32)
	@wget $(LIBSDL_URL)$(LIBSDL_PKG)-win32-x86.zip
	@zip -x            $(LIBSDL_PKG)-win32-x86.zip
	# TODO move to proper location
	@rm -f $(LIBSDL_PKG)-win32-x86.zip
else ifeq ($(OSMODE),win64)
	@wget $(LIBSDL_URL)$(LIBSDL_PKG)-win32-x64.zip
	@zip -x            $(LIBSDL_PKG)-win32-x64.zip
	# TODO move to proper location
	@rm -f $(LIBSDL_PKG).dmg
else ifeq ($(OSMODE),macos)
	@wget $(LIBSDL_URL)$(LIBSDL_PKG).dmg ; \
	listing=`hdiutil attach SDL2-$(LIBSDL_VERSION).dmg | grep Volumes` ; \
	volume=`echo "$$listing" | cut -f 3` ; \
	cp -rf "$$volume"/SDL2.framework $(LIBSDL_BIN) ; \
	hdiutil detach `echo "$$listing" | cut -f 1`
	@rm -f $(LIBSDL_PKG).dmg
else ifeq ($(OSMODE),linux)
	@wget $(LIBSDL_URL)$(LIBSDL_PKG).zip
	@zip -x            $(LIBSDL_PKG).zip
	# TODO build from source code
	@rm -f $(LIBSDL_PKG).zip
endif



.PHONY:\
update-SDL2 # updates the package to the latest version
update-SDL2:
	@wget ; \
	@#$(MAKE) package-SDL2
