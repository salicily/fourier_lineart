#################################
# Types

TYPES := bitmap pointslist doubleslist fbase

#################################
# Translators

TRANSLATORS := disk_bitmap:bitmap \
			   bitmap_pointslist:bitmap,pointslist \
			   shortcycle:pointslist \
			   pointslist_doubleslist:pointslist,doubleslist \
			   doubleslist_fourier:doubleslist,fbase \
			   homothetie:doubleslist

TRANSLATORS_LIST := $(foreach i,$(TRANSLATORS), $(shell echo "$(i)" | sed -e s/:.*//))

#################################
# All

all: bin/mini_fourier bin/check_base

#################################
# Binaries

bin/mini_fourier: $(addsuffix .o,$(addprefix build/types/,$(TYPES))) $(addsuffix .o,$(addprefix build/translators/,$(TRANSLATORS_LIST))) mini_fourier.c
	mkdir -p bin
	gcc -Wall -o $@ $^ -lm

bin/check_base: build/types/fbase.o check_base.c
	mkdir -p bin
	gcc -Wall -o $@ $^ -lm

#################################
# Misc

clean:
	rm -Rf build

distclean:
	rm -Rf build bin

DATE := $(shell date +%Y%m%d%H%M)

archive: distclean
	tar --transform "s/^/fourier\//" -cvzf ../fourier-$(DATE).tgz * > /dev/null

.PHONY: clean distclean all archive

#################################
# Framework

define BUILD_TYPE
build/types/$(1).o: types/$(1).c types/$(1).h
	mkdir -p build/types
	gcc -Wall -o build/types/$(1).o -c types/$(1).c
endef

$(foreach i,$(TYPES),$(eval $(call BUILD_TYPE,$(i))))

COMA=,
define BUILD_TRANSLATOR
build/translators/$(1).o: $$(addsuffix .h,$$(addprefix types/,$$(subst $$(COMA), ,$(2)))) translators/$(1).c translators/$(1).h
	mkdir -p build/translators
	gcc -Wall -o build/translators/$(1).o -c translators/$(1).c
endef

$(foreach i,$(TRANSLATORS),$(eval $(call BUILD_TRANSLATOR,$(shell echo "$(i)" | sed -e s/:.*//),$(shell echo "$(i)" | sed -e s/.*://))))
