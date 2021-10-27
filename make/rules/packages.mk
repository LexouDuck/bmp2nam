#! This file holds rules to build dependencies (libraries, packages, binaries, etc)



.PHONY:\
packages # This rule builds/prepares the dependency external packages
packages: \
packages-libccc \



.PHONY:\
packages-libccc
packages-libccc:
	@$(MAKE) -C $(LIBDIR)libccc $(MODE) \
		CC=$(CC) \
		OSMODE=$(OSMODE) \
		LIBMODE=$(LIBMODE) \
