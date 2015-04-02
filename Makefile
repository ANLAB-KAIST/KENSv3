.PHONY: all clean

MAKE=make

DIRS=
SRCDIR=app
OUTPUT_DIR=build

all:
	@$(MAKE) --directory=src all
	@for dir in $(SRCDIR)/*; do \
		echo $$dir; \
		if [ -f $$dir/Makefile ]; then \
			$(MAKE) --directory=$$dir all; \
		fi ; \
	done

clean:
	rm -f *.o $(OUTPUT_DIR)/*.pcap *.pcap
	rm -rf $(OUTPUT_DIR)/html
	@$(MAKE) --directory=src clean
	@for dir in $(SRCDIR)/*; do \
		echo $$dir; \
		if [ -f $$dir/Makefile ]; then \
			$(MAKE) --directory=$$dir clean; \
		fi ; \
	done

depend:
	@$(MAKE) --directory=src depend
	@for dir in $(SRCDIR)/*; do \
		echo $$dir; \
		if [ -f $$dir/Makefile ]; then \
			$(MAKE) --directory=$$dir depend; \
		fi ; \
	done

doxygen:
	doxygen doxygen/Doxyfile

.PHONY: all clean depend doxygen