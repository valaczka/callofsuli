PRAGMA foreign_keys = ON;

CREATE TABLE mapdata (
	id INTEGER PRIMARY KEY,
	refid INTEGER,
	uuid TEXT UNIQUE,
	md5 TEXT,
	data BLOB
);
