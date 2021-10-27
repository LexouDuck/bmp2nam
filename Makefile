#! This is the root-level Makefile, which includes all the others

# NOTE: the two following lines are to stay at the very top of this Makefile and never move
MKFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CURRENT_DIR := $(patsubst %/,%,$(dir $(MKFILE_PATH)))

#######################################
#          Project variables          #
#######################################

#! Output filename for the program
NAME = bmp2nam



#######################################
#      Project folder structure       #
#######################################

# repository folders

#! The directory for header code files (stores `.h` files)
HDRDIR = ./src/
#! The directory for source code files (stores `.c` files)
SRCDIR = ./src/
#! The directory for dependency library files (stores libs - static:`.a` or dynamic:`.dll`/`.dylib`/`.so`)
LIBDIR = ./lib/
#! The directory for documentation (stores config and scripts to generate doc)
DOCDIR = ./doc/
#! The directory for testing programs (stores source/header code for the various testing programs)
TEST_DIR = ./test/

# generated folders

#! The directory for object assembly files (stores `.o` files)
OBJDIR = ./obj/
#! The directory for built binary files (stores programs/libraries built by this project)
BINDIR = ./bin/
#! The directory for distribution archives (stores `.zip` distributable builds)
DISTDIR = ./dist/
#! The directory for output logs (stores `.txt` outputs of the test suite program)
LOGDIR = ./log/
#! The directory for linter/static analyzer output logs (stores warnings logs)
LINTDIR = ./lint/
#! The directory for temporary (can be used for several things - should always be deleted after use)
TEMPDIR = ./temp/



#######################################
#          Project variables          #
#######################################

#! Path of the file which stores the list of header code files
HDRSFILE = make/lists/hdrs.txt

#! Path of the file which stores the list of source code files
SRCSFILE = make/lists/srcs.txt



#######################################
#     Included Makefile Variables     #
#######################################

# general variables
include make/utils/ansi.mk
include make/utils/sudo.mk
include make/utils/ext.mk

# project-specific rules
include make/config/modes.mk
include make/config/build.mk
include make/config/packages.mk



#######################################
#           Main build rules          #
#######################################

.PHONY:\
all # Builds all targets (this is the default rule)
all: $(MODE)

.PHONY:\
debug # Builds the library, in 'debug' mode (with debug flags and symbol-info)
debug: MODE = debug
debug: CFLAGS += $(CFLAGS_DEBUG)
debug: $(NAME)

.PHONY:\
release # Builds the library, in 'release' mode (with debug flags and symbol-info)
release: MODE = release
release: CFLAGS += $(CFLAGS_RELEASE)
release: $(NAME)



.PHONY:\
re # Deletes all generated files and rebuilds `all`
re: clean-full all



#######################################
#      Included Makefile Rules        #
#######################################

# general rules
include make/utils/help.mk

# project-specific rules
include make/rules/build-program.mk
include make/rules/install-program.mk

include make/rules/init.mk
include make/rules/prereq.mk
include make/rules/clean.mk
include make/rules/version.mk
include make/rules/packages.mk
include make/rules/dist.mk

include make/rules/lint.mk
include make/rules/format.mk
include make/rules/debugging.mk

include make/packages/libccc.mk
include make/packages/SDL2.mk
