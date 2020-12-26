PRAGMA foreign_keys = ON;

CREATE TABLE maps(
	uuid TEXT NOT NULL,
	version INTEGER NOT NULL DEFAULT 1,
	name TEXT,
	draft BOOL DEFAULT TRUE,
	owner TEXT,
	md5 TEXT,
	data BLOB,
	PRIMARY KEY(uuid, version)
);
