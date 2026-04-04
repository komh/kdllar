#
#   Configuration parts of GNU Make/GCC build system.
#   Copyright (C) 2014-2016 by KO Myung-Hun <komh@chollian.net>
#
#   This file is part of GNU Make/GCC build system.
#
#   This program is free software. It comes without any warranty, to
#   the extent permitted by applicable law. You can redistribute it
#   and/or modify it under the terms of the Do What The Fuck You Want
#   To Public License, Version 2, as published by Sam Hocevar. See
#   http://www.wtfpl.net/ for more details.
#

##### Configuration parts that you can modify

# specify sub directories
SUBDIRS :=

# specify gcc compiler flags for all the programs
CFLAGS := -Wall

# specify g++ compiler flags for all the programs
CXXFLAGS := -Wall

# specify linker flags such as -L option for all the programs
LDFLAGS :=

# specify dependent libraries such as -l option for all the programs
LDLIBS :=

ifdef RELEASE
# specify flags for release mode
CFLAGS   += -O3
CXXFLAGS += -O3
LDFLAGS  +=
else
# specify flags for debug mode
CFLAGS   += -O0 -g -DDEBUG
CXXFLAGS += -O0 -g -DDEBUG
LDFLAGS  += -g
endif

# specify resource compiler, default is rc if not set
RC :=

# specify resource compiler flags
RCFLAGS :=

# 1. specify a list of programs without an extension with
#
#   BIN_PROGRAMS
#
# Now, assume
#
#   BIN_PROGRAMS := program
#
# 2. specify sources for a specific program with
#
#   program_SRCS
#
# the above is REQUIRED
#
# 3. specify various OPTIONAL flags for a specific program with
#
#   program_CFLAGS      for gcc compiler flags
#   program_CXXFLAGS    for g++ compiler flags
#   program_LDFLAGS     for linker flags
#   program_LDLIBS      for dependent libraries
#   program_RCSRC       for rc source
#   program_RCFLAGS     for rc flags
#   program_DEF         for .def file
#   program_EXTRADEPS   for extra dependencies

BIN_PROGRAMS := kdllar

kdllar_SRCS     := main.cpp kdllar.cpp kstringv.cpp
ifeq ($(OS2_SHELL),)
kdllar_CFLAGS   := -DHAVE_STDLIB_H -DHAVE_STRING_H -I. -Ilibiberty
kdllar_CXXFLAGS := -DHAVE_STDLIB_H -DHAVE_STRING_H -DHAVE_DECL_BASENAME=1 \
                   -I. -Ilibiberty
kdllar_SRCS     += libiberty/argv.c libiberty/xmalloc.c libiberty/xexit.c \
                   libiberty/xstrdup.c libiberty/safe-ctype.c
endif

# 1. specify a list of libraries without an extension with
#
#   BIN_LIBRARIES
#
# Now, assume
#
#   BIN_PROGRAMS := library
#
# 2. specify sources for a specific library with
#
#   library_SRCS
#
# the above is REQUIRED
#
# 3. set library type flags for a specific library to a non-empty value
#
#   library_LIB         to create a static library
#   library_DLL         to build a DLL
#
# either of the above SHOULD be SET
#
# 4. specify various OPTIONAL flags for a specific library with
#
#   library_CFLAGS      for gcc compiler flags
#   library_CXXFLAGS    for g++ compiler flags
#
# the above is common for LIB and DLL
#
#   library_DLLNAME     for customized DLL name without an extension
#   library_LDFLAGS     for linker flags
#   library_LDLIBS      for dependent libraries
#   library_RCSRC       for rc source
#   library_RCFLAGS     for rc flags
#   library_DEF         for .def file, if not set all the symbols are exported
#   library_EXTRADEPS   for extra dependencies
#
# the above is only for DLL

BIN_LIBRARIES :=

include Makefile.common

# additional stuffs

CLEAN-FILES += kld$(EXE_EXT)

ifneq ($(OS2_SHELL),)
LN_S := ln4exe -s
else
LN_S := ln -s
endif

all: kld$(EXE_EXT)

kld$(EXE_EXT) : kdllar$(EXE_EXT)
	$(QUIET)$(LN_S) kdllar$(EXE_EXT) $@
