#! This file holds rules to clean up the project folder, deleting generated files, etc



.PHONY:\
clean # Deletes any files built by the 'all' rule
clean:
	@printf $(C_CYAN)"Deleting all .o files..."$(C_RESET)"\n"
	@rm -f $(OBJS)
	@rm -f $(OBJS_TEST)
	@printf $(C_CYAN)"Deleting all .d files..."$(C_RESET)"\n"
	@rm -f $(DEPS)
	@rm -f $(DEPS_TEST)
	@rm -f *.d
	@printf $(C_CYAN)"Deleting program: "$(NAME)""$(C_RESET)"\n"
	@rm -f $(NAME)
	@rm -f $(NAME).*



.PHONY:\
clean-obj # Deletes the ./obj folder
clean-obj:
	@printf $(C_CYAN)"Deleting "$(OBJDIR)" folder..."$(C_RESET)"\n"
	@rm -rf $(OBJDIR)

.PHONY:\
clean-bin # Deletes the ./bin folder
clean-bin:
	@printf $(C_CYAN)"Deleting "$(BINDIR)" folder..."$(C_RESET)"\n"
	@rm -rf $(BINDIR)

.PHONY:\
clean-logs # Deletes the ./log folder
clean-logs:
	@printf $(C_CYAN)"Deleting "$(LOGDIR)" folder..."$(C_RESET)"\n"
	@rm -rf $(LOGDIR)

.PHONY:\
clean-lint # Deletes the ./lint folder
clean-lint:
	@printf $(C_CYAN)"Deleting "$(LINTDIR)" folder..."$(C_RESET)"\n"
	@rm -rf $(LINTDIR)



.PHONY:\
clean-full # Deletes every generated file
clean-full: clean clean-exe
