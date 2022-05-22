#!/bin/bash

function generate_character() {
	dir="$1"
	name="$2"

	echo "GENERATE CHARACTER $dir $name"

	shift 2

	array=("$@")

	destdir="../../$dir"

	if [ -d "$destdir" ]; then
		echo "Directory '$destdir' exists, skipped"
		return
	fi

	mkdir "$destdir"

	for filename in ../climbdown.png ../sprite.png ../thumbnail.png; do
		newname="$destdir/$(basename $filename)"
	
		convert $filename \
			-fill "${array[0]}" -opaque '#eef68e' \
			-fill "${array[1]}" -opaque '#a7af59' \
			-fill "${array[2]}" -opaque '#6f772a' \
			-fill "${array[3]}" -opaque '#484803' \
			-fill "${array[4]}" -opaque '#171600' \
			"$newname"
	done
	
	for filename in ../operate.png; do
		newname="$destdir/$(basename $filename)"

		convert $filename \
			-fill "${array[0]}" -opaque '#eff78c' \
			-fill "${array[1]}" -opaque '#adb55a' \
			-fill "${array[2]}" -opaque '#727931' \
			-fill "${array[3]}" -opaque '#484800' \
			-fill "${array[4]}" -opaque '#181800' \
			-fill "${array[0]}" -opaque '#e8f088' \
			-fill "${array[1]}" -opaque '#a8b058' \
			-fill "${array[2]}" -opaque '#707828' \
			"$newname"
	done

	cat ../data.json | sed -e "s+\"name\": \"Default\"+\"name\": \"$name\"+" -e "s+\"source\": \"burn.png\"+\"source\": \"../default/burn.png\"+" >"$destdir/data.json" 
}


elite=("#f7eb6f" "#dfaf43" "#a57207" "#5f3702" "#171600")
amadeus=("#959caa" "#50588f" "#303757" "#1f2737" "#171600")
snow=("#cfcfb5" "#a4a595" "#7f7f77" "#575959" "#2f3737")
pink=("#e68191" "#c65875" "#8d3747" "#6e160e" "#160000")
metro=("#768491" "#575959" "#384148" "#2a2d2e" "#0f1717")

generate_character elite "Elite" "${elite[@]}"
generate_character amadeus "Amadeus" "${amadeus[@]}"
generate_character snow "Snow" "${snow[@]}"
generate_character pink "Pink" "${pink[@]}"
generate_character metro "Metro" "${metro[@]}"

exit 0
