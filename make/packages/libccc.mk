#! External package: libccc



LIBCCC = libccc
LIBCCC_DIR = $(LIBDIR)$(LIBCCC)/
LIBCCC_BIN = $(LIBCCC_DIR)bin/$(OSMODE)/$(LIBMODE)/
LIBCCC_VERSION := $(shell cat $(PACKAGESLIST) | grep '^$(LIBCCC)' | cut -d'@' -f 2 | cut -d'-' -f 1)

LIBCCC_INCLUDE = $(LIBCCC_DIR)hdr/
LIBCCC_LINKDIR = $(LIBCCC_BIN)
LIBCCC_LINKLIB = -lccc
LIBCCC_LINK = -L$(LIBCCC_LINKDIR) $(LIBCCC_LINKLIB)



.PHONY:\
version-libccc # sets the desired version of the package
version-libccc:
	@cat $(PACKAGESLIST) \
	| awk '{ if (/^$(LIBCCC)@/) { print "$(LIBCCC)@" } else { print } }'



.PHONY:\
package-libccc # prepares the package for building
package-libccc:
	@printf $(C_MAGENTA)"Downloading package: $(LIBCCC)..."$(C_RESET)"\n"
	@git submodule update --init $(LIBCCC_DIR)
	@printf $(C_MAGENTA)"Building package: $(LIBCCC)..."$(C_RESET)"\n"
	@$(MAKE) -C $(LIBCCC_DIR) build-$(MODE)
	@printf $(C_GREEN)"SUCCESS!"$(C_RESET)"\n"



.PHONY:\
update-libccc # updates the package to the latest version
update-libccc:
	@printf $(C_MAGENTA)"Checking new versions for package: $(LIBCCC)..."$(C_RESET)"\n"
	@echo "=> Current version is: $(LIBCCC_VERSION)"
	@cd $(LIBCCC_DIR) ; \
	if git status | grep -q "HEAD detached" ; then \
		printf $(C_YELLOW)"WARNING"$(C_RESET)": Your git submodule "$$i" is in detached HEAD state""\n" ; \
		printf "You need to manually go into the submodule folder and do 'git checkout master',""\n" ; \
		printf "after making sure that you have no uncommitted/unpushed local working changes.""\n" ; \
	else \
		git pull ; \
	fi ; \
	cd -
	@$(MAKE) version-libccc
	@$(MAKE) package-libccc
