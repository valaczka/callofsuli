PRAGMA foreign_keys = OFF;


----------------------------------
--- Call Pass
----------------------------------

CREATE TABLE pass(
	id INTEGER NOT NULL PRIMARY KEY,
	groupid INTEGER NOT NULL REFERENCES studentgroup(id) ON UPDATE CASCADE ON DELETE CASCADE,
	starttime INTEGER,
	endtime INTEGER,
	title TEXT,
	grading TEXT
);

CREATE TABLE passCategory(
	id INTEGER NOT NULL PRIMARY KEY,
	username TEXT NOT NULL REFERENCES user(username) ON UPDATE CASCADE ON DELETE CASCADE,
	description TEXT
);

CREATE TABLE passItem(
	id INTEGER NOT NULL PRIMARY KEY,
	passid INTEGER NOT NULL REFERENCES pass(id) ON UPDATE CASCADE ON DELETE CASCADE,
	categoryid INTEGER REFERENCES passCategory(id) ON UPDATE CASCADE ON DELETE SET NULL,
	includepass INTEGER REFERENCES pass(id) ON UPDATE CASCADE ON DELETE CASCADE,
	description TEXT,
	extra BOOL NOT NULL DEFAULT FALSE,
	pts REAL NOT NULL DEFAULT 0
);

CREATE TABLE passResult(
	id INTEGER NOT NULL PRIMARY KEY,
	passitemid INTEGER NOT NULL REFERENCES passItem(id) ON UPDATE CASCADE ON DELETE CASCADE,
	username TEXT NOT NULL REFERENCES user(username) ON UPDATE CASCADE ON DELETE CASCADE,
	result REAL NOT NULL DEFAULT 0,
	UNIQUE(passitemid, username)
);

CREATE VIEW passHierarchy AS
	WITH RECURSIVE ip(root, current, include, base) AS
	(SELECT pass.id AS root, passItem.id AS current, passItem.includePass AS include, pass.id AS base
		FROM pass JOIN passItem ON passItem.passid=pass.id
		UNION ALL
		SELECT ip.include, passItem.id, passItem.includePass, ip.base
		FROM ip JOIN passItem on passItem.passid=ip.include)
	SELECT base AS passid, current AS passitemid, (ip.base=ip.root) AS childless
	FROM ip WHERE include IS NULL;

CREATE VIEW passResultUser AS
	SELECT passResult.passitemid, passResult.username, passResult.result,
		(passResult.result * passItem.pts) AS pts, passItem.pts AS maxPts, passItem.extra AS extra,
		passCategory.description AS category, passItem.categoryid, passItem.description AS description
	FROM passResult LEFT JOIN passItem ON (passItem.id = passResult.passitemid)
	LEFT JOIN passCategory ON (passCategory.id = passItem.categoryid);

CREATE VIEW passSumResult AS
	WITH r AS (SELECT h.passid, h.passitemid, h.childless, p.username, p.result, p.pts, p.maxPts, p.extra
		FROM passHierarchy h
		LEFT JOIN passResultUser p ON (p.passitemid = h.passitemid)),
	u AS (SELECT DISTINCT r.passid, r.username, r.childless FROM r)
	SELECT u.passid, u.username, u.childless,
		(SELECT SUM(pts) FROM r WHERE r.passid=u.passid AND r.username=u.username) AS pts,
		(SELECT SUM(maxPts) FROM r WHERE r.passid=u.passid AND r.username=u.username AND r.extra IS FALSE) AS maxPts
	FROM u;


ALTER TABLE campaign ADD COLUMN passitemid INTEGER REFERENCES passItem(id) ON UPDATE CASCADE ON DELETE SET NULL;

ALTER TABLE exam ADD COLUMN passitemid INTEGER REFERENCES passItem(id) ON UPDATE CASCADE ON DELETE SET NULL;

