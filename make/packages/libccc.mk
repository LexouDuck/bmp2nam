#! External package: libccc



LIBCCC = libccc
LIBCCC_DIR = $(LIBDIR)$(LIBCCC)/
LIBCCC_BIN = $(LIBCCC_DIR)bin/$(OSMODE)/$(LIBMODE)/
LIBCCC_VERSION := $(shell $(call packages_getversion,$(LIBCCC)))

LIBCCC_INCLUDE = $(LIBCCC_DIR)hdr/
LIBCCC_LINKDIR = $(LIBCCC_BIN)
LIBCCC_LINKLIB = -lccc
LIBCCC_LINK = -L$(LIBCCC_LINKDIR) $(LIBCCC_LINKLIB)



LIBCCC_URL = https://github.com/LexouDuck/libccc
LIBCCC_URL_VERSION = https://raw.githubusercontent.com/LexouDuck/libccc/master/VERSION



.PHONY:\
package-libccc # prepares the package for building
package-libccc:
	@$(call packages_setversion,$(LIBCCC),$(LIBCCC_VERSION))
	@printf $(C_CYAN)"Downloading package: $(LIBCCC)@$(LIBCCC_VERSION)..."$(C_RESET)"\n"
	@git submodule update --init $(LIBCCC_DIR)
	@printf $(C_CYAN)"Building package: $(LIBCCC)..."$(C_RESET)"\n"
	@$(MAKE) -C $(LIBCCC_DIR) build-$(MODE)
	@printf $(C_GREEN)"SUCCESS!"$(C_RESET)"\n"



.PHONY:\
update-libccc # updates the package to the latest version
update-libccc:
	@printf $(C_CYAN)"Checking new versions for package: $(LIBCCC)..."$(C_RESET)"\n"
	@echo "=> Current version is: $(LIBCCC_VERSION)"
	@cd $(LIBCCC_DIR) ; \
	if git status | grep -q "HEAD detached" ; then \
		printf $(C_YELLOW)"WARNING"$(C_RESET)": Your git submodule "$$i" is in detached HEAD state""\n" ; \
		printf "You need to manually go into the submodule folder and do 'git checkout master',""\n" ; \
		printf "after making sure that you have no uncommitted/unpushed local working changes.""\n" ; \
		exit 1 ; \
	fi ; \
	newer_version=`curl $(LIBCCC_URL_VERSION) | cut -d'@' -f 2 | cut -d'-' -f 1` ; \
	printf "Newest version is '$${newer_version}'.\n" ; \
	git pull ; \
	cd - ; \
	$(MAKE) package-libccc LIBCCC_VERSION=$$newer_version
