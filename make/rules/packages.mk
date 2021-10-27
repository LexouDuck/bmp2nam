#! This file holds rules to build dependencies (libraries, packages, binaries, etc)



.PHONY:\
packages # This rule builds/prepares the dependency external packages
packages: \
	$(addprefix package-, $(PACKAGES))



.PHONY:\
update-all # This rule update the dependency external packages to their latest versions
update-all: \
	$(addprefix update-, $(PACKAGES))



# TODO: package manager update logic (for submodules, and wget/curl packages)
#
#GIT_SUBMODULES = $(shell cat .gitmodules | grep "\[submodule" | cut -d'"' -f 2)
#
#.PHONY:\
#update-submodules # Updates git submodules
#update-submodules:
#	@for i in $(GIT_SUBMODULES) ; do \
#		cd $$i ; \
#		if git status | grep -q "HEAD detached" ; then \
#			printf $(C_YELLOW)"WARNING"$(C_RESET)": Your git submodule "$$i" is in detached HEAD state""\n" ; \
#			printf "You need to manually go into the submodule folder and do 'git checkout master',""\n" ; \
#			printf "after making sure that you have no uncommitted/unpushed local working changes.""\n" ; \
#		fi ; \
#		git pull ; \
#		cd - ; \
#	done

