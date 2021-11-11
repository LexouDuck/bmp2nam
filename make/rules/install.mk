#! This file holds rules to install/uninstall a C program in the appropriate location



.PHONY:\
install #! Installs the libraries/programs (copies them from `./bin/` to `/usr/local/`, typically)
install: $(NAME)
	@printf $(IO_CYAN)"Installing library: $(NAME)..."$(IO_RESET)"\n"
	@mkdir -p $(INSTALLDIR)/bin/
	@$(INSTALL_PROGRAM) $(NAME) $(INSTALLDIR)/bin/$(NAME)
	@printf $(IO_GREEN)"SUCCESS!"$(IO_RESET)"\n"



.PHONY:\
uninstall #! Removes the installed libraries/programs (deletes files in `/usr/local/`, typically)
uninstall:
	@printf $(IO_CYAN)"Uninstalling library: $(NAME)..."$(IO_RESET)"\n"
	@rm -f $(INSTALLDIR)/bin/$(NAME)
	@printf $(IO_GREEN)"SUCCESS!"$(IO_RESET)"\n"
