#
#   Common parts of GNU Make/GCC build system for multi-targets
#   Copyright (C) 2014 by KO Myung-Hun <komh@chollian.net>
#   All rights reserved.
#   Contact: KO Myung-Hun (komh@chollian.net)
#
#   This file is part of KDllAr
#
#   $BEGIN_LICENSE$
#
#   GNU General Public License Usage
#   This file may be used under the terms of the GNU
#   General Public License version 3.0 as published by the Free Software
#   Foundation and appearing in the file LICENSE.GPL included in the
#   packaging of this file.  Please review the following information to
#   ensure the GNU General Public License version 3.0 requirements will be
#   met: http://www.gnu.org/copyleft/gpl.html.
#
#   $END_LICENSE$
#

# specify gcc compiler flags for all the programs
#CFLAGS := -Wall

# specify g++ compiler flags for all the programs
CXXFLAGS := -Wall

# specify linker flags such as -L option for all the programs
LDFLAGS :=

# specify libraries such as -l option for all the programs
LDLIBS :=

# specify a list of programs without an extension
BIN_PROGRAMS := kdllar

# specify sources for a specific program with program_SRCS. REQUIRED.
# specify various flags for a specific program with program_CFLAGS,
# program_CXXFLAGS, program_LDFLAGS, and program_LDLIBS. OPTIONAL.
kdllar_SRCS     := main.cpp kdllar.cpp kstringv.cpp
kdllar_LDFLAGS  := -Zargs-wild -Zargs-resp

include Makefile.common
