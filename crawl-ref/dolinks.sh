#!/bin/sh

mkdir -p NORMAL
mkdir -p WIZARD

pushd NORMAL ; ln -sf ../source/rltiles . ; ln -sf ../source/util . ; ln -sf ../source/*.h ../source/*.cc ../source/makefile* . ; popd
pushd WIZARD ; ln -sf ../source/rltiles . ; ln -sf ../source/util . ; ln -sf ../source/*.h ../source/*.cc ../source/makefile* . ; popd

if [ -d dat ]; then
	true
else
	ln -sf source/dat dat
fi

if [ -d tiles ]; then
	true
else
	ln -sf source/rltiles tiles
	pushd source/dat/tiles ; ln -sf ../../rltiles/*.png . ; popd
fi
