#! This file holds rules to clean up the project folder, deleting generated files, etc



.PHONY:\
clean #! Deletes files generated by the `make all` default rule
clean:
	$(foreach i,$(BUILDMODES),	@$(MAKE) BUILDMODE=$(i) clean-build $(C_NL))



.PHONY:\
clean-all #! Deletes every generated file/folder
clean-all: \
clean-obj \
clean-bin \
clean-log \
clean-dist \



.PHONY:\
clean-full #! Deletes every generated file/folder (even files checked in to the repo !!!)
clean-full: \
clean-obj \
clean-bin \
clean-log \
clean-dist \



.PHONY:\
clean-obj #! Deletes the ./obj folder
clean-obj:
	@$(call print_message,"Deleting the $(OBJDIR) folder...")
	@rm -rf $(OBJDIR)

.PHONY:\
clean-bin #! Deletes the ./bin folder
clean-bin:
	@$(call print_message,"Deleting the $(BINDIR) folder...")
	@rm -rf $(BINDIR)

.PHONY:\
clean-log #! Deletes the ./log folder
clean-log:
	@$(call print_message,"Deleting the $(LOGDIR) folder...")
	@rm -rf $(LOGDIR)
