PRAGMA foreign_keys = OFF;

----------------------------------
--- Offline permit
----------------------------------

CREATE TABLE permit(
	id INTEGER NOT NULL PRIMARY KEY,
	username TEXT NOT NULL REFERENCES user(username) ON UPDATE CASCADE ON DELETE CASCADE,
	campaignid INTEGER REFERENCES campaign(id) ON UPDATE CASCADE ON DELETE CASCADE,
	device TEXT NOT NULL,
	anchor BLOB NOT NULL,
	step INTEGER NOT NULL,
	expected BLOB NOT NULL,
	UNIQUE (username, campaignid, device)
);

