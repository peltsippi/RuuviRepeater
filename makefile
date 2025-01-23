# makefile for sample cpputest

#https://github.com/bveenema/particle_cpputest/blob/master/Makefile

#CPPUTEST_HOME = c:/cygwin/home/bveenema/cpputest-3.8

ifeq "$(CPPUTEST_HOME)" ""
$(error The environment variable CPPUTEST_HOME is not set. \
Set it to where cpputest is installed)
endif

TESTDIR 		= tests/
SRCDIR 			= src

# CPPUTest is C++Write, so using g++ To compile the test file
CPP       	:= g++
CPPFLAGS  	:= -g -Wall
CPPFLAGS  	+= -I $(CPPUTEST_HOME)/include
CPPFLAGS  	+= -I $(SRCDIR)
LDFLAGS 		:= -L $(CPPUTEST_HOME)/cpputest_build/lib -l CppUTest

SRC_FILES 	:= $(wildcard $(SRCDIR)/*.cpp)
SRC_OBJ 		:= $(addprefix $(SRCDIR)/,$(notdir $(SRC_FILES:.cpp=.o)))

TEST_FILES 	:= $(wildcard $(TESTDIR)/*.cpp)
TEST_OBJ 		:= $(addprefix $(TESTDIR)/,$(notdir $(TEST_FILES:.cpp=.o)))


#Build source for test
src: $(SRC_OBJ)

$(SRCDIR)/%.o: %.cpp
	$(eval CPPFLAGS += -D TESTING)
	$(CPP) -c -o $@ $(CPPFLAGS)

# Testing
test: $(TEST_OBJ)
	$(CPP) -o $(TESTDIR)/$@ $(SRCDIR)/*.o $^ $(LDFLAGS)

$(TESTDIR)/%.o: %.cpp
	$(CPP) -c -o $@ $(CPPFLAGS)

.PHONY: set-testing-flag
set-testing-flag:
	$(eval CPPFLAGS += -D TESTING)

.PHONY: run-test
run-test: set-testing-flag src test
	$(TESTDIR)/test.exe

.PHONY: run-test-clean
run-test-clean: clean run-test

.PHONY: clean
clean:
	@echo "clean..."
	rm -f test/*.exe *.o test/*.o src/*.o