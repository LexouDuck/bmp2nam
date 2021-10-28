#! This file holds rules to install/uninstall a C program in the appropriate location



#! DESTDIR is a GNU-conventional environment variable, typically set to nothing (ie: root folder)
ifeq ($(DESTDIR),)
	DESTDIR := 
endif
#! PREFIX is a GNU-conventional environment variable, but if it is not set, then set default value
ifeq ($(PREFIX),)
	PREFIX := /usr/local
endif
#! The folder in which to install the library
INSTALLDIR = $(DESTDIR)$(PREFIX)



.PHONY:\
install # Installs the program(s) (copies them from `./bin/` to `/usr/local/bin`, typically)
install:
	@echo "Installing program: $(NAME)..."
	@mkdir -p $(INSTALLDIR)/bin/
	@install -m 755 $(NAME) $(INSTALLDIR)/bin/$(NAME)
	@echo "done"



.PHONY:\
uninstall # Removes the installed libraries/programs (deletes files in `/usr/local/`, typically)
uninstall:
	@echo "Uninstalling program: $(NAME)..."
	@rm -f $(INSTALLDIR)/bin/$(NAME)
	@echo "done"
