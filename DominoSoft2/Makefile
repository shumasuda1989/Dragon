TARGET = Rootify EventDisp EthDispCas Pedestal
TARGET2 = Eth Mkoffset
TARGET3 = rbcp
TARGET4 = rbcp2
ALLTRGT = $(TARGET) $(TARGET2) $(TARGET3) $(TARGET4)

SUBDIR = RBCP
CURDIR = $(shell pwd)

DEP = dep.d
TARGETSRCS = $(addsuffix .cc, $(basename $(TARGET)))
SRCS = DSubFunc.cc CDomino.cc CDominoDict.cc
TARGETOBJS = $(addsuffix .o, $(basename $(TARGET)))
OBJS = $(addsuffix .o,$(basename $(SRCS)))

include $(ROOTSYS)/etc/Makefile.arch

all: dep $(ALLTRGT) libCDomino.so  LN

$(TARGET): % : %.o $(OBJS)
	$(LD) $(LDFLAGS) $^ $(LIBS) $(OutPutOpt)$@

CDominoDict.cc: CDomino.h LinkDef.h
	$(ROOTCINT) -f $@ -c $^

libCDomino.so: $(OBJS) 
ifeq ($(ARCH),aix5)
		$(MAKESHARED) $(OutPutOpt) $@ $(LIBS) -p 0 $^
else
ifeq ($(PLATFORM),macosx)
# We need to make both the .dylib and the .so
		$(LD) $(SOFLAGS)$@ $(LDFLAGS) $^ $(OutPutOpt) $@ $(EXPLLINKLIBS)
ifneq ($(subst $(MACOSX_MINOR),,1234),1234)
ifeq ($(MACOSX_MINOR),4)
                ln -sf $@ $(subst .$(DllSuf),.so,$@)
endif
endif
else
ifeq ($(PLATFORM),win32)
		bindexplib $* $^ > $*.def
		lib -nologo -MACHINE:IX86 $^ -def:$*.def \
                   $(OutPutOpt)$(EVENTLIB)
		$(LD) $(SOFLAGS) $(LDFLAGS) $^ $*.exp $(LIBS) \
		   $(OutPutOpt)$@
		$(MT_DLL)
else
		$(LD) $(SOFLAGS) $(LDFLAGS) $^ $(LIBS) $(OutPutOpt) $@ $(EXPLLINKLIBS)
endif
endif
endif

$(TARGET2): % : $(addsuffix .cc, % )
	$(CXX) -o $@ $<

$(TARGET3): $(SUBDIR)/$(addsuffix .c, $(basename $(TARGET3)))
	$(CC) -o $@ $<

$(TARGET4): $(SUBDIR)/$(addsuffix .c, $(basename $(TARGET4)))
	$(CC) -o $@ $<

.cc.o : 
	$(CXX) $(CXXFLAGS) -c $<

.c.o:
	$(CC) $(CXXFLAGS) -c $<

LN: $(ALLTRGT) libCDomino.so
	@cd .. ; ln -sf $(addprefix $(CURDIR)/,$^) ./

dep: $(SRCS) EthDispCas.cc
	@$(CXX) -MM $(CXXFLAGS) $^ > $(DEP)

-include dep.d

clean:
	$(RM) $(ALLTRGT) $(addprefix ../,$(ALLTRGT) libCDomino.so) *.o $(DEP) CDominoDict.cc CDominoDict.h libCDomino.so *~

echo:
	@echo $(CURDIR)