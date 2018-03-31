#!/bin/bash
set -e


if [ -x "tools/build/mkcommon.sh" ]; then
	tools/build/mkcommon.sh $@
else
	buildroot/scripts/mkcommon.sh $@
fi

