PRAGMA foreign_keys = ON;

----------------------------------
--- Campaign
----------------------------------

CREATE TABLE campaignStudent(
	id INTEGER NOT NULL PRIMARY KEY,
	campaignid INTEGER NOT NULL REFERENCES campaign(id) ON UPDATE CASCADE ON DELETE CASCADE,
	username TEXT NOT NULL REFERENCES user(username) ON UPDATE CASCADE ON DELETE CASCADE
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

CREATE VIEW dailyLimit AS
WITH u AS (SELECT username, COALESCE((SELECT value FROM dailyLimitUser WHERE username=user.username), 0) AS userLimit,
		COALESCE((SELECT value FROM dailyLimitClass WHERE classid=user.classid), 0) AS classLimit FROM user),
	t AS (SELECT username, CASE WHEN userLimit>0 THEN userLimit ELSE classLimit END AS userLimit FROM u),
	s AS (SELECT t.username, t.userLimit, SUM(duration)/1000 AS seconds FROM t
		LEFT JOIN game ON (game.username=t.username AND date(game.timestamp) = date('now')) GROUP BY t.username)
	SELECT username, userLimit, seconds, CASE WHEN userLimit > 0 THEN seconds*1.0/userLimit ELSE 0 END AS rate FROM s;

