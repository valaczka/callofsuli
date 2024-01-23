#!/bin/bash

### Inkscape: Edit XML -> Delete <def>

json=../data.json

echo "{" >$json

origJson=orig.json

if [ -f $origJson ]; then
	echo -n "	\"orig\": " >>$json
	cat $origJson | sed 's/^/\t/' >>$json	
	echo "	," >>$json
fi

_pFirst=1

for p in player*; do
	echo "Generate $p..."

	if [ $_pFirst -eq 0 ]; then
		echo "	," >>$json
	else
		_pFirst=0
	fi

	echo "	\"$p\": {" >>$json
	echo "		\"lands\": {" >>$json

	_first=1

	for f in $p/land-*.svg; do
		echo "   - $f"

		bname=$(basename $f)
		
		id=$(echo $bname | sed 's/land-\([0-9]*\).svg/\1/')
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
	
		echo -n "			\"$id\": { \"x\": $x, \"y\": $y }" >>$json
	
		border=$(echo $bname | sed 's/\.svg$/-border.svg/')
		cat $f | sed -e 's/stroke="[^"]*"//g' -e 's/style="fill[^"]*"/style="fill:none;stroke:#808080;stroke-width:10;stroke-dasharray:none"/g' >../$p-$border
		cat $f | sed -e 's/stroke="[^"]*"//g' >../$p-$bname
	done

	echo "" >>$json
	echo "		}" >>$json
	echo "	}" >>$json
done
	
echo "}" >>$json


echo
echo "Done"
echo
echo "Generated JSON:"

cat $json

exit 0
