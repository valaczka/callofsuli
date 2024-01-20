#!/bin/bash

### Inkscape: Edit XML -> Delete <def>

json=data.json

echo "{" >$json
echo "	\"states\": {" >>$json

_first=1

for f in state*.svg; do
	echo $f
	
	id=$(echo $f | sed 's/state-\([0-9]*\).svg/\1/')
	pos=$(cat $f | grep "transform=" | sed 's/^.*transform="translate(\([0-9\.,\-]*\))".*$/\1/g')
	x=$(echo $pos | cut -d, -f 1)
	y=$(echo $pos | cut -d, -f 2)

	if [ "${x:0:1}" = "-" ]; then
		x=${x:1}
	else
		x=-$x
	fi

	if [ "${y:0:1}" = "-" ]; then
		y=${y:1}
	else
		y=-$y
	fi

	if [ $_first -eq 0 ]; then
		echo "," >>$json
	else
		_first=0
	fi

	echo -n "		\"$id\": { \"x\": $x, \"y\": $y }" >>$json

	border=$(echo $f | sed 's/\.svg$/-border.svg/')
	cat $f | sed -e 's/stroke="[^"]*"//g' -e 's/style="fill[^"]*"/style="fill:none;stroke:#808080;stroke-width:10;stroke-dasharray:none"/g' >$border
	sed -e 's/stroke="[^"]*"//g' -i $f
done
	
echo "" >>$json
echo "	}" >>$json
echo "}" >>$json


cat $json

exit 0
