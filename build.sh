#!/bin/sh
set -e
set -x

cd "$(dirname "$0")"

TOP="$PWD"
JOBS=`nproc`
Fl_Spinner_Mod="yes"
DFl_Spinner_Mod="-UFl_Spinner_Mod"
fltk_CFLAGS="-Wall -O2 -fstack-protector --param=ssp-buffer-size=4 -ffunction-sections -fdata-sections -D_FORTIFY_SOURCE=2"
fltk_CXXFLAGS="$fltk_CFLAGS"
fltk_LDFLAGS="-s -Wl,-z,relro -Wl,--as-needed -Wl,--gc-sections"
fltk_CONFIG="--enable-localjpeg --enable-localpng --enable-localzlib --disable-gl --disable-xdbe --disable-xinerama --disable-xcursor --disable-xfixes --disable-xrender"

test -d fltk-src || git clone --depth 1 -b fltk-1.3.4-1-source "https://github.com/darealshinji/fltk-dialog" fltk-src

# patching FLTK/Fl_Spinner
if [ "x$Fl_Spinner_Mod" = "xyes" ]; then
  if [ ! -f fltk-src/FL/Fl_Spinner.H.orig ]; then
    (cd fltk-src && patch --backup -p1 < ../Fl_Spinner_Mod.patch)
  fi
  DFl_Spinner_Mod="-DFl_Spinner_Mod"
fi

# apply zlib patches
if [ ! -f fltk-src/zlib/crc32.c.orig ]; then
  patch -p1 --backup < CVE-2016-9840-9843.patch
fi

# build FLTK
if [ ! -x fltk/bin/fltk-config ]; then
  make -C fltk-src distclean
  (cd fltk-src && CFLAGS="$fltk_CFLAGS" CXXFLAGS="$fltk_CXXFLAGS" LDFLAGS="$fltk_LDFLAGS" \
   ./configure --prefix="$TOP/fltk" $fltk_CONFIG)
  make -j$JOBS -C fltk-src
  make -j1 -C fltk-src install
  rm -f fltk-src/.clean_stamp
fi

test -f ressources.hpp || ./gen_ressources_hpp.sh

set +x

"$TOP/fltk/bin/fltk-config" --use-images --compile main.cpp | sed "s|-O2|-O3 $DFl_Spinner_Mod|g" > stage2
cat stage2 && eval `cat stage2` && rm stage2

set -x

mv -f main simple_timer
ln -sf simple_timer simple_stopwatch
ln -sf simple_timer simple_clock
