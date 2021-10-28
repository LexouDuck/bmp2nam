#! This file holds C-specific rules to build a library



#! List of all C header code files
HDRS := $(shell cat $(HDRSFILE))

#! List of all C source code files
SRCS := $(shell cat $(SRCSFILE))

#! Derive list of compiled object files (.o) from list of srcs
OBJS = ${SRCS:%.c=$(OBJDIR)%.o}

#! Derive list of dependency files (.d) from list of srcs
DEPS = ${OBJS:.o=.d}

#! List of libraries to link against
LDLIBS = $(foreach i,$(PACKAGES_LINK),$($(i)))

#! List of folders which store header code files
INCLUDES = -I$(HDRDIR) $(foreach i,$(PACKAGES_INCLUDE),-I$($(i)))



.PHONY:\
lists # Create/update the list of source/header files
lists:
	@find $(HDRDIR) -name "*.h" | sort | sed "s|$(HDRDIR)/||g" > $(HDRSFILE)
	@find $(SRCDIR) -name "*.c" | sort | sed "s|$(SRCDIR)/||g" > $(SRCSFILE)



.PHONY:\
build-debug # Builds the library, in 'debug' mode (with debug flags and symbol-info)
build-debug: MODE = debug
build-debug: CFLAGS += $(CFLAGS_DEBUG)
build-debug: $(NAME)

.PHONY:\
build-release # Builds the library, in 'release' mode (with debug flags and symbol-info)
build-release: MODE = release
build-release: CFLAGS += $(CFLAGS_RELEASE)
build-release: $(NAME)



#! Compiles object files from source files
$(OBJDIR)%.o : $(SRCDIR)%.c
	@mkdir -p $(@D)
	@printf "Compiling file: "$@" -> "
	@$(CC) -o $@ $(CFLAGS) $(INCLUDES) -MMD -c $<
	@printf $(C_GREEN)"OK!"$(C_RESET)"\n"



#! Compiles the project executable
$(NAME): $(OBJS)
	@printf "Compiling program: "$(NAME)" -> "
	@$(CC) -o $@ $(CFLAGS) $(INCLUDES) $^ $(LDLIBS)
	@printf $(C_GREEN)"OK!"$(C_RESET)"\n"



# The following line is for `.d` dependency file handling
-include ${DEPS}
