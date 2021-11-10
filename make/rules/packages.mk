#! This file holds rules to build dependencies (libraries, packages, binaries, etc)



#! The file which stores all dependency package version info
PACKAGESFILE = make/lists/packages.txt
# if file doesn't exist, create it
ifeq ($(shell test -f $(PACKAGESFILE) ; echo $$?),1)
$(warning NOTE: packages list file '$(PACKAGESFILE)' doesn't exist - creating now...)
$(foreach i,$(PACKAGES), $(shell echo "$(i)@0.0.0-?" >> $(PACKAGESFILE)))
endif



.PHONY:\
packages # This rule builds/prepares the dependency external packages
packages: $(addprefix package-, $(PACKAGES))



.PHONY:\
update-all # This rule update the dependency external packages to their latest versions
update-all: $(addprefix update-, $(PACKAGES))



# include makefiles for each external package
include $(foreach i,$(PACKAGES), make/packages/$(i).mk)
