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

CREATE TABLE dailyLimitClass(
	classid INTEGER NOT NULL REFERENCES class(id) ON UPDATE CASCADE ON DELETE CASCADE,
	value INTEGER NOT NULL DEFAULT 5400,
	UNIQUE(classid)
);

CREATE TABLE dailyLimitUser(
	username TEXT NOT NULL REFERENCES user(username) ON UPDATE CASCADE ON DELETE CASCADE,
	value INTEGER NOT NULL DEFAULT 5400,
	UNIQUE(username)
);

CREATE TABLE extraRole(
	username TEXT NOT NULL REFERENCES user(username) ON UPDATE CASCADE ON DELETE CASCADE,
	role TEXT NOT NULL
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


CREATE TABLE ranklog(
	id INTEGER PRIMARY KEY,
	username TEXT NOT NULL REFERENCES user(username) ON UPDATE CASCADE ON DELETE CASCADE,
	rankid INTEGER NOT NULL REFERENCES rank(id) ON UPDATE CASCADE ON DELETE CASCADE,
	timestamp TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
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
	timestamp TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
	xp INTEGER NOT NULL DEFAULT 0 CHECK (xp>=0)
);


CREATE TRIGGER score_rank_update
AFTER INSERT ON score
FOR EACH ROW
BEGIN
	INSERT INTO ranklog (username, rankid, xp)
		SELECT NEW.username, rank.id, userRank.xp FROM userRank LEFT JOIN rank ON (rank.xp<=userRank.xp)
		WHERE username=NEW.username AND rank.id>userRank.rankid ORDER BY rank.id DESC LIMIT 1;
END;



----------------------------------
--- Grades
----------------------------------

CREATE TABLE grade(
	id INTEGER NOT NULL PRIMARY KEY,
	shortname TEXT,
	longname TEXT,
	value INTEGER NOT NULL DEFAULT 0 CHECK (value>=0)
);

CREATE TABLE grading(
	id INTEGER NOT NULL PRIMARY KEY,
	owner TEXT NOT NULL REFERENCES user(username) ON UPDATE CASCADE ON DELETE CASCADE,
	name TEXT,
	data TEXT
);

----------------------------------
--- Maps
----------------------------------

CREATE TABLE mapOwner(
	mapuuid TEXT NOT NULL,
	username TEXT NOT NULL REFERENCES user(username) ON UPDATE CASCADE ON DELETE CASCADE,
	UNIQUE (mapuuid, username)
);

CREATE TABLE mapTag(
	id INTEGER NOT NULL PRIMARY KEY,
	tag TEXT NOT NULL,
	username TEXT NOT NULL REFERENCES user(username) ON UPDATE CASCADE ON DELETE CASCADE,
	parentId INTEGER REFERENCES mapTag(id) ON UPDATE CASCADE ON DELETE CASCADE
);

CREATE TABLE mapTagBind(
	mapuuid TEXT NOT NULL REFERENCES mapOwner(mapuuid) ON UPDATE CASCADE ON DELETE CASCADE,
	tagid INTEGER REFERENCES mapTag(id) ON UPDATE CASCADE ON DELETE CASCADE,
	UNIQUE(mapuuid, tagid)
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
	starttime INTEGER,
	endtime INTEGER,
	description TEXT,
	started BOOL NOT NULL DEFAULT FALSE,
	finished BOOL NOT NULL DEFAULT FALSE,
	defaultGrade INTEGER REFERENCES grade(id) ON UPDATE CASCADE ON DELETE SET NULL
);

CREATE TABLE task(
	id INTEGER NOT NULL PRIMARY KEY,
	campaignid INTEGER NOT NULL REFERENCES campaign(id) ON UPDATE CASCADE ON DELETE CASCADE,
	gradeid INTEGER REFERENCES grade(id) ON UPDATE CASCADE ON DELETE SET NULL,
	xp INTEGER CHECK (xp>0),
	required BOOL NOT NULL DEFAULT FALSE,
	mapuuid TEXT,
	criterion TEXT
);

CREATE TABLE campaignResult(
	id INTEGER NOT NULL PRIMARY KEY,
	campaignid INTEGER REFERENCES campaign(id) ON UPDATE CASCADE ON DELETE CASCADE,
	username TEXT NOT NULL REFERENCES user(username) ON UPDATE CASCADE ON DELETE CASCADE,
	gradeid INTEGER REFERENCES grade(id) ON UPDATE CASCADE ON DELETE SET NULL,
	scoreid INTEGER REFERENCES score(id) ON UPDATE CASCADE ON DELETE SET NULL,
	UNIQUE(campaignid, username)
);

CREATE TABLE taskSuccess(
	id INTEGER NOT NULL PRIMARY KEY,
	taskid INTEGER REFERENCES task(id) ON UPDATE CASCADE ON DELETE CASCADE,
	username TEXT NOT NULL REFERENCES user(username) ON UPDATE CASCADE ON DELETE CASCADE,
	UNIQUE(taskid, username)
);

CREATE TABLE campaignStudent(
	id INTEGER NOT NULL PRIMARY KEY,
	campaignid INTEGER NOT NULL REFERENCES campaign(id) ON UPDATE CASCADE ON DELETE CASCADE,
	username TEXT NOT NULL REFERENCES user(username) ON UPDATE CASCADE ON DELETE CASCADE
);


----------------------------------
--- Exam
----------------------------------

CREATE TABLE exam(
	id INTEGER NOT NULL PRIMARY KEY,
	groupid INTEGER NOT NULL REFERENCES studentgroup(id) ON UPDATE CASCADE ON DELETE CASCADE,
	mode INTEGER NOT NULL DEFAULT 0,
	state INTEGER NOT NULL DEFAULT 0,
	mapuuid TEXT,
	description TEXT,
	timestamp INTEGER,
	engineData text
);

CREATE TABLE examContent(
	id INTEGER NOT NULL PRIMARY KEY,
	examid INTEGER NOT NULL REFERENCES exam(id) ON UPDATE CASCADE ON DELETE CASCADE,
	username TEXT NOT NULL REFERENCES user(username) ON UPDATE CASCADE ON DELETE CASCADE,
	data TEXT,
	result REAL,
	gradeid INTEGER REFERENCES grade(id) ON UPDATE CASCADE ON DELETE SET NULL,
	UNIQUE (examid, username)
);

CREATE TABLE examAnswer(
	id INTEGER NOT NULL PRIMARY KEY,
	contentid INTEGER NOT NULL REFERENCES examContent(id) ON UPDATE CASCADE ON DELETE CASCADE,
	answer TEXT,
	correction TEXT,
	UNIQUE(contentid)
);

----------------------------------
--- Game
----------------------------------

CREATE TABLE game(
	id INTEGER PRIMARY KEY,
	username TEXT NOT NULL REFERENCES user(username) ON UPDATE CASCADE ON DELETE CASCADE,
	timestamp TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
	mapid TEXT,
	missionid TEXT,
	campaignid INTEGER REFERENCES campaign(id) ON UPDATE CASCADE ON DELETE SET NULL,
	level INTEGER NOT NULL DEFAULT 1,
	mode INTEGER,
	deathmatch BOOL NOT NULL DEFAULT FALSE,
	success BOOL NOT NULL DEFAULT FALSE,
	duration INTEGER,
	scoreid INTEGER REFERENCES score(id) ON UPDATE CASCADE ON DELETE SET NULL
);

CREATE TABLE runningGame(
	gameid INTEGER NOT NULL REFERENCES game(id) ON UPDATE CASCADE ON DELETE CASCADE,
	xp INTEGER NOT NULL DEFAULT 0,
	UNIQUE(gameid)
);

CREATE TABLE inventory(
	id INTEGER NOT NULL PRIMARY KEY,
	username TEXT NOT NULL REFERENCES user(username) ON UPDATE CASCADE ON DELETE CASCADE,
	key TEXT NOT NULL,
	value INTEGER NOT NULL DEFAULT 0,
	UNIQUE(username, key)
);

CREATE TABLE inventoryLimit(
	key TEXT NOT NULL PRIMARY KEY,
	value INTEGER NOT NULL DEFAULT 1
);

INSERT INTO inventoryLimit VALUES ('hp', 50);
INSERT INTO inventoryLimit VALUES ('shield', 50);
INSERT INTO inventoryLimit VALUES ('water', 50);
INSERT INTO inventoryLimit VALUES ('camouflage', 50);
INSERT INTO inventoryLimit VALUES ('pliers', 2);
INSERT INTO inventoryLimit VALUES ('teleporter', 2);


CREATE VIEW streak AS
WITH game_date AS (SELECT DISTINCT username, date(timestamp) AS date FROM game WHERE success=true),
	game_ranked AS (SELECT *, RANK() OVER(PARTITION BY username ORDER BY date) AS rank FROM game_date),
	streak_view AS (SELECT *, date(date, '-'||rank||' day') AS date_group FROM game_ranked)
	SELECT DISTINCT username, date_group, COUNT(*) AS streak, MIN(date) AS started_on, MAX(date) AS ended_on
	FROM streak_view GROUP BY 1,2;


CREATE VIEW dailyLimit AS
WITH u AS (SELECT username, COALESCE((SELECT value FROM dailyLimitUser WHERE username=user.username), 0) AS userLimit,
		COALESCE((SELECT value FROM dailyLimitClass WHERE classid=user.classid), 0) AS classLimit FROM user),
	t AS (SELECT username, CASE WHEN userLimit>0 THEN userLimit ELSE classLimit END AS userLimit FROM u),
	s AS (SELECT t.username, t.userLimit, SUM(duration)/1000 AS seconds FROM t
		LEFT JOIN game ON (game.username=t.username AND date(game.timestamp) = date('now')) GROUP BY t.username)
	SELECT username, userLimit, seconds, CASE WHEN userLimit > 0 THEN seconds*1.0/userLimit ELSE 0 END AS rate FROM s;
