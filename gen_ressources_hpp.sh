#!/bin/sh
set -e

in="7-segment.svg.in"
header="ressources.hpp"
on="FF0000"
off="400000"
n0="abcdef"
n1="bc"
n2="abdeg"
n3="abcdg"
n4="bcfg"
n5="acdfg"
n6="acdefg"
n7="abc"
n8="abcdefg"
n9="abcdfg"

xxd -i icon.png > $header

rsvg-convert dots.svg --height 48 --keep-aspect-ratio --background-color="black" --output dots.png
xxd -i dots.png >> $header
rm dots.png

for n in `seq 0 9`; do
  rm -f $n.svg n${n}.png n${n}s.png
  cp $in $n.svg
  filter=$(eval echo $(echo \$n$n))
  turnOn="$(echo $filter | sed -e 's|.|& |g')"
  turnOff="$(echo abcdefg | sed -e "s|[$filter]||g" -e 's|.|& |g')"
  for x in $turnOn ; do
    sed -i "s|@$x@|$on|" $n.svg
  done
  for x in $turnOff ; do
    sed -i "s|@$x@|$off|" $n.svg
  done
  rsvg-convert $n.svg --height 48 --keep-aspect-ratio --background-color="black" --output n${n}.png
  rsvg-convert $n.svg --height 34 --keep-aspect-ratio --background-color="black" --output n${n}s.png
  xxd -i n${n}.png >> $header
  xxd -i n${n}s.png >> $header
  rm $n.svg n${n}.png n${n}s.png
done

echo 'Fl_PNG_Image *image[] = {' >> $header
for n in `seq 0 9`; do
  echo "  new Fl_PNG_Image(NULL, n${n}_png, (int)n${n}_png_len)," >> $header
done
echo '};' >> $header
echo 'Fl_PNG_Image *image_s[] = {' >> $header
for n in `seq 0 9`; do
  echo "  new Fl_PNG_Image(NULL, n${n}s_png, (int)n${n}s_png_len)," >> $header
done
echo '};' >> $header

echo '#ifdef Fl_Spinner_Mod' >> $header
echo 'static const char *Fl_Spinner_Mod_patch =' >> $header
sed 's|\\|\\\\|g; s|"|\\"|g; s|^|  "|g; s|$|\\n"|g' Fl_Spinner_Mod.patch >> $header
echo ';' >> $header
echo '#endif' >> $header

