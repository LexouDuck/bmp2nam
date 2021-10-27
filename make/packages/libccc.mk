#! External package: libccc



LIBCCC = libccc
LIBCCC_VERSION = 0.8.3
LIBCCC_VERFILE = $(LIBDIR)$(LIBCCC).ver
LIBCCC_DIR = $(LIBDIR)$(LIBCCC)/
LIBCCC_BIN = $(LIBCCC_DIR)bin/$(OSMODE)/$(LIBMODE)/

LIBCCC_INCLUDE = $(LIBCCC_DIR)hdr/
LIBCCC_LINKDIR = $(LIBCCC_BIN)
LIBCCC_LINKLIB = -lccc
LIBCCC_LINK = -L$(LIBCCC_LINKDIR) $(LIBCCC_LINKLIB)



.PHONY:\
package-libccc # prepares the package for building
package-libccc:
	@$(MAKE) -C $(LIBCCC_DIR) $(MODE)



.PHONY:\
update-libccc # updates the package to the latest version
update-libccc:
	@cd $(LIBCCC_DIR) ; \
	if git status | grep -q "HEAD detached" ; then \
		printf $(C_YELLOW)"WARNING"$(C_RESET)": Your git submodule "$$i" is in detached HEAD state""\n" ; \
		printf "You need to manually go into the submodule folder and do 'git checkout master',""\n" ; \
		printf "after making sure that you have no uncommitted/unpushed local working changes.""\n" ; \
	else git pull ; \
	fi ; \
	cd -
