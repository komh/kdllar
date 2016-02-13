/****************************************************************************
**
** KDllAr, DLL generator
** Copyright (C) 2014-2016 by KO Myung-Hun
** All rights reserved.
** Contact: KO Myung-Hun (komh@chollian.net)
**
** This file is part of KDllAr
**
** $BEGIN_LICENSE$
**
** GNU General Public License Usage
** This file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** $END_LICENSE$
**
****************************************************************************/

#ifndef KDLLAR_H
#define KDLLAR_H

#include <string>

#include "kstringv.h"

#define KDLLAR_VERSION  "1.1.0"

class KDllAr
{
public:
    KDllAr( int argc, char *argv[]);
    ~KDllAr();

    int run();

private:
    KStringV    _argv;
    std::string _outputName;
    std::string _description;
    std::string _cc;
    std::string _flags;
    bool        _useOrd;
    std::string _exclude;
    std::string _include;
    std::string _libFlags;
    bool        _useCrtDll;
    std::string _libData;
    bool        _useOmf;
    bool        _useLxlite;
    std::string _defName;
    std::string _implibName;
    bool        _keepDef;
    std::string _symFile;
    bool        _symPrefix;
    std::string _objExt;

    std::string _dllName;
    bool        _defProvided;
    KStringV    _objs;
    KStringV    _gccArgv;

    std::string _includeLibs;
    std::string _excludeLibs;

    int processArg();
    int emxexp();
    int sym2in();
    int gcc();
    int emximp();
    int lxlite();
};
#endif

