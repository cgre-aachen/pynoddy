#!/bin/bash

touch malloc.h

     for fl in *.c; do
     mv $fl $fl.old
     LC_CTYPE=C sed 's/<malloc.h>/"malloc.h"/g' $fl.old > $fl
     rm -f $fl.old

     done
     for fl in *.h; do
     mv $fl $fl.old
     LC_CTYPE=C sed 's/<malloc.h>/"malloc.h"/g' $fl.old > $fl
     rm -f $fl.old
     done

     for fl in dxf*; do
     mv $fl $fl.old
     LC_CTYPE=C sed 's/stricmp/strcmp/g' $fl.old > $fl
     rm -f $fl.old
     done

