PRAGMA foreign_keys = OFF;

----------------------------------
--- Campaign
----------------------------------

ALTER TABLE taskSuccess ADD COLUMN result REAL NOT NULL DEFAULT 1.0;

ALTER TABLE campaignResult ADD COLUMN maxPts INTEGER;

ALTER TABLE campaignResult ADD COLUMN progress REAL;

ALTER TABLE freeplay RENAME TO freeplayT;

CREATE TABLE freeplay(
	groupid INTEGER NOT NULL REFERENCES studentgroup(id) ON UPDATE CASCADE ON DELETE CASCADE,
	mapuuid TEXT NOT NULL,
	mission TEXT NOT NULL DEFAULT '',
	UNIQUE(groupid, mapuuid, mission)
);

INSERT INTO freeplay(groupid, mapuuid) SELECT groupid, mapuuid FROM freeplayT;

DROP TABLE freeplayT;
