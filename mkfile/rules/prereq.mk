#! This file holds project-specific rules to check whether prerequisite programs are correctly installed/available



.PHONY:\
prereq #! Checks all prerequisite tools/programs and their versions
prereq: \
init \
prereq-build \
prereq-dist \
prereq-format \
prereq-lint \
