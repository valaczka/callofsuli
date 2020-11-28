PRAGMA foreign_keys = ON;

CREATE TABLE mapdata (
	uuid TEXT NOT NULL PRIMARY KEY,
	owner TEXT,
	name TEXT,
	md5 TEXT,
	data BLOB
);
