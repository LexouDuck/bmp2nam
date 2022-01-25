#! This file holds C-specific rules to debug the code in various ways



#! The list of output files for 'debug-macros' - essentially just the list of sources, but in the ./obj folder
SRCS_PREPROCESSED = $(SRCS:%.c=$(OBJDIR)%.c)

#! This rule runs the preprocessing step for each .c file, and outputs to obj
$(OBJDIR)%.c: $(SRCDIR)%.c
	@printf "Preprocessing file: "$@" -> "
	@$(CC) -o $@ $(CFLAGS) -E $<
	@printf $(IO_GREEN)"OK!"$(IO_RESET)"\n"

.PHONY:\
debug-macros #! Preprocesses all source files and stores them in the obj folder
debug-macros: all $(SRCS_PREPROCESSED)
	@$(call print_message,"Outputting preprocessed code...")



.PHONY:\
debug-symbols #! Outputs the list of symbols found inside the given `ARGS` binary/ies
debug-symbols: all
ifeq ($(ARGS),)
	@$(call print_error,"This rule expects one or more binary files given as arguments (ARGS=...)")
else
	@nm -a $(ARGS)
endif
	


.PHONY:\
debug-linking #! Outputs the list of linking paths to find dynamic libraries for the given `ARGS`
debug-linking: all
ifeq ($(ARGS),)
	@$(call print_error,"This rule expects one or more binary files given as arguments (ARGS=...)")
else ifeq ($(OSMODE),win32)
	@objdump -p $(ARGS)
else ifeq ($(OSMODE),win64)
	@objdump -p $(ARGS)
else ifeq ($(OSMODE),macos)
	@otool -L $(ARGS)
else ifeq ($(OSMODE),linux)
	@ldd $(ARGS)
else
	@$(call print_error,"Unknown platform, needs manual configuration")
endif



# TODO: rules for leak-checking (valgrind, dr.memory, etc)