/****************************************************************************
**
** KStringV, a convenience class for vector< string >
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

#include <vector>
#include <string>

class KStringV : public std::vector< std::string >
{
public :
    static KStringV split( const std::string& s, char delim = ' ');

    KStringV();
    KStringV( const KStringV& v );
    ~KStringV();

    KStringV& append( const KStringV& v );
};
