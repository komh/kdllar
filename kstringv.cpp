/****************************************************************************
**
** KStringV, a convenience class for vector< string >
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

#include "KStringV.h"

using namespace std;

KStringV::KStringV KStringV::split( const string& s, char delim )
{
    KStringV::KStringV v;

    size_t pos = 0;
    size_t len = 0;

    while( len != string::npos )
    {
        // skip leading spaces
        while( s[ pos ] == ' ')
            pos++;

        // empty string or trailing spaces ?
        if( !s[ pos ])
            break;

        len = s.find( delim, pos );
        if( len != string::npos )
            len = len - pos;

        v.push_back( s.substr( pos, len ));

        pos += len + 1;
    }

    return v;
}

KStringV::KStringV() : vector< string >()
{
}

KStringV::KStringV( const KStringV& v ) : vector< string >( v )
{
}

KStringV::~KStringV()
{
}

KStringV& KStringV::append( const KStringV& v )
{
    for( KStringV::const_iterator it = v.begin(); it != v.end(); ++it )
        push_back( *it );

    return *this;
}

