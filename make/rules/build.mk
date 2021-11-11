#! This file holds C-specific rules to build a library



#! Path of the file which stores the list of header code files
HDRSFILE = make/lists/hdrs.txt
#! The shell command which generates the list of header code files
make_HDRSFILE = find $(HDRDIR) -name "*.h" | sort | sed "s|$(HDRDIR)/||g" > $(HDRSFILE)
# if file doesn't exist, create it
ifeq ($(shell test -f $(HDRSFILE) ; echo $$?),1)
$(warning NOTE: header code list file '$(HDRSFILE)' doesn't exist - creating now...)
$(shell $(call make_HDRSFILE))
endif
#! List of all C header code files
HDRS := $(shell cat $(HDRSFILE))



#! Path of the file which stores the list of source code files
SRCSFILE = make/lists/srcs.txt
#! The shell command which generates the list of source code files
make_SRCSFILE = find $(SRCDIR) -name "*.c" | sort | sed "s|$(SRCDIR)/||g" > $(SRCSFILE)
# if file doesn't exist, create it
ifeq ($(shell test -f $(SRCSFILE) ; echo $$?),1)
$(warning NOTE: source code list file '$(SRCSFILE)' doesn't exist - creating now...)
$(shell $(call make_SRCSFILE))
endif
#! List of all C source code files
SRCS := $(shell cat $(SRCSFILE))



#! Derive list of compiled object files (.o) from list of srcs
OBJS := $(SRCS:$(SRCDIR)%.c=$(OBJDIR)%.o)

#! Derive list of dependency files (.d) from list of srcs
DEPS := $(OBJS:.o=.d)

#! GNU conventional variable: List of libraries to link against
LDLIBS = $(foreach i,$(PACKAGES_LINK),$($(i)))

#! GNU conventional variable: List of folders which store header code files
INCLUDES = -I$(HDRDIR) $(foreach i,$(PACKAGES_INCLUDE),-I$($(i)))



.PHONY:\
lists #! Create/update the list of source/header files
lists:
	@$(call make_HDRSFILE)
	@$(call make_SRCSFILE)



.PHONY:\
build-debug #! Builds the library, in 'debug' mode (with debug flags and symbol-info)
build-debug: MODE = debug
build-debug: CFLAGS += $(CFLAGS_DEBUG)
build-debug: $(NAME)

.PHONY:\
build-release #! Builds the library, in 'release' mode (with debug flags and symbol-info)
build-release: MODE = release
build-release: CFLAGS += $(CFLAGS_RELEASE)
build-release: $(NAME)



#! Compiles object files from source files
$(OBJDIR)%.o : $(SRCDIR)%.c
	@mkdir -p $(@D)
	@printf "Compiling file: "$@" -> "
	@$(CC) -o $@ $(CFLAGS) -MMD $(INCLUDES) -c $<
	@printf $(IO_GREEN)"OK!"$(IO_RESET)"\n"



#! Compiles the project executable
$(NAME): $(OBJS)
	@printf "Compiling program: "$(NAME)" -> "
	@$(CC) -o $@ $(CFLAGS) $(INCLUDES) $^ $(LDLIBS)
	@printf $(IO_GREEN)"OK!"$(IO_RESET)"\n"



# The following line is for `.d` dependency file handling
-include $(DEPS)
