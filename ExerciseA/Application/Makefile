# Makefile (.C VERSION) for MPS
# by TeamMPS
# with automatic source detection
EXENAME   := main
SOURCES   :=
LD_FLAGS  :=
CXX_FLAGS := -Wall



# Auto add sources if none
ifeq ($(SOURCES),)
	C_FILES := $(wildcard *.c)
else
	C_FILES := $(SOURCES)
endif
# Crosscompiling support
ifneq ($(ARCH),target)
	CXX=gcc
	DIR=host
	EXE=$(EXENAME)
else
	CXX=arm-angstrom-linux-gnueabi-g++
	DIR=target
	EXE=$(addsuffix .target,$(EXENAME))
endif
OBJ_FILES := $(addprefix $(DIR)/,$(notdir $(C_FILES:.c=.o)))

$(EXE): $(OBJ_FILES)
	$(CXX) $(LD_FLAGS) -o $@ $^ 

$(DIR)/%.o: %.c | $(DIR)
	$(CXX) $(CXX_FLAGS) -c -o $@ $<

$(DIR):
	@mkdir $(DIR)

clean: 
	@rm $(OBJ_FILES) $(OBJ_FILES:.o=.d) $(EXE)
	@rmdir $(DIR)
	
help:
	@echo "Instructions for makefile:"
	@echo "make [OPTIONS]                 make for host"
	@echo "make ARCH=target [OPTIONS]     make for target"
	@echo "OPTIONS:"
	@echo "clean          removes objects directory and executable"

# Section for dependency files:
CXX_FLAGS += -MMD
-include $(OBJ_FILES:.o=.d)
# end of file
