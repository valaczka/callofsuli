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

CREATE TABLE gradingResult(
	gradingid INTEGER REFERENCES grading(id) ON UPDATE CASCADE ON DELETE CASCADE,
	username TEXT NOT NULL REFERENCES user(username) ON UPDATE CASCADE ON DELETE CASCADE,
	success BOOL NOT NULL DEFAULT FALSE
);

ALTER TABLE score ADD COLUMN assignmentid INTEGER REFERENCES assignment(id) ON UPDATE CASCADE ON DELETE SET NULL;


PRAGMA foreign_keys = OFF;

BEGIN TRANSACTION;

ALTER TABLE bindGroupMap RENAME TO bindGroupMap_old;

CREATE TABLE bindGroupMap(
	id INTEGER PRIMARY KEY,
	groupid INTEGER NOT NULL REFERENCES studentgroup(id) ON UPDATE CASCADE ON DELETE CASCADE,
	mapid TEXT NOT NULL,
	active BOOL NOT NULL DEFAULT FALSE,
	UNIQUE(groupid, mapid)
);

INSERT INTO bindGroupMap SELECT * FROM bindGroupMap_old;

COMMIT;

DROP VIEW groupTrophy;

CREATE VIEW groupTrophy AS
SELECT studentGroupInfo.id, studentGroupInfo.username,
	(SELECT COALESCE(SUM(num),0) FROM userTrophy WHERE userTrophy.username=studentGroupInfo.username
		AND level=1 AND deathmatch=false AND success=true
		AND userTrophy.mapid IN (SELECT mapid FROM bindGroupMap WHERE groupid=studentGroupInfo.id))
	AS t1,
	(SELECT COALESCE(SUM(num),0) FROM userTrophy WHERE userTrophy.username=studentGroupInfo.username
		AND level=2 AND deathmatch=false AND success=true
		AND userTrophy.mapid IN (SELECT mapid FROM bindGroupMap WHERE groupid=studentGroupInfo.id))
	AS t2,
	(SELECT COALESCE(SUM(num),0) FROM userTrophy WHERE userTrophy.username=studentGroupInfo.username
		AND level=3 AND deathmatch=false AND success=true
		AND userTrophy.mapid IN (SELECT mapid FROM bindGroupMap WHERE groupid=studentGroupInfo.id))
	AS t3,
	(SELECT COALESCE(SUM(num),0) FROM userTrophy WHERE userTrophy.username=studentGroupInfo.username
		AND level=1 AND deathmatch=true AND success=true
		AND userTrophy.mapid IN (SELECT mapid FROM bindGroupMap WHERE groupid=studentGroupInfo.id))
	AS d1,
	(SELECT COALESCE(SUM(num),0) FROM userTrophy WHERE userTrophy.username=studentGroupInfo.username
		AND level=2 AND deathmatch=true AND success=true
		AND userTrophy.mapid IN (SELECT mapid FROM bindGroupMap WHERE groupid=studentGroupInfo.id))
	AS d2,
	(SELECT COALESCE(SUM(num),0) FROM userTrophy WHERE userTrophy.username=studentGroupInfo.username
		AND level=3 AND deathmatch=true AND success=true
		AND userTrophy.mapid IN (SELECT mapid FROM bindGroupMap WHERE groupid=studentGroupInfo.id))
	AS d3,
	(SELECT COALESCE(SUM(xp),0) FROM userTrophy WHERE userTrophy.username=studentGroupInfo.username
		AND userTrophy.mapid IN (SELECT mapid FROM bindGroupMap WHERE groupid=studentGroupInfo.id))
	AS sumxp
	FROM studentGroupInfo;

DROP TABLE bindGroupMap_old;

PRAGMA foreign_keys=ON;

UPDATE system SET versionMajor=3, versionMinor=2;
