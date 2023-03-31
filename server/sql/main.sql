PRAGMA foreign_keys = ON;

----------------------------------
--- System
----------------------------------

CREATE TABLE system(
	versionMajor INTEGER,
	versionMinor INTEGER,
	serverName TEXT,
	config TEXT
);


----------------------------------
--- Class
----------------------------------

CREATE TABLE class(
	id INTEGER NOT NULL PRIMARY KEY,
	name TEXT
);


CREATE TABLE classCode(
	classid INTEGER REFERENCES class(id) ON UPDATE CASCADE ON DELETE CASCADE,
	code TEXT,
	UNIQUE(classid),
	UNIQUE(code)
);


----------------------------------
--- Users & Auth
----------------------------------

CREATE TABLE user(
	username TEXT NOT NULL PRIMARY KEY,
	familyName TEXT,
	givenName TEXT,
	active BOOL NOT NULL DEFAULT false,
	classid INTEGER REFERENCES class(id) ON UPDATE CASCADE ON DELETE SET NULL,
	isTeacher BOOL NOT NULL DEFAULT false,
	isAdmin BOOL NOT NULL DEFAULT false,
	isPanel BOOL NOT NULL DEFAULT false,
	nickname TEXT,
	character TEXT,
	picture TEXT,
	UNIQUE (username)
);

CREATE TABLE auth(
	username TEXT NOT NULL REFERENCES user(username) ON UPDATE CASCADE ON DELETE CASCADE,
	password TEXT,
	salt TEXT,
	oauth TEXT,
	oauthData TEXT,
	UNIQUE (username)
);


----------------------------------
--- Ranks
----------------------------------

CREATE TABLE rank(
	id INTEGER NOT NULL PRIMARY KEY,
	level INTEGER NOT NULL CHECK(level>=0),
	sublevel INTEGER CHECK(sublevel>=0),
	xp INTEGER CHECK(xp>=0),
	name TEXT NOT NULL
);


----------------------------------
--- Grades
----------------------------------

CREATE TABLE grade(
	id INTEGER NOT NULL PRIMARY KEY,
	shortname TEXT,
	longname TEXT,
	value INTEGER NOT NULL DEFAULT 0 CHECK (value>=0)
);


----------------------------------
--- Maps
----------------------------------

CREATE TABLE mapOwner(
	mapuuid TEXT NOT NULL,
	username TEXT NOT NULL REFERENCES user(username) ON UPDATE CASCADE ON DELETE CASCADE,
	UNIQUE (mapuuid, username)
);



----------------------------------
--- Groups
----------------------------------


CREATE TABLE studentgroup(
	id INTEGER PRIMARY KEY,
	name TEXT,
	owner TEXT NOT NULL REFERENCES user(username) ON UPDATE CASCADE ON DELETE CASCADE,
	active BOOL NOT NULL DEFAULT true
);

CREATE TABLE bindGroupStudent(
	groupid INTEGER NOT NULL REFERENCES studentgroup(id) ON UPDATE CASCADE ON DELETE CASCADE,
	username TEXT NOT NULL REFERENCES user(username) ON UPDATE CASCADE ON DELETE CASCADE,
	UNIQUE(groupid, username)
);

CREATE TABLE bindGroupClass(
	groupid INTEGER NOT NULL REFERENCES studentgroup(id) ON UPDATE CASCADE ON DELETE CASCADE,
	classid INTEGER NOT NULL REFERENCES class(id) ON UPDATE CASCADE ON DELETE CASCADE,
	UNIQUE(groupid, classid)
);

CREATE VIEW studentGroupInfo AS
	SELECT id, name, owner, studentgroup.active, username FROM studentgroup
		INNER JOIN bindGroupStudent ON (bindGroupStudent.groupid = studentgroup.id)
	UNION
	SELECT id, name, owner, studentgroup.active, username FROM studentgroup
		INNER JOIN bindGroupClass ON (bindGroupClass.groupid = studentgroup.id)
		INNER JOIN user ON (user.classid = bindGroupClass.classid);


----------------------------------
--- Campaign
----------------------------------

CREATE TABLE campaign(
	id INTEGER NOT NULL PRIMARY KEY,
	groupid INTEGER NOT NULL REFERENCES studentgroup(id) ON UPDATE CASCADE ON DELETE CASCADE,
	starttime TEXT NOT NULL DEFAULT (datetime('now')),
	endtime TEXT NOT NULL DEFAULT (datetime('now')),
	description TEXT,
	started BOOL NOT NULL DEFAULT FALSE,
	finished BOOL NOT NULL DEFAULT FALSE,
	defaultGrade INTEGER REFERENCES grade(id) ON UPDATE CASCADE ON DELETE SET NULL
);

CREATE TABLE task(
	id INTEGER NOT NULL PRIMARY KEY,
	campaignid INTEGER NOT NULL REFERENCES campaign(id) ON UPDATE CASCADE ON DELETE CASCADE,
	gradeid INTEGER REFERENCES grade(id) ON UPDATE CASCADE ON DELETE SET NULL,
	xp INTEGER,
	required BOOL NOT NULL DEFAULT FALSE,
	mapuuid TEXT,
	criterion TEXT
);



----------------------------------
--- Rank log
----------------------------------

CREATE TABLE ranklog(
	id INTEGER PRIMARY KEY,
	username TEXT NOT NULL REFERENCES user(username) ON UPDATE CASCADE ON DELETE CASCADE,
	rankid INTEGER NOT NULL REFERENCES rank(id) ON UPDATE CASCADE ON DELETE CASCADE,
	timestamp TEXT NOT NULL DEFAULT (datetime('now')),
	xp INTEGER
);

CREATE VIEW userRank AS
SELECT u.username, COALESCE(s.xp, 0) as xp, r.id as rankid, r.name as name, r.level as level, r.sublevel as sublevel
	FROM user u
	LEFT JOIN (SELECT username, (SELECT SUM(xp) FROM score WHERE score.username=uuu.username) as xp FROM user uuu) s ON (s.username=u.username)
	LEFT JOIN (SELECT uu.username,
			CASE WHEN uu.isTeacher=1 THEN COALESCE((SELECT MAX(id) FROM rank WHERE xp IS null), (SELECT MIN(id) FROM rank))
			ELSE COALESCE(rl.rankid, (SELECT MIN(id) FROM rank))
			END as rankid
		FROM user uu
		LEFT JOIN (SELECT username, rankid FROM ranklog GROUP BY username HAVING MAX(timestamp) AND MAX(id)) rl ON (rl.username=uu.username)) ur
		ON (ur.username=u.username)
	LEFT JOIN (SELECT id, name, level, sublevel FROM rank) r ON (r.id=ur.rankid)
	WHERE u.isPanel=false;


----------------------------------
--- Score
----------------------------------


CREATE TABLE score(
	id INTEGER PRIMARY KEY,
	username TEXT NOT NULL REFERENCES user(username) ON UPDATE CASCADE ON DELETE CASCADE,
	timestamp TEXT NOT NULL DEFAULT (datetime('now')),
	xp INTEGER NOT NULL DEFAULT 0 CHECK (xp>=0)
);


CREATE TRIGGER score_rank_update
AFTER INSERT ON score
BEGIN
	INSERT INTO ranklog (username, rankid, xp)
		SELECT NEW.username, rank.id, userRank.xp FROM userRank LEFT JOIN rank ON (rank.xp<=userRank.xp)
		WHERE username=NEW.username AND rank.id>userRank.rankid ORDER BY rank.id DESC LIMIT 1;
END;
