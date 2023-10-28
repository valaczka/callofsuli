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
