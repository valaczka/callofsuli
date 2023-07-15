PRAGMA foreign_keys = ON;

----------------------------------
--- System
----------------------------------

CREATE TABLE system(
	versionMajor INTEGER,
	versionMinor INTEGER
);


----------------------------------
--- Map
----------------------------------

CREATE TABLE map(
	uuid TEXT NOT NULL PRIMARY KEY,
	name TEXT,
	version INTEGER NOT NULL DEFAULT 1,
	md5 TEXT,
	lastModified TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
	lastEditor TEXT,
	data BLOB
);


----------------------------------
--- Draft
----------------------------------

CREATE TABLE draft(
	uuid TEXT NOT NULL REFERENCES map(uuid) ON UPDATE CASCADE ON DELETE CASCADE,
	lastModified TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
	version INTEGER NOT NULL DEFAULT 1,
	data BLOB,
	UNIQUE(uuid)
);


----------------------------------
--- Cache
----------------------------------

CREATE TABLE cache(
	uuid TEXT NOT NULL REFERENCES map(uuid) ON UPDATE CASCADE ON DELETE CASCADE,
	data TEXT,
	UNIQUE(uuid)
);

