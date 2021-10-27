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



.PHONY:\
package-SDL2 # prepares the package for building
package-SDL2:
	@mkdir -p $(LIBSDL_BIN)
ifeq ($(OSMODE),other)
	@printf $(C_YELLOW)"WARNING"$(C_RESET)": Unsupported platform: SDL2 must be configured manually""\n"
else ifeq ($(OSMODE),win32)
	@wget https://www.libsdl.org/release/SDL2-$(LIBSDL_VERSION)-win32-x86.zip
	@zip -x                              SDL2-$(LIBSDL_VERSION)-win32-x86.zip
else ifeq ($(OSMODE),win64)
	@wget https://www.libsdl.org/release/SDL2-$(LIBSDL_VERSION)-win32-x64.zip
	@zip -x                              SDL2-$(LIBSDL_VERSION)-win32-x64.zip
else ifeq ($(OSMODE),macos)
	@wget https://www.libsdl.org/release/SDL2-$(LIBSDL_VERSION).dmg ; \
	listing=`hdiutil attach SDL2-$(LIBSDL_VERSION).dmg | grep Volumes` ; \
	volume=`echo "$$listing" | cut -f 3` ; \
	cp -rf "$$volume"/SDL2.framework $(LIBSDL_BIN) ; \
	hdiutil detach `echo "$$listing" | cut -f 1` ; \
	rm -f SDL2-$(LIBSDL_VERSION).dmg
else ifeq ($(OSMODE),linux)
	@wget https://www.libsdl.org/release/SDL2-$(LIBSDL_VERSION).zip
	@zip -x                              SDL2-$(LIBSDL_VERSION).zip
	# TODO build from source code
endif



.PHONY:\
update-SDL2 # updates the package to the latest version
update-SDL2:
	@cd $(LIBDIR)libccc ; \
	if git status | grep -q "HEAD detached" ; then \
		printf $(C_YELLOW)"WARNING"$(C_RESET)": Your git submodule "$$i" is in detached HEAD state""\n" ; \
		printf "You need to manually go into the submodule folder and do 'git checkout master',""\n" ; \
		printf "after making sure that you have no uncommitted/unpushed local working changes.""\n" ; \
	else git pull ; \
	fi ; \
	cd -
