extproc sh

# $1 : base name, $2 : version macro, $3 : header

normal_ver=$(sed -n "s/#define $2.*\"\(.*\)\"/\1/p" $3)
short_ver=$(echo $normal_ver|sed 's/[^0-9]//g')

sed -e "s/@SHORT_VER@/$short_ver/g" -e "s/@VER@/$normal_ver/g" $1.txt > $1$short_ver.txt
mv $1.zip $1$short_ver.zip
