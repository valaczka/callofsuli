PRAGMA foreign_keys = ON;

CREATE TABLE mapdata (
	id INTEGER PRIMARY KEY,
	refid INTEGER,
	name TEXT,
	uuid TEXT,
	md5 TEXT,
	data BLOB
);
