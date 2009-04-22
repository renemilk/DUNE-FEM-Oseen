#!/bin/bash

if test $# -lt 1 ; then
  echo "Usage: $0 <header>"
  exit 1
fi

CXX="g++"

WORKINGDIR=`pwd`
cd `dirname $0`
SCRIPTSDIR=`pwd`
cd "$SCRIPTSDIR/.."
FEMDIR=`pwd`
HEADER=$1

if ! test -e $HEADER ; then
  echo "'$HEADER' does not exist."
  exit 1
fi

cd $WORKINGDIR
BASEFILE=`mktemp -p . tmp-header-XXXXXX`
MAKEFILE=$BASEFILE.make
CCFILE=$BASEFILE.cc
OFILE=$BASEFILE.o

echo "GRIDTYPE=ALUGRID_CONFORM" >> $MAKEFILE
echo "GRIDDIM=2" >> $MAKEFILE
echo ".cc.o:" >> $MAKEFILE
echo -e -n "\t$CXX -c -I$FEMDIR" >> $MAKEFILE
echo ' -I/usr/local/diplomarbeit_felix_rene/dune-common -I/usr/local/diplomarbeit_felix_rene/dune-grid -I/usr/local/diplomarbeit_felix_rene/dune-istl   -I/usr/local/diplomarbeit_felix_rene/dune-common -I/usr/local/diplomarbeit_felix_rene/dune-grid -I/usr/local/diplomarbeit_felix_rene/dune-istl -DGRIDDIM=$(GRIDDIM) -D$(GRIDTYPE) -I/usr/local/grape -I/usr/include/X11 -pthread -I/usr/local/alberta-1.2/include -DENABLE_ALBERTA -I/usr/local/alugrid-1.14/include -I/usr/local/alugrid-1.14/include/serial -I/usr/local/alugrid-1.14/include/duneinterface -DENABLE_ALUGRID -g -o $@ $<' >> $MAKEFILE

echo "#include <config.h>" >> $CCFILE
echo "#include <${HEADER}>" >> $CCFILE
echo "#include <${HEADER}>" >> $CCFILE
echo "int main () {}" >> $CCFILE

make -f $MAKEFILE $OFILE 1>/dev/null
SUCCESS=$?

rm -f $OFILE
rm -f $CCFILE
rm -f $MAKEFILE
rm -f $BASEFILE

exit $SUCCESS
