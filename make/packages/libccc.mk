#! External package: libccc



libccc = libccc
libccc_VERSION := $(shell $(call packages_getversion,$(libccc)))
libccc_DIR = $(LIBDIR)libccc/
libccc_BIN = $(libccc_DIR)bin/$(OSMODE)/$(LIBMODE)/
libccc_INCLUDE = $(libccc_DIR)hdr/
libccc_LINKDIR = $(libccc_BIN)
libccc_LINKLIB = -lccc
libccc_LINK = -L$(libccc_LINKDIR) $(libccc_LINKLIB)



libccc_URL = https://github.com/LexouDuck/libccc
libccc_URL_VERSION = https://raw.githubusercontent.com/LexouDuck/libccc/master/VERSION

libccc_GITBRANCH = dev



#! The shell command to retrieve and output list of newer versions, if any
package_libccc_checkupdates = \
	curl --silent $(libccc_URL_VERSION) \
	| cut -d'@' -f 2 \
	| cut -d'-' -f 1 \



.PHONY:\
package-libccc #! prepares the package for building
package-libccc:
	@$(call packages_setversion,$(libccc),$(libccc_VERSION))
	@printf $(IO_CYAN)"Downloading package: $(libccc)@$(libccc_VERSION)..."$(IO_RESET)"\n"
	@git submodule update --init $(libccc_DIR)
	@printf $(IO_CYAN)"Building package: $(libccc)..."$(IO_RESET)"\n"
	@$(MAKE) -C $(libccc_DIR) build-$(MODE)
	@printf $(IO_GREEN)"SUCCESS!"$(IO_RESET)"\n"



.PHONY:\
update-libccc #! updates the package to the latest version
update-libccc:
	@printf $(IO_CYAN)"Checking new versions for package: $(libccc)..."$(IO_RESET)"\n"
	@echo "=> Current version is: $(libccc_VERSION)"
	@cd $(libccc_DIR) ; \
	if git status | grep -q "HEAD detached" ; then \
		printf $(IO_YELLOW)"WARNING"$(IO_RESET)": Your git submodule "$$i" is in detached HEAD state.\n" ; \
		printf "You need to manually go into the submodule folder and do 'git checkout $(libccc_GITBRANCH)',\n" ; \
		printf "after making sure that you have no uncommitted/unpushed local working changes.\n" ; \
	else \
		newer_version=`$(call package_libccc_checkupdates)` ; \
		printf "Newest version is '$${newer_version}'.\n" ; \
		git fetch ; \
		git checkout $(libccc_GITBRANCH) ; \
		cd - ; \
		$(MAKE) package-libccc libccc_VERSION=$$newer_version ; \
	fi
