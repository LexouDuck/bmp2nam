#! This file holds rules to build dependencies (libraries, packages, binaries, etc)



.PHONY:\
packages # This rule builds/prepares the dependency external packages
packages: \
	package-libccc \
	package-SDL2 \
