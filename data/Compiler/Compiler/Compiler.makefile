ENGINE_ROOT_DIR := ./../../..

BUILDDIR   := ./build
OUTDIR     := ../gccRelease
EXECUTABLE := Compiler
INCLUDES   := $(ENGINE_ROOT_DIR)/ $(BUILDDIR)/
DEFINES    := GCC_BUILD NDEBUG _LINUX

ENGINE_DIR := $(ENGINE_ROOT_DIR)/Engine

# Postprocess defines
DEFINES  := $(addprefix -D, ${DEFINES})
INCLUDES := $(addprefix -I, ${INCLUDES})

# -Wno-multichar - we use these for sdc resoruce headers
CXXFLAGS := -O2 -flto -std=c++11 -Wall -Wno-multichar -pthread $(INCLUDES) $(DEFINES) $(EXTRA_FLAGS) $(EXTRA_CXXFLAGS)
CFLAGS   := -O2 -flto -std=c11 -Wall -pthread $(INCLUDES) $(DEFINES) $(EXTRA_FLAGS) $(EXTRA_CFLAGS)
LDFLAGS  := -O2 -flto $(EXTRA_FLAGS) $(EXTRA_LDFLAGS)
LDLIBS   := -lpthread

SOURCES := \
    AbstractItemManager.cpp \
    Article.cpp \
    Articles.cpp \
    ArticlesCompress.cpp \
    ArticlesLoader.cpp \
    Compiler.cpp \
    CompressByCharChain.cpp \
    CompressByCountChar.cpp \
    CompressByStore.cpp \
    Compress.cpp \
    CSSDataManager.cpp \
    FileResourceManagement.cpp \
    ImageManager.cpp \
    ListCompress.cpp \
    ListLoader.cpp \
    Log.cpp \
    Material.cpp \
    MaterialManager.cpp \
    MetadataManager.cpp \
    MorphoDataManager.cpp \
    MyProcessTimer.cpp \
    Out.cpp \
    Platform.cpp \
    Scene.cpp \
    SceneManager.cpp \
    SDC_CRC.cpp \
    SDCRead.cpp \
    SDCWrite.cpp \
    SlovoEdProject.cpp \
    SortTablesManager.cpp \
    SoundContainer.cpp \
    SoundManager.cpp \
    SpeexPack.cpp \
    StringCompare.cpp \
    StringFunctions.cpp \
    StringStore.cpp \
    Styles.cpp \
    ThreadPool.cpp \
    Tools.cpp \
    TreeCharChainCompressor.cpp \
    VideoManager.cpp \
    WordList.cpp \
    WordLists.cpp \
    XmlTagContent.cpp \
    PugiXML/pugixml.cpp \
    lz4/lz4.c \
    lz4/lz4hc.c \
    fmt/format.cc \
    fmt/printf.cc \

OBJECTS := $(patsubst %.cpp, ${BUILDDIR}/%.o, $(filter %.cpp, ${SOURCES})) \
           $(patsubst %.cc, ${BUILDDIR}/%.o, $(filter %.cc, ${SOURCES})) \
           $(patsubst %.c, ${BUILDDIR}/%.o, $(filter %.c, ${SOURCES}))

BINARY = $(OUTDIR)/$(EXECUTABLE)

.PHONY: all echo_setup clean

all: echo_setup $(SOURCES) $(BINARY)

# Prints the compiler invocation string
echo_setup:
	@echo Using CXX = '`'$(CXX) $(CXXFLAGS)'`'
	@echo Using CC = '`'$(CC) $(CFLAGS)'`'

# Cleans intermediate and output files (objects, libraries, executables)...
clean:
	@echo Cleaning up...
	@$(RM) -r $(BUILDDIR)
	@$(RM) $(OUTDIR)/*.a
	@$(RM) $(OUTDIR)/*.so
	@$(RM) $(OUTDIR)/*.dll
	@$(RM) $(OUTDIR)/*.exe

# Linking our main binary
$(BINARY): $(OBJECTS)
	@echo Linking...
	@mkdir -p $(dir $@)
	$(CXX) $(LDFLAGS) $(OBJECTS) $(LDLIBS) -o $@

# Rules for automagically building the metadata strings serialization code
MetadataManager.cpp : $(BUILDDIR)/MetadataManagerStrings.incl
$(BUILDDIR)/MetadataManagerStrings.incl : $(ENGINE_DIR)/SldMetadataTypes.h
	@echo GEN $(notdir $@)
	@mkdir -p $(dir $@)
	@./$(notdir $@).in.py $< $@

# Rule for automatically building css props descriptor tables
CSSDataManager.cpp : $(BUILDDIR)/CSSProperties.incl
$(BUILDDIR)/CSSProperties.incl : $(ENGINE_DIR)/sld_css_props.py
	@echo GEN $(notdir $@)
	@mkdir -p $(dir $@)
	@./$(notdir $@).py $(dir $<) $@

# Generic rule for building object files (also builds dependency files in the same step)
$(BUILDDIR)/%.o: %.cpp
	@echo CXX $<
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -MMD -MP -o $@ -c $<

$(BUILDDIR)/%.o: %.cc
	@echo CXX $<
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -MMD -MP -o $@ -c $<

$(BUILDDIR)/%.o: %.c
	@echo CC $<
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -MMD -MP -o $@ -c $<

# Include all of the dependency files
-include $(patsubst %.cpp, ${BUILDDIR}/%.d, $(filter %.cpp, ${SOURCES}))
-include $(patsubst %.cc, ${BUILDDIR}/%.d, $(filter %.cc, ${SOURCES}))
-include $(patsubst %.c, ${BUILDDIR}/%.d, $(filter %.c, ${SOURCES}))
