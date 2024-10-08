#! This file stores the main/default targets of the build system



.PHONY:\
all #! Builds all targets (this is the default rule)
all:
	$(foreach i,$(BUILDMODES),	@$(MAKE) BUILDMODE=$(i) build)



.PHONY:\
re #! Deletes all generated files and rebuilds `all`
re: \
clean \
all \



.PHONY:\
setup #! Performs initial setup steps for the project
setup: \
init \
version \
prereq \
packages \
