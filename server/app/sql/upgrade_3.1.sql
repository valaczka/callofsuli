PRAGMA foreign_keys = ON;

DROP TABLE IF EXISTS registration;

DROP TABLE IF EXISTS passwordReset;

CREATE TABLE classRegistration(
	classid INTEGER REFERENCES class(id) ON UPDATE CASCADE ON DELETE CASCADE,
	code TEXT NOT NULL,
	UNIQUE(classid),
	UNIQUE(code)
);


UPDATE system SET versionMajor=3, versionMinor=1;
