#!/bin/bash

sum=0; for f in $(find -type f -and \( -path "./client/app/*" -or -path "./client/deploy/*" -or -path "./server/*" \) ); do x=$(wc -l $f | cut -f 1 -d" "); sum=$(expr $sum + $x) ; echo $f - $sum ; done

