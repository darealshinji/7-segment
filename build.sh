#!/bin/sh
set -e
set -x

cd "$(dirname "$0")"

TOP="$PWD"
JOBS=`nproc`
Fl_Spinner_Mod="yes"
DFl_Spinner_Mod="-UFL_SPINNER_MOD"
fltk_CFLAGS="-Wall -O2 -fstack-protector --param=ssp-buffer-size=4 -ffunction-sections -fdata-sections -D_FORTIFY_SOURCE=2"
fltk_CXXFLAGS="$fltk_CFLAGS"
fltk_LDFLAGS="-s -Wl,-z,relro -Wl,--as-needed -Wl,--gc-sections"
fltk_CONFIG="--disable-gl --disable-xdbe --disable-xinerama --disable-xcursor --disable-xfixes --disable-xrender"

test -d fltk-src || git clone --depth 1 "https://github.com/darealshinji/fltk-1.3.4" fltk-src

# patching FLTK/Fl_Spinner
if [ "x$Fl_Spinner_Mod" = "xyes" ]; then
  if [ ! -f fltk-src/FL/Fl_Spinner.H.orig ]; then
    (cd fltk-src && patch --backup -p1 < ../Fl_Spinner_Mod.patch)
  fi
  DFl_Spinner_Mod="-DFL_SPINNER_MOD"
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

xxd -i DSEG7-Classic-Bold-modified.ttf > ressources.h

set +x

"$TOP/fltk/bin/fltk-config" --compile main.cpp | sed "s|-O2|-O3 -DFLTK_STATIC $DFl_Spinner_Mod|g" > stage2
cat stage2 && eval `cat stage2` && rm stage2

set -x

mv -f main simple_timer
ln -sf simple_timer simple_stopwatch
ln -sf simple_timer simple_clock

