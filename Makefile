S = $(shell uname -s)

# CFLAGS += -march=native -masm=intel -pipe -fsanitize=address,signed-integer-overflow,undefined -pedantic -Wall -Wextra -Werror -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wrestrict -Wnull-dereference -Wdouble-promotion -Wshadow -Wformat=2 -Wfloat-equal -Wundef -Wpointer-arith -Wcast-align -Wstrict-overflow=5 -Wwrite-strings -Wswitch-default -Wswitch-enum -Wconversion -Wunreachable-code -Winit-self -fno-strict-aliasing -Wno-unknown-warning-option -Isrc -Ilib -Isubmodules/rmrf/src
CFLAGS += -march=native -masm=intel -pipe -pedantic -Wall -Wextra -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wrestrict -Wnull-dereference -Wdouble-promotion -Wshadow -Wformat=2 -Wfloat-equal -Wundef -Wpointer-arith -Wcast-align -Wstrict-overflow=5 -Wwrite-strings -Wswitch-default -Wswitch-enum -Wconversion -Wunreachable-code -Winit-self -fno-strict-aliasing -Wno-unknown-warning-option -Isrc -Ilib -Isubmodules/rmrf/src

CXXFLAGS += ${CFLAGS} -std=c++11 -std=c++17 -std=c++2a -Wuseless-cast -Weffc++ -I/usr/local/include -Wno-non-virtual-dtor
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
CFLAGS += -O3 -fomit-fame-pointer -funrollloop -mavx2
CXXFLAGS += -O3 -fomit-fame-pointer -funrollloop -mavx2
else
CFLAGS += -g -Og -march=native -masm=intel
CXXFLAGS += -g -Og -march=native -masm=intel
endif

PKG_TOOL = pkg-config
PROTO_TOOL = protoc

CFLAGS_RMRF_NET += `${PKG_TOOL} --cflags libnl-3.0`
CXXFLAGS_RMRF_NET += `${PKG_TOOL} --cflags libnl-3.0`

CFLAGS_PROTO += `${PKG_TOOL} --cflags protobuf`
CXXFLAGS_PROTO += `${PKG_TOOL} --cflags protobuf`

DEPFLAGS += `${PKG_TOOL} --cflags spdlog`
# LFLAGS += `${PKG_TOOL} --libs libevent`
LFLAGS += `${PKG_TOOL} --libs spdlog`

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
APPDIR ?= ${SRCDIR}/app
BINDIR ?= bin
DEPDIR ?= dep
OBJDIR ?= obj
POTDIR ?= po/tpl
PODIR ?= po/lang
MODIR ?= po/bin


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

PROTO_SRCOBJS := $(patsubst ${PROTO_SRCDIR}/%.pb.cc,${PROTO_OBJDIR}/%.o,$(patsubst ${PROTO_SRCDIR}/%.pb.cc,${PROTO_OBJDIR}/%.o,${PROTO_SOURCES_B}))
DEPFLAGS_PROTO = ${DEPFLAGS} -Wno-unused-command-line-argument -Wno-unused-parameter -Wno-shadow

.PRECIOUS: ${DEPDIR}/%.d ${OBJDIR}/**/%.o ${POTOBJS} ${POOBJS}
.PHONY: all clean install lintian style translation

all: ${BINDIR}/fish
	echo Done

${PROTO_SRCDIR}/%.pb.cc: ${PROTO_DEFDIR}/%.proto Makefile
	${MKDIR} ${@D} && ${PROTO_TOOL} -I=${PROTO_DEFDIR} --cpp_out=${PROTO_SRCDIR} $< && touch $@

${PROTO_OBJDIR}/%.o: ${PROTO_SRCDIR}/%.pb.cc $(PROTO_SOURCES_B)
	${MKDIR} ${@D} && ${MKDIR} $(patsubst ${OBJDIR}/%,${DEPDIR}/%,${@D}) && ${CXX} ${CXXFLAGS_A_PROTO} ${DEPFLAGS_PROTO} ${CXXFLAGS_PROTO} -o $@ -c $< && touch $@

${OBJDIR}/libproto.a: ${PROTO_SRCOBJS}
	${MKDIR} ${@D} && ar rsv $@ $^ && touch $@

${RMRFNET_OBJDIR}/%.o: ${RMRFNET_SRCDIR}/%.cpp Makefile
	${MKDIR} ${@D} && ${MKDIR} $(patsubst ${OBJDIR}/%,${DEPDIR}/%,${@D}) && ${CXX} ${CXXFLAGS} ${DEPFLAGS_RMRF} ${CXXFLAGS_RMRF_NET} -o $@ -c $< && touch $@

${OBJDIR}/librmrfnet.a: ${RMRFNET_SRCOBJS}
	${MKDIR} ${@D} && ar rsv $@ $^ && touch $@

${OBJDIR}/%.o: ${SRCDIR}/%.cpp $(PROTO_SOURCES_B) Makefile
	${MKDIR} ${@D} && ${MKDIR} $(patsubst ${OBJDIR}/%,${DEPDIR}/%,${@D}) && ${CXX} ${CXXFLAGS} ${DEPFLAGS} -o $@ -c $< && touch $@

${BINDIR}/fish: ${SRCOBJS} ${OBJDIR}/librmrfnet.a ${OBJDIR}/libproto.a
	${MKDIR} ${@D} && ${CXX} -o $@ $^ ${LFLAGS} && touch $@

clean:
	rm -rf ${BINDIR}
	rm -rf ${OBJDIR}
	rm -rf ${DEPDIR}
	rm -rf ${MODIR}
	rm -rf ${PROTO_SRCDIR}
