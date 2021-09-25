PRAGMA foreign_keys = ON;

CREATE TABLE stat(
	username TEXT NOT NULL,
	map TEXT NOT NULL,
	objective TEXT NOT NULL,
	success BOOL NOT NULL DEFAULT FALSE,
	elapsed INTEGER NOT NULL DEFAULT 0,
	dtstamp TEXT NOT NULL DEFAULT (datetime('now'))
);
