/* pack.cmd to package a project using GNU Make/GCC build system and git */

/****** Configuration parts begin ******/

sPackageName = 'kdllar'
sVerMacro = 'KDLLAR_VERSION'
sVerHeader = 'kdllar.h'

sDistFiles = 'kdllar.exe',
             'README'

/****** Configuration parts end ******/

sCmd = 'sed -n "s/^#define' sVerMacro '*\\\"\(.*\)\\\"$/\1/p"' sVerHeader
sVer = getOutput( sCmd )
sShortVer = removeNonNumbers( sVer )

'gmake' 'clean'
'gmake' 'RELEASE=1'

'sed' '-e' 's/@VER@/' || sVer || '/g',
      '-e' 's/@SHORT_VER@/' || sShortVer || '/g',
       sPackageName || '.txt' '>' sPackageName || sShortVer || '.txt'

'git' 'archive' '--format' 'zip' sVer '>' 'src.zip'
'zip' sPackageName || sShortVer || '.zip' sDistFiles 'src.zip'
'rm' '-f' 'src.zip'

exit 0

/* Get outputs from commands */
getOutput: procedure
    parse arg sCmd

    nl = x2c('d') || x2c('a')

    rqNew = rxqueue('create')
    rqOld = rxqueue('set', rqNew )

    address cmd sCmd '| rxqueue' rqNew

    sResult = ''
    do while queued() > 0
        parse pull sLine
        sResult = sResult || sLine || nl
    end

    call rxqueue 'Delete', rqNew
    call rxqueue 'Set', rqOld

    /* Remove empty lines at end */
    do while right( sResult, length( nl )) = nl
        sResult = delstr( sResult, length( sResult ) - length( nl ) + 1 )
    end

    return sResult

/* Remove non-number characters */
removeNonNumbers: procedure
    parse arg sStr

    do i = length( sStr) to 1 by -1
        if datatype( substr( sStr, i, 1 ), 'n') = 0  then
            sStr = delstr( sStr, i, 1 )
    end

    return sStr
