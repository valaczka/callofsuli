PRAGMA foreign_keys = ON;

----------------------------------
--- Freeplay
----------------------------------

CREATE TABLE freeplay(
	groupid INTEGER NOT NULL REFERENCES studentgroup(id) ON UPDATE CASCADE ON DELETE CASCADE,
	mapuuid TEXT NOT NULL,
	UNIQUE(groupid, mapuuid)
);
