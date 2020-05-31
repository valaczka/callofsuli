PRAGMA foreign_keys = ON;

CREATE TABLE system(
	versionMajor INTEGER,
	versionMinor INTEGER,
	socketHost TEXT,
	socketPort INTEGER,
	serverName TEXT,
	connections INTEGER
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
	email TEXT,
	active BOOL NOT NULL DEFAULT false,
	classid INTEGER REFERENCES class(id) ON UPDATE CASCADE ON DELETE SET NULL,
	isTeacher BOOL NOT NULL DEFAULT false,
	isAdmin BOOL NOT NULL DEFAULT false,
	UNIQUE (email)
);

CREATE TRIGGER user_username_insert
BEFORE INSERT ON user
BEGIN
	SELECT CASE
		WHEN NEW.username LIKE '%@%' THEN RAISE (ABORT, 'Invalid username')
	END;
END;


CREATE TRIGGER user_username_update
BEFORE UPDATE ON user
BEGIN
	SELECT CASE
		WHEN NEW.username LIKE '%@%' THEN RAISE (ABORT, 'Invalid username')
	END;
END;

CREATE TRIGGER user_email_insert
BEFORE INSERT ON user
BEGIN
	SELECT CASE
		WHEN NEW.email IS NOT NULL AND NEW.email<>'' AND NEW.email NOT LIKE '%_@__%.__%' THEN RAISE (ABORT, 'Invalid email')
	END;
END;


CREATE TRIGGER user_email_update
BEFORE UPDATE ON user
BEGIN
	SELECT CASE
		WHEN NEW.email IS NOT NULL AND NEW.email<>'' AND NEW.email NOT LIKE '%_@__%.__%' THEN RAISE (ABORT, 'Invalid email')
	END;
END;

CREATE TABLE auth(
	username TEXT NOT NULL REFERENCES user(username) ON UPDATE CASCADE ON DELETE CASCADE,
	password TEXT,
	salt TEXT,
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



CREATE TABLE map(
	id INTEGER PRIMARY KEY,
	owner TEXT NOT NULL REFERENCES user(username) ON UPDATE CASCADE ON DELETE CASCADE,
	name TEXT,
	uuid TEXT NOT NULL UNIQUE,
	md5 TEXT,
	timeCreated TEXT NOT NULL DEFAULT (datetime('now')),
	timeModified TEXT NOT NULL DEFAULT (datetime('now')),
	version INTEGER NOT NULL DEFAULT 0,
	objectives INTEGER NOT NULL DEFAULT 0 CHECK (objectives>=0)
);


CREATE TABLE bindGroupMap(
	groupid INTEGER NOT NULL REFERENCES studentgroup(id) ON UPDATE CASCADE ON DELETE CASCADE,
	mapid INTEGER NOT NULL REFERENCES map(id) ON UPDATE CASCADE ON DELETE CASCADE
);



CREATE VIEW mapGroupInfo AS
	SELECT studentGroupInfo.id as groupid, studentGroupInfo.name as groupname, studentGroupInfo.owner as groupowner, studentGroupInfo.username as username,
	mapid, map.name as mapname, map.uuid as uuid, map.md5 as md5, map.version as version
	FROM studentGroupInfo
	LEFT JOIN bindGroupMap ON (bindGroupMap.groupid = studentGroupInfo.id)
	LEFT JOIN map ON (map.id = bindGroupMap.mapid);


CREATE TABLE mission(
	id INTEGER PRIMARY KEY,
	uuid TEXT NOT NULL UNIQUE,
	mapid INTEGER REFERENCES map(id) ON UPDATE CASCADE ON DELETE SET NULL
);


CREATE TABLE game(
	id INTEGER PRIMARY KEY,
	username TEXT NOT NULL REFERENCES user(username) ON UPDATE CASCADE ON DELETE CASCADE,
	missionuuid TEXT NOT NULL REFERENCES mission(uuid) ON UPDATE CASCADE ON DELETE CASCADE,
	timestamp TEXT NOT NULL DEFAULT (datetime('now')),
	level INTEGER NOT NULL DEFAULT 1,
	active BOOL NOT NULL DEFAULT TRUE,
	hpLoss INTEGER,
	timelength TEXT
);



CREATE TABLE level(
	id INTEGER PRIMARY KEY
);

CREATE TABLE xpReason(
	id INTEGER PRIMARY KEY,
	name TEXT NOT NULL,
	UNIQUE (name)
);

CREATE TABLE point(
	id INTEGER PRIMARY KEY,
	reason TEXT NOT NULL REFERENCES xpReason(name) ON UPDATE CASCADE ON DELETE CASCADE,
	level INTEGER REFERENCES level(id) ON UPDATE CASCADE ON DELETE CASCADE,
	xp INTEGER NOT NULL DEFAULT 0,
	UNIQUE (reason, level)
);


CREATE VIEW pointInfo AS
SELECT xpReason.name as reason, level.id as level, COALESCE(xp, 0) as xp
	FROM xpReason
	CROSS JOIN level
	LEFT JOIN point ON (point.reason=xpReason.name AND point.level=level.id);



CREATE TABLE rank(
	id INTEGER PRIMARY KEY,
	name TEXT NOT NULL,
	level INTEGER NOT NULL DEFAULT 1,
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
	username TEXT NOT NULL REFERENCES user(username) ON UPDATE CASCADE ON DELETE SET NULL,
	timestamp TEXT NOT NULL DEFAULT (datetime('now')),
	xp INTEGER NOT NULL DEFAULT 0 CHECK (xp>=0),
	xpreasonid INTEGER REFERENCES xpReason(id) ON UPDATE CASCADE ON DELETE SET NULL,
	gameid INTEGER REFERENCES game(id) ON UPDATE CASCADE ON DELETE SET NULL
);


CREATE TABLE registration(
	id INTEGER PRIMARY KEY,
	email TEXT NOT NULL,
	firstname TEXT,
	lastname TEXT,
	code TEXT NOT NULL DEFAULT (upper(hex(randomblob(4)))),
	timestamp TEXT NOT NULL DEFAULT (datetime('now')),
	UNIQUE(email)
);


CREATE TABLE passwordReset(
	username TEXT NOT NULL REFERENCES user(username) ON UPDATE CASCADE ON DELETE CASCADE,
	code TEXT NOT NULL DEFAULT (upper(hex(randomblob(4)))),
	timestamp TEXT NOT NULL DEFAULT (datetime('now')),
	UNIQUE (username)
);


CREATE VIEW userInfo AS
SELECT u.username, u.firstname, u.lastname, u.email, u.active, u.isTeacher, u.isAdmin, u.classid,
	c.name as classname, COALESCE(s.xp, 0) as xp, r.id as rankid, r.name as rankname
	FROM user u
	LEFT JOIN class c ON (c.id=u.classid)
	LEFT JOIN (SELECT username, (SELECT SUM(xp) FROM score WHERE score.username=uuu.username) as xp FROM user uuu) s ON (s.username=u.username)
	LEFT JOIN (SELECT uu.username,
			CASE WHEN uu.isTeacher=1 THEN COALESCE((SELECT MAX(id) FROM rank WHERE xp IS null), (SELECT MIN(id) FROM rank))
			ELSE COALESCE(rl.rankid, (SELECT MIN(id) FROM rank))
			END as rankid
		FROM user uu
		LEFT JOIN (SELECT username, rankid FROM ranklog GROUP BY username HAVING MAX(timestamp)) rl ON (rl.username=uu.username)) ur
		ON (ur.username=u.username)
	LEFT JOIN (SELECT id, name FROM rank) r ON (r.id=ur.rankid);


CREATE TRIGGER rank_update
AFTER INSERT ON score
BEGIN
	INSERT INTO ranklog (username, rankid, xp)
		SELECT NEW.username, rank.id, userInfo.xp FROM userInfo LEFT JOIN rank ON (rank.xp<=userInfo.xp)
		WHERE username=NEW.username AND rank.id>userInfo.rankid ORDER BY rank.id;
END;


INSERT INTO rank VALUES (0,'közkatona',1,0);

INSERT INTO rank VALUES (1,'őrvezető',1,500);

INSERT INTO rank VALUES (2,'tizedes',1,1150);

INSERT INTO rank VALUES (3,'szakaszvezető',1,1950);

INSERT INTO rank VALUES (4,'őrmester',1,2900);

INSERT INTO rank VALUES (5,'törzsőrmester',1,4000);

INSERT INTO rank VALUES (6,'főtörzsőrmester',1,5250);

INSERT INTO rank VALUES (7,'zászlós',1,6650);

INSERT INTO rank VALUES (8,'törzszászlós',1,8200);

INSERT INTO rank VALUES (9,'főtörzszászlós',1,9900);

INSERT INTO rank VALUES (10,'alhadnagy',1,11750);

INSERT INTO rank VALUES (11,'hadnagy',1,13750);

INSERT INTO rank VALUES (12,'főhadnagy',1,15900);

INSERT INTO rank VALUES (13,'százados',1,18200);

INSERT INTO rank VALUES (14,'őrnagy',1,20650);

INSERT INTO rank VALUES (15,'alezredes',1,23250);

INSERT INTO rank VALUES (16,'ezredes',1,26000);

INSERT INTO rank VALUES (17,'dandártábornok',1,28900);

INSERT INTO rank VALUES (18,'vezérőrnagy',1,31950);

INSERT INTO rank VALUES (19,'altábornagy',1,35150);

INSERT INTO rank VALUES (100,'vezérezredes',1,null);






