PRAGMA foreign_keys = ON;

CREATE TABLE system(
	versionMajor INTEGER,
	versionMinor INTEGER,
	serverName TEXT,
	serverUuid TEXT
);

CREATE TABLE settings(
	key TEXT NOT NULL,
	value TEXT,
	UNIQUE (key)
);

CREATE TABLE class(
	id INTEGER PRIMARY KEY,
	name TEXT
);

CREATE TABLE user(
	username TEXT NOT NULL PRIMARY KEY,
	firstname TEXT,
	lastname TEXT,
	active BOOL NOT NULL DEFAULT false,
	classid INTEGER REFERENCES class(id) ON UPDATE CASCADE ON DELETE SET NULL,
	isTeacher BOOL NOT NULL DEFAULT false,
	isAdmin BOOL NOT NULL DEFAULT false,
	nickname TEXT,
	character TEXT,
	picture TEXT,
	UNIQUE (username)
);


CREATE TABLE auth(
	username TEXT NOT NULL REFERENCES user(username) ON UPDATE CASCADE ON DELETE CASCADE,
	password TEXT,
	salt TEXT,
	oauthToken TEXT,
	UNIQUE (username)
);


CREATE TABLE session(
	token TEXT NOT NULL UNIQUE DEFAULT (lower(hex(randomblob(16)))),
	username TEXT NOT NULL REFERENCES user(username) ON UPDATE CASCADE ON DELETE CASCADE,
	lastDate TEXT NOT NULL DEFAULT (datetime('now'))
);


CREATE TABLE studentgroup(
	id INTEGER PRIMARY KEY,
	name TEXT,
	owner TEXT NOT NULL REFERENCES user(username) ON UPDATE CASCADE ON DELETE CASCADE
);

CREATE TABLE bindGroupStudent(
	groupid INTEGER NOT NULL REFERENCES studentgroup(id) ON UPDATE CASCADE ON DELETE CASCADE,
	username TEXT NOT NULL REFERENCES user(username) ON UPDATE CASCADE ON DELETE CASCADE
);

CREATE TABLE bindGroupClass(
	groupid INTEGER NOT NULL REFERENCES studentgroup(id) ON UPDATE CASCADE ON DELETE CASCADE,
	classid INTEGER NOT NULL REFERENCES class(id) ON UPDATE CASCADE ON DELETE CASCADE
);

CREATE VIEW studentGroupInfo AS
	SELECT id, name, owner, username FROM studentgroup
		INNER JOIN bindGroupStudent ON (bindGroupStudent.groupid = studentgroup.id)
	UNION
	SELECT id, name, owner, username FROM studentgroup
		INNER JOIN bindGroupClass ON (bindGroupClass.groupid = studentgroup.id)
		LEFT JOIN user ON (user.classid = bindGroupClass.classid);



CREATE TABLE bindGroupMap(
	id INTEGER PRIMARY KEY,
	groupid INTEGER NOT NULL REFERENCES studentgroup(id) ON UPDATE CASCADE ON DELETE CASCADE,
	mapid TEXT NOT NULL,
	active BOOL NOT NULL DEFAULT FALSE,
	UNIQUE(groupid, mapid)
);



CREATE TABLE game(
	id INTEGER PRIMARY KEY,
	username TEXT NOT NULL REFERENCES user(username) ON UPDATE CASCADE ON DELETE CASCADE,
	mapid TEXT NOT NULL,
	missionid TEXT NOT NULL,
	timestamp TEXT NOT NULL DEFAULT (datetime('now')),
	level INTEGER NOT NULL DEFAULT 1,
	success BOOL NOT NULL DEFAULT FALSE,
	deathmatch BOOL NOT NULL DEFAULT FALSE,
	lite BOOL NOT NULL DEFAULT FALSE,
	duration INTEGER,
	tmpScore INTEGER
);





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



CREATE TABLE rank(
	id INTEGER PRIMARY KEY,
	name TEXT NOT NULL,
	level INTEGER,
	image TEXT NOT NULL,
	xp INTEGER CHECK (xp>=0)
);

CREATE TABLE ranklog(
	id INTEGER PRIMARY KEY,
	username TEXT NOT NULL REFERENCES user(username) ON UPDATE CASCADE ON DELETE CASCADE,
	rankid INTEGER NOT NULL REFERENCES rank(id) ON UPDATE CASCADE ON DELETE CASCADE,
	timestamp TEXT NOT NULL DEFAULT (datetime('now')),
	xp INTEGER
);




CREATE TABLE score(
	id INTEGER PRIMARY KEY,
	username TEXT NOT NULL REFERENCES user(username) ON UPDATE CASCADE ON DELETE CASCADE,
	timestamp TEXT NOT NULL DEFAULT (datetime('now')),
	xp INTEGER NOT NULL DEFAULT 0 CHECK (xp>=0),
	gameid INTEGER REFERENCES game(id) ON UPDATE CASCADE ON DELETE SET NULL,
	assignmentid INTEGER REFERENCES assignment(id) ON UPDATE CASCADE ON DELETE SET NULL,
	maxStreak INTEGER
);




CREATE TABLE classRegistration(
	classid INTEGER REFERENCES class(id) ON UPDATE CASCADE ON DELETE CASCADE,
	code TEXT NOT NULL,
	UNIQUE(classid,
	UNIQUE(code)
);



CREATE VIEW userInfo AS
SELECT u.username, u.firstname, u.lastname, u.active, u.isTeacher, u.isAdmin, u.classid, u.nickname, u.character, u.picture,
	c.name as classname, COALESCE(s.xp, 0) as xp, r.id as rankid, r.name as rankname, r.level as ranklevel, r.image as rankimage
	FROM user u
	LEFT JOIN class c ON (c.id=u.classid)
	LEFT JOIN (SELECT username, (SELECT SUM(xp) FROM score WHERE score.username=uuu.username) as xp FROM user uuu) s ON (s.username=u.username)
	LEFT JOIN (SELECT uu.username,
			CASE WHEN uu.isTeacher=1 THEN COALESCE((SELECT MAX(id) FROM rank WHERE xp IS null), (SELECT MIN(id) FROM rank))
			ELSE COALESCE(rl.rankid, (SELECT MIN(id) FROM rank))
			END as rankid
		FROM user uu
		LEFT JOIN (SELECT username, rankid FROM ranklog GROUP BY username HAVING MAX(timestamp) AND MAX(id)) rl ON (rl.username=uu.username)) ur
		ON (ur.username=u.username)
	LEFT JOIN (SELECT id, name, level, image FROM rank) r ON (r.id=ur.rankid);




CREATE VIEW userTrophy AS
SELECT game.username, mapid, level, deathmatch, success, COUNT(*) as num, SUM(xp) as xp
	FROM game LEFT JOIN score ON (score.gameid=game.id)
	GROUP BY game.username, mapid, level, deathmatch, success;


CREATE VIEW missionTrophy AS
SELECT game.username, mapid, missionid, level, deathmatch, success, COUNT(*) as num, SUM(xp) as xp
	FROM game LEFT JOIN score ON (score.gameid=game.id)
	GROUP BY game.username, mapid, missionid, level, deathmatch, success;



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




CREATE VIEW fullTrophy AS
SELECT user.username,
	(SELECT COALESCE(SUM(num),0) FROM userTrophy WHERE userTrophy.username=user.username
		AND level=1 AND deathmatch=false AND success=true)
	AS t1,
	(SELECT COALESCE(SUM(num),0) FROM userTrophy WHERE userTrophy.username=user.username
		AND level=2 AND deathmatch=false AND success=true)
	AS t2,
	(SELECT COALESCE(SUM(num),0) FROM userTrophy WHERE userTrophy.username=user.username
		AND level=3 AND deathmatch=false AND success=true)
	AS t3,
	(SELECT COALESCE(SUM(num),0) FROM userTrophy WHERE userTrophy.username=user.username
		AND level=1 AND deathmatch=true AND success=true)
	AS d1,
	(SELECT COALESCE(SUM(num),0) FROM userTrophy WHERE userTrophy.username=user.username
		AND level=2 AND deathmatch=true AND success=true)
	AS d2,
	(SELECT COALESCE(SUM(num),0) FROM userTrophy WHERE userTrophy.username=user.username
		AND level=3 AND deathmatch=true AND success=true)
	AS d3,
	(SELECT COALESCE(SUM(xp),0) FROM userTrophy WHERE userTrophy.username=user.username)
	AS sumxp
	FROM user;




CREATE TRIGGER rank_update
AFTER INSERT ON score
BEGIN
	INSERT INTO ranklog (username, rankid, xp)
		SELECT NEW.username, rank.id, userInfo.xp FROM userInfo LEFT JOIN rank ON (rank.xp<=userInfo.xp)
		WHERE username=NEW.username AND rank.id>userInfo.rankid ORDER BY rank.id DESC LIMIT 1;
END;

