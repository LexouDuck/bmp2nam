#! This file holds rules to install/uninstall a C program in the appropriate location



.PHONY:\
install # Installs the program(s) (copies them from `./bin/` to `/usr/local/bin`, typically)
install:
	@echo "Installing program: $(NAME)..."
	@mkdir -p $(INSTALLDIR)/bin/
	@$(INSTALL_PROGRAM) $(NAME) $(INSTALLDIR)/bin/$(NAME)
	@echo "done"



.PHONY:\
uninstall # Removes the installed libraries/programs (deletes files in `/usr/local/`, typically)
uninstall:
	@echo "Uninstalling program: $(NAME)..."
	@rm -f $(INSTALLDIR)/bin/$(NAME)
	@echo "done"
