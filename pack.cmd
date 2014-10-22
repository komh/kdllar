@echo off
setlocal

SET SRC=src.zip

gmake clean
gmake RELEASE=1

git export > %SRC%

zip kdllar kdllar.exe README %SRC%
del %SRC%

call name.cmd kdllar KDLLAR_VERSION kdllar.h

endlocal
