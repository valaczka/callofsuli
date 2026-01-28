#!/bin/bash

PATCHER=./patcher

MANIFEST="$1"

if [ "$MANIFEST" = "" ]; then
	echo "Usage: $0 <manifest> <apk> [...]" >&2
	exit 1
fi

shift

while [ "$1" != "" ]; do
	hex=$(java -jar ~/usr/android-sdk/build-tools/35.0.0/lib/apksigner.jar verify --print-certs "$1" | grep "SHA-256" | sed -e 's/^.*SHA-256 digest: //')
	echo "SHA-256: $hex"
	$PATCHER --android "$hex" "$MANIFEST"

	shift
done

exit 0
