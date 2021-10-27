#! This file holds rules to install/uninstall a C library in the appropriate location



#! The destination folder in which to install the program
INSTALL_DIR = /usr/local/bin/



.PHONY:\
install # Installs the program(s) (copies them from `./bin/` to `/usr/local/bin`, typically)
install:
	@cp $(BINDIR)$(OSMODE)/$(NAME) \
			$(INSTALL_DIR)$(NAME).$(VERSION)$(EXE)
	@ln -s	$(INSTALL_DIR)$(NAME).$(VERSION)$(EXE) \
			$(INSTALL_DIR)$(NAME)$(EXE)



.PHONY:\
uninstall # Removes the installed libraries/programs (deletes files in `/usr/local/`, typically)
uninstall:
	@printf "Removing the following files:\n"
	@find $(INSTALL_DIR) -name "$(NAME).*" -print -delete
