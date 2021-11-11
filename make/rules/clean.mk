#! This file holds rules to clean up the project folder, deleting generated files, etc



.PHONY:\
clean #! Deletes all intermediary build files
clean:
	@printf $(IO_CYAN)"Deleting all .o files..."$(IO_RESET)"\n"
	@rm -f $(OBJS)
	@printf $(IO_CYAN)"Deleting all .d files..."$(IO_RESET)"\n"
	@rm -f $(DEPS)
	@rm -f *.d



.PHONY:\
clean-exe #! Deletes any libraries/executables
clean-exe:
	@printf $(IO_CYAN)"Deleting program: "$(NAME)""$(IO_RESET)"\n"
	@rm -f $(NAME)
	@rm -f $(NAME).*

.PHONY:\
clean-obj #! Deletes the ./obj folder
clean-obj:
	@printf $(IO_CYAN)"Deleting "$(OBJDIR)" folder..."$(IO_RESET)"\n"
	@rm -rf $(OBJDIR)

.PHONY:\
clean-bin #! Deletes the ./bin folder
clean-bin:
	@printf $(IO_CYAN)"Deleting "$(BINDIR)" folder..."$(IO_RESET)"\n"
	@rm -rf $(BINDIR)

.PHONY:\
clean-logs #! Deletes the ./log folder
clean-logs:
	@printf $(IO_CYAN)"Deleting "$(LOGDIR)" folder..."$(IO_RESET)"\n"
	@rm -rf $(LOGDIR)

.PHONY:\
clean-lint #! Deletes the ./lint folder
clean-lint:
	@printf $(IO_CYAN)"Deleting "$(LINTDIR)" folder..."$(IO_RESET)"\n"
	@rm -rf $(LINTDIR)



.PHONY:\
clean-full #! Deletes every generated file
clean-full: clean clean-exe
