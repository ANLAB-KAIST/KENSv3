.PHONY: all clean

MAKE=make

DIRS=
SRCDIR=app
OUTPUT_DIR=build

all:
	@$(MAKE) --directory=src all
	@for dir in $(SRCDIR)/*; do \
		if [ -f $$dir/Makefile ]; then \
			$(MAKE) --directory=$$dir all; \
		fi ; \
	done

clean:
	rm -f *.o $(OUTPUT_DIR)/*.pcap *.pcap *.xml
	rm -rf $(OUTPUT_DIR)/html
	@$(MAKE) --directory=src clean
	@for dir in $(SRCDIR)/*; do \
		if [ -f $$dir/Makefile ]; then \
			$(MAKE) --directory=$$dir clean; \
		fi ; \
	done

depend:
	@$(MAKE) --directory=src depend
	@for dir in $(SRCDIR)/*; do \
		if [ -f $$dir/Makefile ]; then \
			$(MAKE) --directory=$$dir depend; \
		fi ; \
	done

test: test_part1 test_part2 test_part3 test_part4

test_part1: all
	@echo "Running test cases for project1..."
	@-build/testTCP --gtest_filter="TestEnv_Reliable.TestOpen:TestEnv_Reliable.TestBind_*" --gtest_output=xml:part1.xml

test_part2: test_part1
	@echo "Running test cases for project2..."
	@-build/testTCP --gtest_filter="TestEnv_Reliable.TestAccept_*:TestEnv_Any.TestAccept_*:TestEnv_Any.TestConnect_*:TestEnv_Any.TestClose_*" --gtest_output=xml:part2.xml

test_part3: test_part2
	@echo "Running test cases for project3..."
	@-build/testTCP --gtest_filter="TestEnv_Any.TestTransfer_*" --gtest_output=xml:part3.xml

test_part4: test_part3
	@echo "Running test cases for project4..."
	@-build/testTCP --gtest_filter="TestEnv_Congestion*" --gtest_output=xml:part4.xml
	@echo "Note that passing this test does not mean that you are finished."
	@echo "Check the pcap file that you have implemented congestion control well."

doxygen:
	doxygen doxygen/Doxyfile

.PHONY: all clean test test_part1 test_part2 test_part3 test_part4 depend doxygen
