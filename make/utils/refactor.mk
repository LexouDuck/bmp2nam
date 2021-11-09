#! This file holds useful rules to refactor source code (language-agnostic)



TEMPFILE = refactor.tmp



.PHONY:\
refactor-indent # On all source files: changes any tab characters which are not at the start of a line into spaces
refactor-indent:
	@for i in $(SRCS) ; do \
		echo $$i ; \
		cat $$i \
		| expand -t 4 \
		| unexpand -t 4 \
		> $(TEMPFILE) ; \
		mv $(TEMPFILE) $$i ; \
	done



.PHONY:\
refactor-replace # On all source files: performs a regular expression match&replace (args: `OLD` and `NEW`)
refactor-replace:
	@if [ "$(OLD)" == "" ]; then printf $(C_RED)"ERROR"$(C_RESET)": no 'OLD' argument specified\n" ; exit 1 ; fi
	@if [ "$(NEW)" == "" ]; then printf $(C_RED)"ERROR"$(C_RESET)": no 'NEW' argument specified\n" ; exit 1 ; fi
	@for i in $(SRCS) ; do \
		sed -E 's/$(OLD)/$(NEW)/g' $$i \
		> $(TEMPFILE) ; \
		mv $(TEMPFILE) $$i ; \
	done
