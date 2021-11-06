aaa = $$cat(../deploy/CallOfSuli.iss, blob)

lines = $$replace(aaa, %VERSION%, $${VERSION})

write_file($${OUT_PWD}/$${DESTDIR}/InnoSetup.iss, lines)

message(Create InnoSetup: $${OUT_PWD}/$${DESTDIR}/InnoSetup.iss)
