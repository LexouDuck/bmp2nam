#! External package: libccc



PACKAGE_libccc = libccc
PACKAGE_libccc_VERSION := $(shell $(call packages_getversion,$(PACKAGE_libccc)))
PACKAGE_libccc_DIR = $(LIBDIR)libccc/
PACKAGE_libccc_BIN = $(PACKAGE_libccc_DIR)bin/$(OSMODE)/$(LIBMODE)/
PACKAGE_libccc_INCLUDE = $(PACKAGE_libccc_DIR)hdr/
PACKAGE_libccc_LINKDIR = $(PACKAGE_libccc_BIN)
PACKAGE_libccc_LINKLIB = -lccc
PACKAGE_libccc_LINK = -L$(PACKAGE_libccc_LINKDIR) $(PACKAGE_libccc_LINKLIB)



PACKAGE_libccc_URL = https://github.com/LexouDuck/libccc
PACKAGE_libccc_URL_VERSION = https://raw.githubusercontent.com/LexouDuck/libccc/master/VERSION

PACKAGE_libccc_GITBRANCH = dev



#! The shell command to retrieve and output list of newer versions, if any
package_libccc_checkupdates = \
	curl --silent $(PACKAGE_libccc_URL_VERSION) \
	| cut -d'@' -f 2 \
	| cut -d'-' -f 1 \



.PHONY:\
package-libccc #! prepares the package for building
package-libccc:
	@$(call packages_setversion,$(PACKAGE_libccc),$(PACKAGE_libccc_VERSION))
	@printf $(IO_CYAN)"Downloading package: $(PACKAGE_libccc)@$(PACKAGE_libccc_VERSION)..."$(IO_RESET)"\n"
	@git submodule update --init $(PACKAGE_libccc_DIR)
	@printf $(IO_CYAN)"Building package: $(PACKAGE_libccc)..."$(IO_RESET)"\n"
	@$(MAKE) -C $(PACKAGE_libccc_DIR) build-$(MODE)
	@printf $(IO_GREEN)"SUCCESS!"$(IO_RESET)"\n"



.PHONY:\
update-libccc #! updates the package to the latest version
update-libccc:
	@printf $(IO_CYAN)"Checking new versions for package: $(PACKAGE_libccc)..."$(IO_RESET)"\n"
	@echo "=> Current version is: $(PACKAGE_libccc_VERSION)"
	@cd $(PACKAGE_libccc_DIR) ; \
	if git status | grep -q "HEAD detached" ; then \
		printf $(IO_YELLOW)"WARNING"$(IO_RESET)": Your git submodule "$$i" is in detached HEAD state.\n" ; \
		printf "You need to manually go into the submodule folder and do 'git checkout $(PACKAGE_libccc_GITBRANCH)',\n" ; \
		printf "after making sure that you have no uncommitted/unpushed local working changes.\n" ; \
	else \
		newer_version=`$(call package_libccc_checkupdates)` ; \
		printf "Newest version is '$${newer_version}'.\n" ; \
		git fetch ; \
		git checkout $(PACKAGE_libccc_GITBRANCH) ; \
		cd - ; \
		$(MAKE) package-libccc PACKAGE_libccc_VERSION=$$newer_version ; \
	fi
