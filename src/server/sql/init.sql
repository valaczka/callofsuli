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
	key TEXT,
	value TEXT
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
	isAdmin BOOL NOT NULL DEFAULT false
);

CREATE TABLE auth(
	username TEXT NOT NULL REFERENCES user(username) ON UPDATE CASCADE ON DELETE CASCADE,
	password TEXT,
	salt TEXT
);


CREATE TABLE session(
	token TEXT NOT NULL UNIQUE DEFAULT (lower(hex(randomblob(16)))),
	username TEXT NOT NULL REFERENCES user(username) ON UPDATE CASCADE ON DELETE CASCADE,
	lastDate TEXT NOT NULL DEFAULT (datetime('now'))
);


CREATE TABLE studentgroup(
	id INTEGER PRIMARY KEY,
	name TEXT,
	owner TEXT REFERENCES user(username) ON UPDATE CASCADE ON DELETE CASCADE
);

CREATE TABLE bindGroupStudent(
	groupid INTEGER NOT NULL REFERENCES studentgroup(id) ON UPDATE CASCADE ON DELETE CASCADE,
	username TEXT NOT NULL REFERENCES user(username) ON UPDATE CASCADE ON DELETE CASCADE
);

CREATE TABLE map(
	id INTEGER PRIMARY KEY,
	owner TEXT NOT NULL REFERENCES user(username) ON UPDATE CASCADE ON DELETE CASCADE,
	name TEXT,
	timeCreated TEXT NOT NULL DEFAULT (datetime('now')),
	timeModified TEXT NOT NULL DEFAULT (datetime('now')),
	version INTEGER NOT NULL DEFAULT 0,
	objectives INTEGER NOT NULL DEFAULT 0 CHECK (objectives>=0),
	hash TEXT,
	solution TEXT
);


CREATE TABLE bindGroupMap(
	groupid INTEGER NOT NULL REFERENCES studentgroup(id) ON UPDATE CASCADE ON DELETE CASCADE,
	mapid INTEGER NOT NULL REFERENCES map(id) ON UPDATE CASCADE ON DELETE CASCADE
);


CREATE TABLE point(
	level INTEGER PRIMARY KEY,
	obj INTEGER NOT NULL DEFAULT 0 CHECK (obj>=0),
	ch1 INTEGER NOT NULL DEFAULT 0 CHECK (ch1>=0),
	chR INTEGER NOT NULL DEFAULT 0 CHECK (chR>=0),
	m1 INTEGER NOT NULL DEFAULT 0 CHECK (m1>=0),
	mR INTEGER NOT NULL DEFAULT 0 CHECK (mR>=0),
	ca1 INTEGER NOT NULL DEFAULT 0 CHECK (ca1>=0),
	caR INTEGER NOT NULL DEFAULT 0 CHECK (caR>=0)
);


CREATE TABLE achievement(
	id TEXT NOT NULL,
	levelid INTEGER NOT NULL REFERENCES point(level) ON UPDATE CASCADE ON DELETE CASCADE,
	xp INTEGER NOT NULL DEFAULT 0 CHECK (xp>=0)
);

CREATE TABLE rank(
	id INTEGER PRIMARY KEY,
	name TEXT NOT NULL,
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
	achievementid INTEGER REFERENCES achievement(id) ON UPDATE CASCADE ON DELETE SET NULL,
	mapid INTEGER REFERENCES map(id) ON UPDATE CASCADE ON DELETE SET NULL,
	mission INTEGER
);




CREATE VIEW userInfo AS
SELECT u.username, u.firstname, u.lastname, u.email, u.active, u.isTeacher, u.isAdmin, u.classid,
	c.name as classname, COALESCE(s.xp, 0) as xp, r.id as rankid, r.name as rankname
	FROM user u
	LEFT JOIN class c ON (c.id=u.classid)
	LEFT JOIN (SELECT username, (SELECT SUM(xp) FROM score WHERE score.username=uuu.username) as xp FROM user uuu) s ON (s.username=u.username)
	LEFT JOIN (SELECT uu.username, COALESCE(rl.rankid, (SELECT MIN(id) FROM rank)) as rankid
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


INSERT INTO rank VALUES (0,'közkatona',0);

INSERT INTO rank VALUES (1,'őrvezető',500);

INSERT INTO rank VALUES (2,'tizedes',1150);

INSERT INTO rank VALUES (3,'szakaszvezető',1950);

INSERT INTO rank VALUES (4,'őrmester',2900);

INSERT INTO rank VALUES (5,'törzsőrmester',4000);

INSERT INTO rank VALUES (6,'főtörzsőrmester',5250);

INSERT INTO rank VALUES (7,'zászlós',6650);

INSERT INTO rank VALUES (8,'törzszászlós',8200);

INSERT INTO rank VALUES (9,'főtörzszászlós',9900);

INSERT INTO rank VALUES (10,'alhadnagy',11750);

INSERT INTO rank VALUES (11,'hadnagy',13750);

INSERT INTO rank VALUES (12,'főhadnagy',15900);

INSERT INTO rank VALUES (13,'százados',18200);

INSERT INTO rank VALUES (14,'őrnagy',20650);

INSERT INTO rank VALUES (15,'alezredes',23250);

INSERT INTO rank VALUES (16,'ezredes',26000);

INSERT INTO rank VALUES (17,'dandártábornok',28900);

INSERT INTO rank VALUES (18,'vezérőrnagy',31950);

INSERT INTO rank VALUES (19,'altábornagy',35150);

INSERT INTO rank VALUES (100,'vezérezredes',null);



INSERT INTO point VALUES (1, 3, 50, 15, 200, 175, 500, 400);

INSERT INTO point VALUES (2, 5, 60, 20, 250, 195, 600, 500);

INSERT INTO point VALUES (3, 7, 70, 25, 300, 215, 700, 600);







