PRAGMA foreign_keys = ON;

CREATE TABLE result (
	id INTEGER PRIMARY KEY,
	uuid TEXT,
	level INTEGER,
	attempt INTEGER,
	success INTEGER,
	UNIQUE (uuid, level)
);
