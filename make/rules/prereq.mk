#! This file holds project-specific rules to check whether prerequisite programs are correctly installed/available



check_prereq = \
	printf "\n\n"$(C_CYAN)"|=> Checking prerequisite: $(1)"$(C_RESET)"\n$$ $(2)\n" ; \
	{ $(2) ; } || \
	{ \
		printf $(C_YELLOW)"$(1) is not installed"$(C_RESET)"\n" ; \
		printf $(C_MAGENTA)"Installing...\n" ; \
		$(3) ; \
	}



ifeq ($(OS),Windows_NT)
install_prereq = \
	printf $(C_RED)"ERROR"$(C_RESET)": Windows platform detected. You must manually install: " \

else ifeq ($(UNAME_S),Darwin)
install_prereq = \
	brew install $(1) \

else ifeq ($(UNAME_S),Linux)
install_prereq = \
	if   [ -x "$(command -v apk)"     ]; then $(SUDO) apk add --no-cache $(1) ; \
	elif [ -x "$(command -v apt-get)" ]; then $(SUDO) apt-get install    $(1) ; \
	elif [ -x "$(command -v yum)"     ]; then $(SUDO) yum     install    $(1) ; \
	elif [ -x "$(command -v pacman)"  ]; then $(SUDO) pacman  -S         $(1) ; \
	elif [ -x "$(command -v dnf)"     ]; then $(SUDO) dnf     install    $(1) ; \
	elif [ -x "$(command -v zypp)"    ]; then $(SUDO) zypp    install    $(1) ; \
	elif [ -x "$(command -v zypper)"  ]; then $(SUDO) zypper  install    $(1) ; \
	else \
		printf $(C_RED)"ERROR"$(C_RESET)": Package manager not found. You must manually install: $(1)" >&2 ; \
	fi \

else
install_prereq = \
	printf $(C_RED)"ERROR"$(C_RESET)": Unknown platform. You must manually install: $(1)"
endif





.PHONY:\
prereq # Checks version numbers for all prerequisite tools
prereq: init \
prereq-build \
prereq-tests \
prereq-format \
prereq-lint
	@printf $(C_RESET)"\n\n"



.PHONY:\
prereq-build # Checks prerequisite installs to build the library/program
prereq-build:
	@-$(call check_prereq,\
		(build) compiler: $(CC),\
		$(CC) --version,\
		$(call install_prereq,$(CC)))
	@-$(call check_prereq,\
		(build) archiver: $(AR),\
		which $(AR),\
		$(call install_prereq,binutils))

.PHONY:\
prereq-tests # Checks prerequisite installs to run the various tests
prereq-tests:
ifeq ($(OSMODE),other)
	@printf $(C_YELLOW)"WARNING"$(C_RESET)": Unsupported platform: memory leak checking tool must be configured manually""\n"
else ifeq ($(OSMODE),win32)
	@-# TODO drmemory.exe ?
else ifeq ($(OSMODE),win64)
	@-# TODO drmemory.exe ?
else ifeq ($(OSMODE),macos)
	@-$(call check_prereq,\
		(tests) Xcode leaks checker,\
		leaks --help,\
		$(call install_prereq,leaks))
else ifeq ($(OSMODE),linux)
	@-$(call check_prereq,\
		(tests) Valgrind,\
		valgrind --version,\
		$(call install_prereq,valgrind))
else
	@printf $(C_YELLOW)"WARNING"$(C_RESET)": Unsupported platform: memory leak checking tool must be configured manually""\n"
endif

.PHONY:\
prereq-format # Checks prerequisite installs to run the automatic code style formatter
prereq-format:
	@-$(call check_prereq,\
		(format) indent,\
		which indent,\
		$(call install_prereq,indent))

.PHONY:\
prereq-lint # Checks prerequisite installs to run the linter/static analyzer
prereq-lint:
	@-$(call check_prereq,\
		(lint) cppcheck,\
		cppcheck --version,\
		$(call install_prereq,cppcheck))
	@-$(call check_prereq,\
		(lint) splint,\
		splint --help,\
		$(call install_prereq,splint))
