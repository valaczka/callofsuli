PRAGMA foreign_keys = ON;

----------------------------------
--- Campaign
----------------------------------

CREATE TABLE campaignStudent(
	id INTEGER NOT NULL PRIMARY KEY,
	campaignid INTEGER NOT NULL REFERENCES campaign(id) ON UPDATE CASCADE ON DELETE CASCADE,
	username TEXT NOT NULL REFERENCES user(username) ON UPDATE CASCADE ON DELETE CASCADE
);
