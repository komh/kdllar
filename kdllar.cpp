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

#include <ar.h>
#include <a_out.h>

using namespace std;

static inline size_t getLastDirSepPos( const string& filename )
{
    return filename.find_last_of("/\\:");
}

static inline size_t getLastExtDotPos( const string& filename )
{
    size_t lastDirSepPos = getLastDirSepPos( filename );
    size_t pos = filename.rfind('.');

    if( lastDirSepPos == string::npos || pos == string::npos ||
        pos > lastDirSepPos + 1 /* not a first character of a name */ )
        return pos;

    return string::npos;
}

static inline string getName( const string &filename )
{
    return filename.substr( 0, getLastExtDotPos( filename ));
}

static inline string getDir( const string& filename )
{
    size_t pos = getLastDirSepPos( filename );
    if( pos == string::npos )
        return string();

    return filename.substr( 0, pos + 1 );
}

static inline string getFName( const string &filename )
{
    size_t pos = getLastDirSepPos( filename );
    pos = ( pos == string::npos ) ? 0 : ( pos + 1 );

    size_t len = getLastExtDotPos( filename );
    if( len != string::npos )
        len -= pos;

    return filename.substr( pos, len );
}

static inline string getExt( const string &filename )
{
    size_t pos = getLastExtDotPos( filename );

    if( pos == string::npos )
        return string();

    return filename.substr( pos );
}

static inline int stricmp( const string& s1, const string& s2 )
{
    return ::stricmp( s1.c_str(), s2.c_str());
}

static inline bool isExcluded( const string& name,
                               const KStringV& exclude,
                               bool symbol = true )
{
    for( KStringV::const_iterator it = exclude.begin();
         it != exclude.end(); ++it )
    {
        string pattern;

        if (symbol)
            pattern += "*\"";

        pattern += *it;

        if (symbol)
            pattern += "\"*";

        if( !fnmatch( pattern.c_str(), name.c_str(), _FNM_POSIX ))
            return true;
    }

    return false;
}

static inline bool isIncluded( const string& name,
                               const KStringV& include,
                               bool symbol = true )
{
    if( include.size() == 0 )
        return true;

    for( KStringV::const_iterator it = include.begin();
         it != include.end(); ++it )
    {
        string pattern;

        if (symbol)
            pattern += "*\"";

        pattern += *it;

        if (symbol)
            pattern += "\"*";

        if( !fnmatch( pattern.c_str(), name.c_str(), _FNM_POSIX ))
            return true;
    }

    return false;
}

static int execute( const KStringV& argv, const KStringV& rspArgv = KStringV(),
                    int mode = P_WAIT, string* rspName = 0 )
{
    for( KStringV::const_iterator it = argv.begin(); it != argv.end();
         ++it )
        cerr << *it << " ";

    for( KStringV::const_iterator it = rspArgv.begin(); it != rspArgv.end();
         ++it )
        cerr << *it << " ";

    cerr << endl;

    vector< char * > spawn_argv;

    for( KStringV::const_iterator it = argv.begin(); it != argv.end(); ++it )
        spawn_argv.push_back( const_cast< char * >(( *it ).c_str()));

    string rspTemp( getFName( argv[ 0 ]));
    rspTemp += ".rsp";

    string rspArg("@");

    if( rspArgv.size() > 0 )
    {
        ofstream ofs;

        ofs.open( rspTemp.c_str());
        if( !ofs.is_open())
        {
            cerr << "Failed to create a response file, " << rspTemp << endl;

            return -1;
        }

        for( KStringV::const_iterator it = rspArgv.begin();
             it != rspArgv.end(); ++it )
            ofs << *it << endl;

        ofs.close();

        rspArg += rspTemp;

        spawn_argv.push_back( const_cast< char * >( rspArg.c_str()));
    }

    spawn_argv.push_back( 0 );

    int rc = spawnvp( mode, spawn_argv[ 0 ], &spawn_argv[0]);

    if( rspArgv.size() > 0 )
    {
        if( rc == -1 || !rspName )
            remove( rspTemp.c_str());
        else
            *rspName = rspTemp;
    }

    return rc;
}

static bool isObject( const string& name, const KStringV& objExt,
                      const KStringV& includeLibs,
                      const KStringV& excludeLibs )
{
    string ext( getExt( name ));

    for( KStringV::const_iterator it = objExt.begin(); it != objExt.end();
         ++it )
    {
        if( !stricmp( ext, *it ))
            return (stricmp( ext, ".a") && stricmp( ext, ".lib")) ||
                   ( isIncluded( name, includeLibs, false ) &&
                     !isExcluded( name, excludeLibs, false ));
    }

    return false;
}

static void usage()
{
    static const char *msg = "\
K DLL Archiver v" KDLLAR_VERSION " Copyright (C) 2014-2016 KO Myung-Hun\n\
Usage: kdllar [-o[utput] output_file] [-d[escription] \"dll descrption\"]\n\
       [-cc \"CC\"] [-f[lags] \"CFLAGS\"] [-ord[inals]] [-ex[clude] \"symbol(s)\"]\n\
       [-in[clude] \"symbol(s)\"] [-libf[lags] \"{INIT|TERM}{GLOBAL|INSTANCE}\"]\n\
       [-nocrt[dll]] [-libd[ata] \"DATA\"] [-omf] [-nolxlite] [-def def_file]\n\
       [-nokeepdef] [-implib implib_file] [-symfile \"symbol files\"]\n\
       [-symprefix] [-objext \"obj_extension(s)\"]\n\
       [-ex[clude]libs \"lib(s)\"] [-in[clude]libs \"lib(s)\"]\n\
       [-noexport] [*.o] [*.a]\n\
*> \"output_file\" should have no extension.\n\
   If it has the .o, .a or .dll extension, it is automatically removed.\n\
   The import library name is derived from this and is set to \"name\"_dll.a\n\
   unless -implib is used.\n\
*> \"cc\" is used to use another GCC executable.   (default: gcc.exe)\n\
*> \"flags\" should be any set of valid GCC flags. (default: -Zcrtdll)\n\
   These flags will be put at the start of GCC command line.\n\
*> -ord[inals] tells kdllar to export entries by ordinals. Be careful.\n\
*> -ex[clude] defines symbols which will not be exported. You can define\n\
   multiple symbols, for example -ex \"myfunc yourfunc _GLOBAL*\".\n\
   If the last character of a symbol is \"*\", all symbols beginning\n\
   with the prefix before \"*\" will be exclude, (see _GLOBAL* above).\n\
*> -in[clude] defines symbols which will be exported. You can define\n\
   multiple symbols. for example -in \"myfunc yourfunc _GLOBAL*\".\n\
   If the last character of a symbol is \"*\", all symbols beginning\n\
   with the prefix before \"*\" will be included, (see _GLOBAL* above).\n\
   If the same symbols are specified by -ex as well, they will be excluded.\n\
*> -libf[lags] can be used to add INITGLOBAL/INITINSTANCE and/or\n\
   TERMGLOBAL/TERMINSTANCE flags to the dynamically-linked library.\n\
   (default: INITINSTANCE TERMINSTANCE)\n\
*> -libd[ata] can be used to add data segment attributes flags to the\n\
   dynamically-linked library. (default: MULTIPLE NONSHARED)\n\
*> -nocrtdll switch will disable linking the library against emx's\n\
   C runtime DLLs.\n\
*> -nolxlite does not compress executable\n\
*> -def def_file do not generate .def file, use def_file instead.\n\
*> -nokeepdef do not keep generated .def file.\n\
*> -omf will put -Zomf at the start of GCC command line. This is always used.\n\
*> -implib implib_file will create an import library named \"implib_file\"\n\
   instead of \"name\"_dll.a. \"implib_file\" should have .a or .lib\n\
   extension.\n\
*> -symfile uses \"symbol_files\" separated by a space to create a .def file.\n\
   A symbol file should contain symbols only.\n\
*> -symprefix prepends an underline to each symbol name. This applies only to\n\
   the symbols in the symbol files specified by -symfile.\n\
*> -objext specifies additional object extensions. A leading dot is needed.\n\
   (default: .o, .obj, .a, .lib)\n\
*> -ex[clude]libs specifies libraries not to export symbols. Wildcards(*,?)\n\
   are supported.\n\
*> -in[clude]libs specifies libraries to export symbols. Wildcards(*,?) are\n\
   supported. If the same libraries are specified by -exlibs, they will not\n\
   be exported.\n\
*> -noexport do not export any symbols via .def file. This is useful if all\n\
   symbols are exported in sources via keywords such as\n\
   __declspec(dllexport). This is equivalent to -ex \"*\".\n\
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
        , _useOmf( true )
        , _useLxlite( true )
        , _keepDef( true )
        , _symPrefix( false )
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
                _outputName = getName( _argv[ i ]);
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
                _exclude += " " + _argv[ i ];
            }
        }
        else if( !arg.compare("-in") ||
                 !arg.compare("-include"))
        {
            if( i + 1 < _argv.size())
            {
                i++;
                _include += " " + _argv[ i ];
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
        else if( !arg.compare("-implib"))
        {
            if( i + 1 < _argv.size())
            {
                i++;
                _implibName =  _argv[ i ];

                if( getExt( _implibName ).empty())
                    _implibName += ".a";
            }
        }
        else if( !arg.compare("-nokeepdef"))
        {
            _keepDef = false;
        }
        else if( !arg.compare("-symfile"))
        {
            if( i + 1 < _argv.size())
            {
                i++;
                _symFile += " " + _argv[ i ];
            }
        }
        else if( !arg.compare("-symprefix"))
        {
            _symPrefix = true;
        }
        else if( !arg.compare("-objext"))
        {
            if( i + 1 < _argv.size())
            {
                i++;
                _objExt += " " + _argv[ i ];
            }
        }
        else if( !arg.compare("-inlibs") ||
                 !arg.compare("-includelibs"))
        {
            if( i + 1 < _argv.size())
            {
                i++;
                _includeLibs += " " + _argv[ i ];
            }
        }
        else if( !arg.compare("-exlibs") ||
                 !arg.compare("-excludelibs"))
        {
            if( i + 1 < _argv.size())
            {
                i++;
                _excludeLibs += " " + _argv[ i ];
            }
        }
        else if( !arg.compare("-noexport"))
        {
            _exclude += " *";
        }
        else
        {
            _gccArgv.push_back( arg );
        }
    }

    _objExt += " .o .obj .a .lib";

    KStringV objExt( KStringV::split( _objExt ));

    KStringV includeLibs( KStringV::split( _includeLibs ));
    KStringV excludeLibs( KStringV::split( _excludeLibs ));

    for( KStringV::iterator it = _gccArgv.begin();
         it != _gccArgv.end(); ++it )
    {
        if(( *it )[ 0 ] != '-' &&
           isObject(( *it ), objExt, includeLibs, excludeLibs ))
        {
            if( _outputName.empty())
                _outputName = getName( *it );

            if( emxomf( &( *it )) == -1 )
                return -1;

            _objs.push_back( *it );
        }
    }

    if( _objs.size() == 0 )
    {
        usage();

        cerr << "Error: no input files" << endl;

        return -1;
    }

    if( _cc.empty())
        _cc = "gcc";

    if( _useCrtDll )
        _flags += " -Zcrtdll";

    if( _useOmf )
        _flags += " -Zomf";

    _exclude += " _DLL_InitTerm";

    if( _libFlags.empty())
        _libFlags = "INITINSTANCE TERMINSTANCE";

    if( !stricmp( _libData.substr( 0, 5 ), "DATA "))
        _libData.erase( 0, 5 );

    if( _libData.empty())
        _libData = "MULTIPLE NONSHARED";

    if( _defName.empty())
        _defName = _outputName + ".def";

    if( _implibName.empty())
        _implibName = _outputName + "_dll.a";

    _dllName = getDir( _outputName ) + getFName( _outputName ).substr( 0, 8 )
               + ".dll";

    if( !_defProvided && !_keepDef )
        _tempFiles.push_back( _defName );

    return 0;
}

int KDllAr::run()
{
    if( processArg())
        return -1;

    if( !_defProvided && ( sym2in() || emxexp()))
        return -1;

    if( gcc())
        return -1;

    if( emximp())
        return -1;

    if( lxlite())
        return -1;

    if( removeTempFiles())
        return -1;

    return 0;
}

int KDllAr::emxomf( string *obj )
{
    union AoutHdr
    {
        ar_hdr arHdr;
        exec   aoutHdr;
    } hdr;

    ifstream ifs;

    ifs.open( obj->c_str(), ifs.in | ifs.binary );
    if( !ifs.is_open())
    {
        cerr << "Failed to open " << *obj << endl;

        return -1;
    }

    ifs.read( reinterpret_cast< char * >( &hdr ), sizeof( hdr ));
    if( !ifs.good() && !ifs.eof())
    {
        cerr << "Failed to read " << *obj << endl;

        return -1;
    }

    string omfExt;
    if( memcmp( hdr.arHdr.ar_name, ARMAG, SARMAG ) == 0 )
        omfExt = ".lib";
    else if( N_MAGIC( hdr.aoutHdr ) == OMAGIC )
        omfExt = ".obj";

    if( !omfExt.empty())
    {
        KStringV argv;

        argv.push_back("emxomf");
        argv.push_back("-o");
        argv.push_back( *obj + omfExt );
        argv.push_back( *obj );
        if( execute( argv ) == -1 )
            return -1;

        *obj += omfExt;

        _tempFiles.push_back( *obj );
    }

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

    string rspName;

    int rc = execute( argv, _objs, P_NOWAIT, &rspName );

    dup2( oldStdOut, STDOUT_FILE_NO );
    close( oldStdOut );

    if( rc != -1 )
    {
        stringstream ss;

        ss << "LIBRARY " << getFName( _dllName ) << " " << _libFlags << endl;

        if( !_description.empty())
            ss << "DESCRIPTION \"" << _description << "\"" << endl;

        ss << "DATA " << _libData << endl;
        ss << "EXPORTS" << endl;

        KStringV include( KStringV::split( _include ));
        KStringV exclude( KStringV::split( _exclude ));

        FILE* fp = fdopen( fd[ 0 ], "rt");
        char line[ 512 ];

        while( fgets( line, sizeof( line ), fp ))
        {
            if( isIncluded( line, include ) && !isExcluded( line, exclude ))
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

int KDllAr::sym2in()
{
    ifstream ifs;
    string line;

    KStringV symFile( KStringV::split( _symFile ));

    for( KStringV::const_iterator it = symFile.begin(); it != symFile.end();
         ++it )
    {
        ifs.open(( *it ).c_str());
        if( !ifs.is_open())
        {
            cerr << "Failed to open a symbol file, " << *it << endl;

            return -1;
        }

        while( !ifs.eof())
        {
            getline( ifs, line );

            // skip white spaces
            while( line[ 0 ] == ' ' || line[ 0 ] == '\t')
                line.erase( 0, 1 );

            // remove EXPORTS and blank lines
            if( line.compare("EXPORTS" ) && line.length() > 0 )
            {
                _include += " ";

                if( _symPrefix )
                    _include += "_";

                _include += line;
            }
        }

        ifs.close();
        ifs.clear();
    }

    return 0;
}

int KDllAr::gcc()
{
    KStringV argv;

    argv.push_back( _cc );

    _flags += " -Zdll";

    argv.append( KStringV::split( _flags ));

    char *ldType = getenv("EMXOMFLD_TYPE");
    if( ldType && !stricmp( ldType, "WLINK"))
    {
        argv.push_back("-Zlinker");
        argv.push_back("DISABLE");

        argv.push_back("-Zlinker");
        argv.push_back("1121");
    }

    argv.push_back("-o");
    argv.push_back( _dllName );
    argv.push_back( _defName );

    return execute( argv, _gccArgv );
}

int KDllAr::emximp()
{
    KStringV argv;

    argv.push_back("emximp");
    argv.push_back("-o");
    argv.push_back( _implibName );
    argv.push_back( _dllName );

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

    return execute( argv );
}

int KDllAr::removeTempFiles()
{
    for( KStringV::const_iterator it = _tempFiles.begin();
         it != _tempFiles.end(); ++it )
        remove(( *it ).c_str());

    return 0;
}
