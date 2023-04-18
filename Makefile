OS = $(shell uname -s)

# CFLAGS += -march=native -masm=intel -pipe -fsanitize=address,signed-integer-overflow,undefined -pedantic -Wall -Wextra -Werror -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wrestrict -Wnull-dereference -Wdouble-promotion -Wshadow -Wformat=2 -Wfloat-equal -Wundef -Wpointer-arith -Wcast-align -Wstrict-overflow=5 -Wwrite-strings -Wswitch-default -Wswitch-enum -Wconversion -Wunreachable-code -Winit-self -fno-strict-aliasing -Wno-unknown-warning-option -Isrc -Ilib -Isubmodules/rmrf/src
CFLAGS += -march=native -masm=intel -pipe -pedantic -Wall -Wextra -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wrestrict -Wnull-dereference -Wdouble-promotion -Wshadow -Wformat=2 -Wfloat-equal -Wundef -Wpointer-arith -Wcast-align -Wstrict-overflow=5 -Wwrite-strings -Wswitch-default -Wswitch-enum -Wconversion -Wunreachable-code -Winit-self -fno-strict-aliasing -Wno-unknown-warning-option -Isrc -Ilib -Isubmodules/rmrf/src -Isrc/allocators

CXXFLAGS += ${CFLAGS} -std=c++2a -Wuseless-cast -Weffc++ -I/usr/local/include -Wno-non-virtual-dtor
DEPFLAGS = -MT $@ -MMD -MP -MF $(patsubst ${OBJDIR}/%.o,${DEPDIR}/%.d,$@) -pthread

ifeq "${OS}" "Linux"
CFLAGS += -flto
CXXFLAGS += -flto
# LFLAGS += -flto -lnl
LFLAGS += -flto
else
CXXFLAGS += -Wno-undef -Wno-error
endif

LFLAGS += -lev -lprotobuf

ifeq "${BUILD_MODE}" "Release"
# TODO add march for target arch once decision has been made
# TODO make sure AVX2 is avaiable on the selected embedded processor
CFLAGS += -O3 -fomit-frame-pointer -funroll-loops -mavx2
CXXFLAGS += -O3 -fomit-frame-pointer -funroll-loops -mavx2
else
CFLAGS += -g -Og -march=native -masm=intel
CXXFLAGS += -g -Og -march=native -masm=intel
endif

PKG_TOOL = pkg-config
PROTO_TOOL = protoc
XSDTOOL = xsdcxx

CFLAGS_RMRF_NET += `${PKG_TOOL} --cflags libnl-3.0`
CXXFLAGS_RMRF_NET += `${PKG_TOOL} --cflags libnl-3.0`

CFLAGS_PROTO += `${PKG_TOOL} --cflags protobuf`
CXXFLAGS_PROTO += `${PKG_TOOL} --cflags protobuf`

DEPFLAGS += `${PKG_TOOL} --cflags spdlog`
# LFLAGS += `${PKG_TOOL} --libs libevent`
LFLAGS += `${PKG_TOOL} --libs spdlog`
LFLAGS += `${PKG_TOOL} --libs protobuf`
LFLAGS += `${PKG_TOOL} --libs xerces-c`
LFLAGS += `${PKG_TOOL} --libs fmt`
LFLAGS += `${PKG_TOOL} --libs libusb`
LFLAGS += `${PKG_TOOL} --libs libftdi`
CFLAGS += `${PKG_TOOL} --cflags xerces-c`
CFLAGS += `${PKG_TOOL} --cflags libusb`
CFLAGS += `${PKG_TOOL} --cflags libftdi`

CXXFLAGS_A_PROTO := ${CXXFLAGS}
# CXXFLAGS += -Werror
# CFLAGS += -Werror

CC ?= gcc
CXX ?= g++

MKDIR ?= mkdir -p
INSTALL ?= install
prefix ?= /usr
XGETTEXT ?= xgettext
XGETTEXT_FLAGS ?= -k_ -c -s -i --no-wrap --force-po --from-code=UTF-8 --check=ellipsis-unicode --sentence-end=single-space \
    --foreign-user --package-name=${PODOMAIN} --package-version=${POVERSION}
MSGINIT ?= msginit
MSGMERGE ?= msgmerge
MSGFMT ?= msgfmt
SED ?= sed

SRCDIR ?= src
LIBSRCDIR ?= lib
BINDIR ?= bin
DEPDIR ?= dep
OBJDIR ?= obj
POTDIR ?= po/tpl
PODIR ?= po/lang
MODIR ?= po/bin

TESTDIR ?= test
TESTBINDIR ?= bin/test
TESTOBJDIR ?= obj/test

rwildcard = $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))

SOURCES := $(call rwildcard,${SRCDIR},*.cpp *.c)
RMRFNET_SRCDIR := ${LIBSRCDIR}/rmrf-net
RMRFNET_SOURCES := $(call rwildcard,${RMRFNET_SRCDIR}/,*.cpp *.c)
#RMRFNET_SRCOBJS := ${LIBSRCDIR}/rmrf-net
RMRFNET_OBJDIR := ${OBJDIR}/rmrf-net

SRCOBJS := $(patsubst ${SRCDIR}/%.c,${OBJDIR}/%.o,$(patsubst ${SRCDIR}/%.cpp,${OBJDIR}/%.o,${SOURCES}))
RMRFNET_SRCOBJS := $(patsubst ${LIBSRCDIR}/rmrf-net/%.c,${OBJDIR}/rmrf-net/%.o,$(patsubst ${LIBSRCDIR}/rmrf-net/%.cpp,${OBJDIR}/rmrf-net/%.o,${RMRFNET_SOURCES}))
DEPFLAGS_RMRF = ${DEPFLAGS} -Isubmodules/rmrf/src -Wno-unused-command-line-argument -Wno-unused-parameter -Wno-shadow


PROTO_DEFDIR := ${LIBSRCDIR}/IPCMessages
PROTO_SRCDIR := ${LIBSRCDIR}/proto_src
PROTO_SOURCES_A := $(call rwildcard,${PROTO_DEFDIR},*.proto)
PROTO_SOURCES_B := $(patsubst ${PROTO_DEFDIR}/%.proto,${PROTO_SRCDIR}/%.pb.cc,${PROTO_SOURCES_A})
PROTO_OBJDIR := ${OBJDIR}/proto

TEST_SOURCES := $(call rwildcard,${TESTDIR},*.cpp *.c)
TEST_SRCOBJS := $(patsubst ${TESTDIR}/%.c,${TESTOBJDIR}/%.o,$(patsubst ${TESTDIR}/%.cpp,${TESTOBJDIR}/%.o,${TEST_SOURCES}))
TEST_TARGETS := $(patsubst ${TESTOBJDIR}/%.o,${TESTBINDIR}/%, $(filter %_test.o, ${TEST_SRCOBJS}))

PROTO_SRCOBJS := $(patsubst ${PROTO_SRCDIR}/%.pb.cc,${PROTO_OBJDIR}/%.o,$(patsubst ${PROTO_SRCDIR}/%.pb.cc,${PROTO_OBJDIR}/%.o,${PROTO_SOURCES_B}))
DEPFLAGS_PROTO = ${DEPFLAGS} -Wno-unused-command-line-argument -Wno-unused-parameter -Wno-shadow

XMLTREE_DEFDIR := ${LIBSRCDIR}/ProjectFile
XMLTREE_SRCDIR := ${LIBSRCDIR}/xml_src
XMLTREE_SCHEMAS := $(call rwildcard,${XMLTREE_DEFDIR},*.xsd)
XMLTREE_CFILES := $(patsubst ${XMLTREE_DEFDIR}/%.xsd,${XMLTREE_SRCDIR}/%.xml.cpp,${XMLTREE_SCHEMAS})
XMLTREE_OBJDIR := ${OBJDIR}/project_xml
XMLTREE_SRCOBJS := $(patsubst ${XMLTREE_SRCDIR}/%.xml.cpp,${XMLTREE_OBJDIR}/%.o,$(patsubst ${XMLTREE_SRCDIR}/%.xml.cpp,${XMLTREE_OBJDIR}/%.o,${XMLTREE_CFILES}))
XMLTREE_CXXFLAGS := ${CXXFLAGS}

XSD_ARGS := --generate-doxygen --generate-polymorphic --std c++11 --hxx-suffix .xml.hpp --cxx-suffix .xml.cpp

OBJECTS := $(filter-out %_test.o ,${TEST_SRCOBJS}) $(filter-out obj/main.o ,${SRCOBJS}) ${OBJDIR}/libproto.a ${OBJDIR}/librmrfnet.a ${OBJDIR}/showxml.a

.PRECIOUS: ${DEPDIR}/%.d ${OBJDIR}/**/%.o ${POTOBJS} ${POOBJS}
.PHONY: all tools test clean install lintian style translation

all: ${BINDIR}/fish tools
	echo Done

test: ${TEST_TARGETS} all
	for a in ${TEST_TARGETS}; do \
		echo $$a; \
		$$a; \
	done

tools: ${BINDIR}/tools/sample_xml_generator
	echo Created tools.

${PROTO_SRCDIR}/%.pb.cc: ${PROTO_DEFDIR}/%.proto Makefile
	${MKDIR} ${@D} && ${PROTO_TOOL} -I=${PROTO_DEFDIR} --cpp_out=${PROTO_SRCDIR} $< && touch $@

${PROTO_OBJDIR}/%.o: ${PROTO_SRCDIR}/%.pb.cc $(PROTO_SOURCES_B)
	${MKDIR} ${@D} && ${MKDIR} $(patsubst ${OBJDIR}/%,${DEPDIR}/%,${@D}) && ${CXX} ${CXXFLAGS_A_PROTO} ${DEPFLAGS_PROTO} ${CXXFLAGS_PROTO} -o $@ -c $< && touch $@

${OBJDIR}/libproto.a: ${PROTO_SRCOBJS}
	${MKDIR} ${@D} && ar rsv $@ $^ && touch $@

${XMLTREE_SRCDIR}/%.xml.cpp: ${XMLTREE_DEFDIR}/%.xsd Makefile
	${MKDIR} ${@D} && cd ${@D} && ${XSDTOOL} cxx-tree ${XSD_ARGS} ../../$< && cd ../.. && touch $@

${XMLTREE_OBJDIR}/%.o: ${XMLTREE_SRCDIR}/%.xml.cpp $(XMLTREE_CFILES)
	${MKDIR} ${@D} && ${MKDIR} $(patsubst ${OBJDIR}/%,${DEPDIR}/%,${@D}) && ${CXX} ${XMLTREE_CXXFLAGS} -o $@ -c $< && touch $@

${OBJDIR}/showxml.a: ${XMLTREE_SRCOBJS}
	${MKDIR} ${@D} && ar rsv $@ $^ && touch $@

${RMRFNET_OBJDIR}/%.o: ${RMRFNET_SRCDIR}/%.cpp Makefile
	${MKDIR} ${@D} && ${MKDIR} $(patsubst ${OBJDIR}/%,${DEPDIR}/%,${@D}) && ${CXX} ${CXXFLAGS} ${DEPFLAGS_RMRF} ${CXXFLAGS_RMRF_NET} -o $@ -c $< && touch $@

${OBJDIR}/librmrfnet.a: ${RMRFNET_SRCOBJS}
	${MKDIR} ${@D} && ar rsv $@ $^ && touch $@

${OBJDIR}/%.o: ${SRCDIR}/%.cpp $(PROTO_SOURCES_B) $(XMLTREE_CFILES) Makefile
	${MKDIR} ${@D} && ${MKDIR} $(patsubst ${OBJDIR}/%,${DEPDIR}/%,${@D}) && ${CXX} ${CXXFLAGS} ${DEPFLAGS} -o $@ -c $< && touch $@

${BINDIR}/fish: ${SRCOBJS} ${OBJDIR}/librmrfnet.a ${OBJDIR}/libproto.a ${OBJDIR}/showxml.a
	${MKDIR} ${@D} && ${CXX} -o $@ $^ ${LFLAGS} && touch $@

${TESTDIR}/%.ldflags:
	touch $@

${TESTBINDIR}/%: ${TESTOBJDIR}/%.o ${OBJECTS} Makefile ${TESTDIR}/%.ldflags all
	${MKDIR} ${@D} && ${MKDIR} $(patsubst ${TESTOBJDIR}/%,${DEPDIR}/%,${@D}) && ${CXX} ${CXXFLAGS}  $< $(shell [ -r $(patsubst ${TESTOBJDIR}/%.o,${TESTDIR}/%.ldflags,$<) ] && cat $(patsubst ${TESTOBJDIR}/%.o,${TESTDIR}/%.ldflags,$<) ) ${OBJECTS} ${LFLAGS} -o $@ && touch $@

${TESTOBJDIR}/%.o: ${TESTDIR}/%.cpp ${DEPDIR}/%.d ${DEPDIR}/test Makefile
	${MKDIR} ${@D} && ${MKDIR} $(patsubst ${TESTOBJDIR}/%,${DEPDIR}/%,${@D}) && ${CXX} -I${TESTDIR} ${CXXFLAGS} ${DEPFLAGS} ${LFLAGS} -o $@ -c $< && touch $@

${TESTOBJDIR}/%.o: ${TESTDIR}/%.c ${DEPDIR}/%.d ${DEPDIR}/test Makefile
	${MKDIR} ${@D} && ${MKDIR} $(patsubst ${TESTOBJDIR}/%,${DEPDIR}/%,${@D}) && ${CC} -I${TESTDIR} -std=c11 -std=c17 ${CFLAGS} ${DEPFLAGS} ${LFLAGS} -o $@ -c $< && touch $@

${DEPDIR}/%.d: ;

${BINDIR}/tools/sample_xml_generator: Makefile ${XMLTREE_DEFDIR}/ShowFile_v0.xsd
	${MKDIR} ${@D} && ${MKDIR} tools/generator-tmp && cd tools/generator-tmp && \
	${XSDTOOL} cxx-tree ${XSD_ARGS} --generate-serialization ../../${XMLTREE_DEFDIR}/ShowFile_v0.xsd && cd ../.. && \
	${CXX} ${CFLAGS} ${CXXFLAGS} -Itools ${DEPFLAGS} tools/sample_xml_generator.cpp tools/generator-tmp/ShowFile_v0.xml.cpp ${LFLAGS} -o $@

${DEPDIR}/test:
	${MKDIR} ${DEPDIR}/test

clean:
	rm -rf ${BINDIR}
	rm -rf ${OBJDIR}
	rm -rf ${DEPDIR}
	rm -rf ${MODIR}
	rm -rf ${PROTO_SRCDIR}
	rm -rf ${XMLTREE_SRCDIR}
	rm -rf tools/generator-tmp
