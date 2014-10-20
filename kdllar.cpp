/****************************************************************************
**
** KDllAr, DLL generator
** Copyright (C) 2014 by KO Myung-Hun
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

#include "kdllar.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <io.h>
#include <process.h>
#include <fnmatch.h>
#include <sys/types.h>
#include <sys/wait.h>

using namespace std;

static inline string getname( const string &filename )
{
    string name( filename );

    size_t pos = name.rfind('.');

    if( pos != string::npos && pos != 0 )
        name.erase( pos );

    return name;
}

static inline string getext( const string &filename )
{
    size_t pos = filename.rfind('.');

    if( pos == string::npos || pos == 0 )
        return string();

    return filename.substr( pos );
}

static inline int stricmp( const string& s1, const string& s2 )
{
    return ::stricmp( s1.c_str(), s2.c_str());
}

static inline bool isExcluded( const string& name,
                               const KStringV& exclude )
{
    for( KStringV::const_iterator it = exclude.begin();
         it != exclude.end(); ++it )
    {
        string pattern;

        pattern += "*\"";
        pattern += *it;
        pattern += "\"*";

        if( !fnmatch( pattern.c_str(), name.c_str(), _FNM_POSIX ))
            return true;
    }

    return false;
}

static int execute( const KStringV& argv, int mode = P_WAIT,
                    bool useResponse = true, string* rspName = 0 )
{
    for( KStringV::const_iterator it = argv.begin(); it != argv.end();
         ++it )
        cerr << *it << " ";
    cerr << endl;

    if( useResponse )
    {
        string rspTemp( argv[ 0 ]);
        rspTemp += ".rsp";

        ofstream ofs;

        ofs.open( rspTemp.c_str());
        if( !ofs.is_open())
        {
            cerr << "Failed to create a response file, " << rspTemp << endl;

            return -1;
        }

        for( KStringV::const_iterator it = argv.begin() + 1; it != argv.end();
            ++it )
            ofs << *it << endl;

        ofs.close();

        string rspArg( string("@") + rspTemp );

        char* spawn_argv[] = { const_cast< char* >( argv[ 0 ].c_str()),
                               const_cast< char* >( rspArg.c_str()),
                               0 };

        int rc = spawnvp( mode, spawn_argv[ 0 ], spawn_argv );

        if( rc == -1 || !rspName )
            remove( rspTemp.c_str());
        else
            *rspName = rspTemp;

        return rc;
    }

    char** spawn_argv = new ( char* )[ argv.size() + 1 ];

    for( size_t i = 0; i < argv.size(); i++ )
        spawn_argv[ i ] = const_cast< char* >( argv[ i ].c_str());

    spawn_argv[ argv.size()] = 0;

    int rc = spawnvp( mode, spawn_argv[ 0 ], spawn_argv );

    delete[] spawn_argv;

    return rc;
}

static void usage()
{
    static const char *msg = "\
kdllar: no input files\n\
Usage: kdllar [-o[utput] output_file] [-d[escription] \"dll descrption\"]\n\
       [-cc \"CC\"] [-f[lags] \"CFLAGS\"] [-ord[inals]] -ex[clude] \"symbol(s)\"\n\
       [-libf[lags] \"{INIT|TERM}{GLOBAL|INSTANCE}\"] [-nocrt[dll]]\n\
       [-libd[ata] \"DATA\"] [-omf] [-nolxlite] [-def def_file] [*.o] [*.a]\n\
*> \"output_file\" should have no extension.\n\
   If it has the .o, .a or .dll extension, it is automatically removed.\n\
   The import library name is derived from this and is set to \"name\"_dll.a.\n\
*> \"cc\" is used to use another GCC executable.   (default: gcc.exe)\n\
*> \"flags\" should be any set of valid GCC flags. (default: -Zcrtdll)\n\
   These flags will be put at the start of GCC command line.\n\
*> -ord[inals] tells kdllar to export entries by ordinals. Be careful.\n\
*> -ex[clude] defines symbols which will not be exported. You can define\n\
   multiple symbols, for example -ex \"myfunc yourfunc _GLOBAL*\".\n\
   If the last character of a symbol is \"*\", all symbols beginning\n\
   with the prefix before \"*\" will be exclude, (see _GLOBAL* above).\n\
*> -libf[lags] can be used to add INITGLOBAL/INITINSTANCE and/or\n\
   TERMGLOBAL/TERMINSTANCE flags to the dynamically-linked library.\n\
   (default: INITINSTANCE TERMINSTANCE)\n\
*> -libd[ata] can be used to add data segment attributes flags to the\n\
   dynamically-linked library. (default: MULTIPLE NONSHARED)\n\
*> -nocrtdll switch will disable linking the library against emx's\n\
   C runtime DLLs.\n\
*> -nolxlite does not compress executable\n\
*> -def def_file do not generate .def file, use def_file instead.\n\
*> -omf will use OMF tools to extract the static library objects. deprecated.\n\
*> All other switches (for example -L./ or -lmylib) will be passed\n\
   unchanged to GCC at the end of command line.\n\
*> If you create a DLL from a library and you do not specify -o,\n\
   the basename for DLL will be set to library name, and import library \n\
   will have _dll suffix. i.e. \"kdllar gcc.a\" will create gcc.dll\n\
   and gcc_dll.a.\n\
*> If a DLL name is longer than 8 characters, it will be truncated up to\n\
   8 characters. This is a limitaiton of OS/2. But an import library name\n\
   is not truncated.\n\
--------\n\
Example:\n\
   kdllar -o gcc290.dll libgcc.a -d \"GNU C runtime library\" -ord\n\
          -ex \"__main __ctordtor*\" -libf \"INITINSTANCE TERMINSTANCE\"\n\
";

    printf("%s", msg );
}

KDllAr::KDllAr( int argc, char* argv[])
        : _useOrd( false )
        , _useCrtDll( true )
        , _useOmf( false )
        , _useLxlite( true )
        , _defProvided( false )
{
    int i;

    for( i = 0; i < argc; i++ )
        _argv.push_back( argv[ i ]);
}

KDllAr::~KDllAr()
{
}

int KDllAr::processArg()
{
    size_t i;

    for( i = 1; i < _argv.size(); i++ )
    {
        const string& arg( _argv.at( i ));

        if( !arg.compare("-o") ||
            !arg.compare("-output"))
        {
            if( i + 1 < _argv.size())
            {
                i++;
                _outputName = getname( _argv[ i ]);
            }
        }
        else if( !arg.compare("-d") ||
                 !arg.compare("-description"))
        {
            if( i + 1 < _argv.size())
            {
                i++;
                _description =  _argv[ i ];
            }
        }
        else if( !arg.compare("-cc"))
        {
            if( i + 1 < _argv.size())
            {
                i++;
                _cc =  _argv[ i ];
            }
        }
        else if( !arg.compare("-f") ||
                 !arg.compare("-flags"))
        {
            if( i + 1 < _argv.size())
            {
                i++;
                _flags =  _argv[ i ];
            }
        }
        else if( !arg.compare("-ord") ||
                 !arg.compare("-ordinals"))
        {
            _useOrd = true;
        }
        else if( !arg.compare("-ex") ||
                 !arg.compare("-exclude"))
        {
            if( i + 1 < _argv.size())
            {
                i++;
                _exclude =  _argv[ i ];
            }
        }
        else if( !arg.compare("-libf") ||
                 !arg.compare("-libflags"))
        {
            if( i + 1 < _argv.size())
            {
                i++;
                _libFlags =  _argv[ i ];
            }
        }
        else if( !arg.compare("-nocrt") ||
                 !arg.compare("-nocrtdll"))
        {
            _useCrtDll = false;
        }
        else if( !arg.compare("-libd") ||
                 !arg.compare("-libdata"))
        {
            if( i + 1 < _argv.size())
            {
                i++;
                _libData =  _argv[ i ];
            }
        }
        else if( !arg.compare("-omf"))
        {
            _useOmf = true;
        }
        else if( !arg.compare("-nolxlite"))
        {
            _useLxlite = false;
        }
        else if( !arg.compare("-def"))
        {
            if( i + 1 < _argv.size())
            {
                i++;
                _defName =  _argv[ i ];

                _defProvided = true;
            }
        }
        else
        {
            string ext( getext( arg ));

            if( !stricmp( ext, ".o") || !stricmp( ext, ".obj")
                || !stricmp( ext, ".a") || !stricmp( ext, ".lib"))
            {
                if( _outputName.empty())
                    _outputName = getname( arg );

                _objs.push_back( arg );
            }

            _gcc_argv.push_back( arg );
        }
    }

    if( _argv.size() == 1 || _objs.size() == 0 )
    {
        usage();

        return -1;
    }

    if( _cc.empty())
        _cc = "gcc";

    if( _useCrtDll )
        _flags += "-Zcrtdll";

    if( _libFlags.empty())
        _libFlags = "INITINSTANCE TERMINSTANCE";

    if( _libData.empty())
        _libData = "MULTIPLE NONSHARED";

    if( _defName.empty())
        _defName = _outputName + ".def";

    if( _implibName.empty())
        _implibName = _outputName + "_dll.a";

    _dllName = _outputName.substr( 0, 8 ) + ".dll";

    return 0;
}

int KDllAr::run()
{
    if( processArg())
        return -1;

    if( emxexp())
        return -1;

    if( gcc())
        return -1;

    if( emximp())
        return -1;

    if( lxlite())
        return -1;

    return 0;
}

static const int STDOUT_FILE_NO = 1;

int KDllAr::emxexp()
{
    if( _defProvided )
        return 0;

    int fd[ 2 ];

    if( pipe( fd ))
    {
        perror("pipe");

        return -1;
    }

    int oldStdOut = dup( STDOUT_FILE_NO );

    dup2( fd[ 1 ], STDOUT_FILE_NO );
    close( fd[ 1 ]);

    KStringV argv;

    argv.push_back("emxexp");

    if( _useOrd )
        argv.push_back("-o");

    argv.append( _objs );

    string rspName;

    int rc = execute( argv, P_NOWAIT, true, &rspName );

    dup2( oldStdOut, STDOUT_FILE_NO );
    close( oldStdOut );

    if( rc != -1 )
    {
        stringstream ss;

        ss << "LIBRARY " << getname( _dllName ) << " " << _libFlags << endl;

        if( !_description.empty())
            ss << "DESCRIPTION \"" << _description << "\"" << endl;

        ss << "DATA " << _libData << endl;
        ss << "EXPORTS" << endl;

        KStringV exclude( KStringV::split( _exclude ));

        FILE* fp = fdopen( fd[ 0 ], "rt");
        char line[ 512 ];

        while( fgets( line, sizeof( line ), fp ))
        {
            if( !isExcluded( line, exclude ))
                ss << line;
        }

        fclose( fp );

        int stat_val;

        if( waitpid( rc, &stat_val, 0 )  == rc
            && WIFEXITED( stat_val ) && WEXITSTATUS( stat_val ) == 0 )
        {
            ofstream ofs;

            ofs.open( _defName.c_str());

            if( ofs.is_open())
            {
                ofs << ss.str();

                ofs.close();

                rc = 0;
            }
            else
            {
                cerr << "Failed to create a def file, " << _defName << endl;

                rc = -1;
            }
        }

        remove( rspName.c_str());
    }
    else
        cerr << "Failed to spawn " << argv[ 0 ] << endl;

    close( fd[ 0 ]);

    return rc;
}

int KDllAr::gcc()
{
    KStringV argv;

    argv.push_back( _cc );

    _flags += " -Zdll";

    argv.append( KStringV::split( _flags ));

    argv.push_back("-o");
    argv.push_back( _dllName );

    argv.append( _gcc_argv );

    argv.push_back( _defName );

    return execute( argv );
}

int KDllAr::emximp()
{
    KStringV argv;

    argv.push_back("emximp");
    argv.push_back("-o");
    argv.push_back( _implibName );
    argv.push_back( _defName );

    return execute( argv );
}

int KDllAr::lxlite()
{
    if( !_useLxlite )
        return 0;

    KStringV argv;

    argv.push_back("lxlite");
    argv.push_back("-cs");
    argv.push_back("-t:");
    argv.push_back("-mrn");
    argv.push_back("-ml1");
    argv.push_back( _dllName );

    return execute( argv, P_WAIT, false );
}
