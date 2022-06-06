PRAGMA foreign_keys = ON;

ALTER TABLE game ADD COLUMN lite BOOL NOT NULL DEFAULT FALSE;

CREATE TABLE grade(
	id INTEGER NOT NULL PRIMARY KEY,
	owner TEXT NOT NULL REFERENCES user(username) ON UPDATE CASCADE ON DELETE CASCADE,
	shortname TEXT NOT NULL,
	longname TEXT,
	value INTEGER NOT NULL DEFAULT 0,
	UNIQUE(owner, shortname)
);


CREATE TABLE campaign(
	id INTEGER NOT NULL PRIMARY KEY,
	groupid INTEGER NOT NULL REFERENCES studentgroup(id) ON UPDATE CASCADE ON DELETE CASCADE,
	starttime TEXT NOT NULL DEFAULT (datetime('now')),
	endtime TEXT NOT NULL DEFAULT (datetime('now')),
	description TEXT,
	started BOOL NOT NULL DEFAULT FALSE,
	finished BOOL NOT NULL DEFAULT FALSE,
	mapopen TEXT,
	mapclose TEXT
);

CREATE TABLE assignment(
	id INTEGER NOT NULL PRIMARY KEY,
	campaignid INTEGER NOT NULL REFERENCES campaign(id) ON UPDATE CASCADE ON DELETE CASCADE,
	name TEXT
);

CREATE TABLE grading(
	id INTEGER NOT NULL PRIMARY KEY,
	assignmentid INTEGER REFERENCES assignment(id) ON UPDATE CASCADE ON DELETE CASCADE,
	gradeid INTEGER REFERENCES grade(id) ON UPDATE CASCADE ON DELETE SET NULL,
	xp INTEGER,
	criteria TEXT
);

CREATE TABLE gradebook(
	id INTEGER NOT NULL PRIMARY KEY,
	username TEXT NOT NULL REFERENCES user(username) ON UPDATE CASCADE ON DELETE CASCADE,
	groupid INTEGER NOT NULL REFERENCES studentgroup(id) ON UPDATE CASCADE ON DELETE CASCADE,
	timestamp TEXT NOT NULL DEFAULT (datetime('now')),
	gradeid INTEGER NOT NULL REFERENCES grade(id) ON UPDATE CASCADE ON DELETE CASCADE,
	assignmentid INTEGER REFERENCES assignment(id) ON UPDATE CASCADE ON DELETE SET NULL
);

ALTER TABLE score ADD COLUMN assignmentid INTEGER REFERENCES assignment(id) ON UPDATE CASCADE ON DELETE SET NULL;

UPDATE system SET versionMajor=3, versionMinor=2;
