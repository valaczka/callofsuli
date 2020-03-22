#!/bin/bash

color1=""
color2=""
color3=""
color4=""
color5=""

for filename in _climbdown.png _sprite.png _thumbnail.png; do
	newname=$(echo $filename | sed "s/^_//")

	convert $filename \
		-fill $color1 -opaque '#eef68e' \
		-fill $color2 -opaque '#a7af59' \
		-fill $color3 -opaque '#6f772a' \
		-fill $color4 -opaque '#484803' \
		-fill $color5 -opaque '#171600' \
		$newname
done

convert _operate.png \
		-fill $color1 -opaque '#eff78c' \
		-fill $color2 -opaque '#adb55a' \
		-fill $color3 -opaque '#727931' \
		-fill $color4 -opaque '#484800' \
		-fill $color5 -opaque '#181800' \
	operate.png
