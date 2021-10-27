#! This file holds rules to clean up the project folder, deleting generated files, etc



.PHONY:\
clean # Deletes any files built by the 'all' rule
clean:
	@printf "Deleting all .o files...\n"
	@rm -f $(OBJS)
	@rm -f $(OBJS_TEST)
	@printf "Deleting all .d files...\n"
	@rm -f $(DEPS)
	@rm -f $(DEPS_TEST)
	@rm -f *.d
	@printf "Deleting program: "$(NAME)"\n"
	@rm -f $(NAME)
	@rm -f $(NAME).*



.PHONY:\
clean-obj # Deletes the ./obj folder
clean-obj:
	@printf "Deleting "$(OBJDIR)" folder...\n"
	@rm -rf $(OBJDIR)

.PHONY:\
clean-bin # Deletes the ./bin folder
clean-bin:
	@printf "Deleting "$(BINDIR)" folder...\n"
	@rm -rf $(BINDIR)

.PHONY:\
clean-logs # Deletes the ./log folder
clean-logs:
	@printf "Deleting "$(LOGDIR)" folder...\n"
	@rm -rf $(LOGDIR)

.PHONY:\
clean-lint # Deletes the ./lint folder
clean-lint:
	@printf "Deleting "$(LINTDIR)" folder...\n"
	@rm -rf $(LINTDIR)



.PHONY:\
clean-full # Deletes every generated file
clean-full: clean clean-exe
