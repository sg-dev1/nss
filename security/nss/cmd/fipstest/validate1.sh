#!/bin/sh
TESTDIR=${1-.}
request=${2}
extraneous_response=${3}
extraneous_fax=${4}
name=`basename $request .req`
echo ">>>>>  $name"
sed -e 's;
sed -e 's;
diff -w -B /tmp/y1 /tmp/y2