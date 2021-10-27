#! This file holds C-specific rules to build a library



define update_list_hdrs
	find $(HDRDIR) -name "*.h" | sort | sed "s|$(HDRDIR)/||g" > $(HDRSFILE)
endef
ifeq ($(shell test -f $(HDRSFILE) && echo _),)
$(warning NOTE: header list file "$(HDRSFILE)" doesn't exist, creating it...)
$(shell $(call update_list_hdrs))
endif

define update_list_srcs
	find $(SRCDIR) -name "*.c" | sort | sed "s|$(SRCDIR)/||g" > $(SRCSFILE)
endef
ifeq ($(shell test -f $(SRCSFILE) && echo _),)
$(warning NOTE: source list file "$(SRCSFILE)" doesn't exist, creating it...)
$(shell $(call update_list_srcs))
endif



#! List of all C header code files
HDRS := $(shell cat $(HDRSFILE))

#! List of all C source code files
SRCS := $(shell cat $(SRCSFILE))

#! Derive list of compiled object files (.o) from list of srcs
OBJS = ${SRCS:%.c=$(OBJDIR)%.o}

#! Derive list of dependency files (.d) from list of srcs
DEPS = ${OBJS:.o=.d}



.PHONY:\
update-lists-build # Create/update the list of source/header files
update-lists-build:
	@$(call update_list_hdrs)
	@$(call update_list_srcs)



#! Compiles object files from source files
$(OBJDIR)%.o : $(SRCDIR)%.c
	@mkdir -p $(@D)
	@printf "Compiling file: "$@" -> "
	@$(CC) -o $@ $(CFLAGS) -MMD -I$(HDRDIR) -c $<
	@printf $(C_GREEN)"OK!"$(C_RESET)"\n"



#! Compiles the project executable
$(NAME): $(OBJS)
	@printf "Compiling program: "$(NAME)" -> "
	@$(CC) $(CFLAGS) -o $@ $^ $(INCLUDE_DIRS) $(LIBS)
	@printf $(GREEN)"OK!"$(RESET)"\n"



# The following line is for `.d` dependency file handling
-include ${DEPS}