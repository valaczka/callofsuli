lines = "$${LITERAL_HASH}define COSversion = $${VERSION}"
lines += "$${LITERAL_HASH}define COSexe = \"Call_of_Suli_$${VERSION}_install\""
lines += $$cat(../deploy/CallOfSuli.iss, blob)


write_file($${OUT_PWD}/$${DESTDIR}/InnoSetup.iss, lines)

message(Create InnoSetup: $${OUT_PWD}/$${DESTDIR}/InnoSetup.iss)
