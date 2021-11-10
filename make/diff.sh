#!/bin/sh

# this script is used to compare makefiles
PROJECT_1=~/Projects/bmp2nam
PROJECT_2=~/Projects/libccc
FILES='
	make/config/build.mk
	make/config/modes.mk
	make/config/packages.mk
	make/lists/packages.txt
	make/rules/all.mk
	make/rules/build.mk
	make/rules/clean.mk
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
