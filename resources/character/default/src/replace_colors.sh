#!/bin/bash

if [ "$1" = "" ]; then
	echo "Fájlnév?"
	exit 1
fi

outfile=$2

if [ "$outfile" = "" ]; then
	outfile=$1_out.png
fi

color1=""
color2=""
color3=""
color4=""
color5=""

convert $1 \
	-fill $color1 -opaque '#eef68e' \
	-fill $color2 -opaque '#a7af59' \
	-fill $color3 -opaque '#6f772a' \
	-fill $color4 -opaque '#484803' \
	-fill $color5 -opaque '#171600' \
	$outfile
