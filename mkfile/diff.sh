#!/bin/sh

# this script is used to compare makefiles
PROJECT_1=~/Projects/libccc
PROJECT_2=~/Projects/bmp2nam
FILES='
	Makefile
	make/config/build.mk
	make/config/modes.mk
	make/lists/packages.txt
	make/rules/all.mk
	make/rules/build.mk
	make/rules/clean.mk
	make/rules/lint.mk
	make/rules/lists.mk
	make/rules/packages.mk
	make/rules/version.mk
	make/rules/install.mk
	make/rules/lint.mk
	make/rules/prereq.mk
'
for i in $FILES
do
	echo ""
	echo "===================="
	echo "| $i "
	echo "===================="
	diff $PROJECT_1/$i $PROJECT_2/$i
done
