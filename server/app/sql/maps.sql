PRAGMA foreign_keys = ON;

CREATE TABLE maps(
	uuid TEXT NOT NULL PRIMARY KEY,
	version INTEGER NOT NULL DEFAULT 1,
	name TEXT,
	owner TEXT,
	md5 TEXT,
	lastModified TEXT NOT NULL DEFAULT (datetime('now')),
	data BLOB
);
