PRAGMA foreign_keys = ON;

----------------------------------
--- Statistics
----------------------------------

CREATE TABLE system(
	versionMajor INTEGER,
	versionMinor INTEGER
);


CREATE TABLE statistics(
	username TEXT,
	mode INTEGER,
	map TEXT,
	objective TEXT,
	success BOOL,
	elapsed INTEGER,
	module TEXT
);

