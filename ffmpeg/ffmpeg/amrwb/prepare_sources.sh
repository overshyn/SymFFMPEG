#!/bin/bash

set -o errexit
trap "rm -rf c-code *.c *.h readme.txt *.new *.def *.all" 0

cd "$1"

rm -rf c-code *.c *.h readme.txt
/cygdrive/c/Program Files/Common Files/Symbian/tools/unzip 26204-700_ANSI-C_source_code.zip

for FILE in c-code/readme.txt c-code/*.c  c-code/*.h ; do
    tr -d '\r\000' <$FILE >${FILE#*/}.new
    mv ${FILE#*/}.new ${FILE#*/}
done
echo >>typedef.h

rm -r c-code

/cygdrive/c/job/AppMakr/patch/bin/patch <amrwb-intsizes.patch
/cygdrive/c/job/AppMakr/patch/bin/patch <amrwb-any-cflags.patch
/cygdrive/c/job/AppMakr/patch/bin/patch <amrwb-dhf-declaration.patch

for FILE in dec_if.[ch] enc_if.[ch] ; do
    echo "modifying file $FILE"
    if test $FILE = ${FILE%.h} ; then
	ENDSTRING="^}"
    else
	ENDSTRING=");"
    fi
    sed -n "/^[^# ][^ ]* ._IF_..code/,/$ENDSTRING/p" <$FILE >$FILE.def
    sed 's/^\([^# ][^ ]*\) \(._IF_..code\)/\1 GP3\2/
	 s:ifndef IF2:if 1 /* & */:
	 s:ifdef IF2:if 0 /* & */:
	' <$FILE.def >$FILE.all
    sed 's/^\([^# ][^ ]*\) \(._IF_..code\)/\1 IF2\2/
	 s:ifndef IF2:if 0 /* & */:
	 s:ifdef IF2:if 1 /* & */:
	' <$FILE.def >>$FILE.all
    sed "/^[^# ][^ ]* ._IF_..code/,/$ENDSTRING/c\\
/* Double the code with different defines and names */
" <$FILE |
    sed "/Double the code with different defines and names/r $FILE.all" >$FILE.new
    mv $FILE.new $FILE
    rm $FILE.def $FILE.all
done

trap 0
