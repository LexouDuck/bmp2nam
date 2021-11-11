#! This file holds rules to install/uninstall a C program in the appropriate location



.PHONY:\
install #! Installs the libraries/programs (copies them from `./bin/` to `/usr/local/`, typically)
install: $(NAME)
	@printf $(C_CYAN)"Installing library: $(NAME)..."$(C_RESET)"\n"
	@mkdir -p $(INSTALLDIR)/bin/
	@$(INSTALL_PROGRAM) $(NAME) $(INSTALLDIR)/bin/$(NAME)
	@printf $(C_GREEN)"SUCCESS!"$(C_RESET)"\n"



.PHONY:\
uninstall #! Removes the installed libraries/programs (deletes files in `/usr/local/`, typically)
uninstall:
	@printf $(C_CYAN)"Uninstalling library: $(NAME)..."$(C_RESET)"\n"
	@rm -f $(INSTALLDIR)/bin/$(NAME)
	@printf $(C_GREEN)"SUCCESS!"$(C_RESET)"\n"
