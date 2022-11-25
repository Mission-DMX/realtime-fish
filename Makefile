CFLAGS += -march=native -masm=intel -pipe -fsanitize=address,signed-integer-overflow,undefined -pedantic -Wall -Wextra -Werror -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wrestrict -Wnull-dereference -Wdouble-promotion -Wshadow -Wformat=2 -Wfloat-equal -Wundef -Wpointer-arith -Wcast-align -Wstrict-overflow=5 -Wwrite-strings -Wswitch-default -Wswitch-enum -Wconversion -Wunreachable-code -Winit-self -fno-strict-aliasing -Wno-unknown-warning-option -Isrc -Ilib
CXXFLAGS += ${CFLAGS} -std=c++11 -std=c++17 -std=c++2a -Wuseless-cast -Weffc++ -I/usr/local/include -Wno-non-virtual-dtor
DEPFLAGS = -MT $@ -MMD -MP -MF $(patsubst ${OBJDIR}/%.o,${DEPDIR}/%.d,$@) -pthread

ifeq "${OS}" "Linux"
CFLAGS += -flto
CXXFLAGS += -flto
LFLAGS += -flto -lnl
else
CXXFLAGS += -Wno-undef
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

CFLAGS_RMRF_NET += `${PKG_TOOL} --cflags libnl-3.0`
CXXFLAGS_RMRF_NET += `${PKG_TOOL} --cflags libnl-3.0`

DEPFLAGS += `${PKG_TOOL} --cflags spdlog`

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
DEPFLAGS_RMRF := ${DEPFLAGS} -Isubmodules/rmrf/src -Wno-unused-command-line-argument -Wno-unused-parameter -Wno-shadow

.PRECIOUS: ${DEPDIR}/%.d ${OBJDIR}/**/%.o ${POTOBJS} ${POOBJS}
.PHONY: all clean install lintian style translation

all: ${BINDIR}/fish
	echo Done

${RMRFNET_OBJDIR}/%.o: ${RMRFNET_SRCDIR}/%.cpp Makefile
	${MKDIR} ${@D} && ${MKDIR} $(patsubst ${RMRFNET_OBJDIR}/%,${DEPDIR}/%,${@D}) && ${CXX} ${CXXFLAGS} ${CXXFLAGS_RMRF_NET} ${DEPFLAGS_RMRF} -o $@ -c $< && touch $@

${OBJDIR}/librmrfnet.a: ${RMRFNET_SRCOBJS}
	${MKDIR} ${@D} && ${CXX} -o $@ $^ && touch $@

${OBJDIR}/%.o: ${SRCDIR}/%.cpp Makefile
	${MKDIR} ${@D} && ${MKDIR} $(patsubst ${OBJDIR}/%,${DEPDIR}/%,${@D}) && ${CXX} ${CXXFLAGS} ${DEPFLAGS} -o $@ -c $< && touch $@

${BINDIR}/fish: ${SRCOBJS} ${OBJDIR}/librmrfnet.a
	${MKDIR} ${@D} && ${CXX} -o $@ ${LFLAGS} $^ && touch $@

clean:
	rm -rf ${BINDIR}
	rm -rf ${OBJDIR}
	rm -rf ${DEPDIR}
	rm -rf ${MODIR}


