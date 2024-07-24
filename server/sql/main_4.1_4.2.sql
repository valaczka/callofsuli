PRAGMA foreign_keys = OFF;

----------------------------------
--- Wallet
----------------------------------

CREATE TABLE wallet(
	id INTEGER NOT NULL PRIMARY KEY,
	username TEXT NOT NULL REFERENCES user(username) ON UPDATE CASCADE ON DELETE CASCADE,
	type INTEGER,
	name TEXT,
	amount INTEGER,
	expiry TEXT,
	gameid INTEGER REFERENCES game(id) ON UPDATE CASCADE ON DELETE SET NULL,
	timestamp TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
	UNIQUE (username, gameid, type, name)
);


CREATE TABLE currency(
	id INTEGER NOT NULL PRIMARY KEY,
	username TEXT NOT NULL REFERENCES user(username) ON UPDATE CASCADE ON DELETE CASCADE,
	amount INTEGER,
	gameid INTEGER REFERENCES game(id) ON UPDATE CASCADE ON DELETE SET NULL,
	timestamp TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
	UNIQUE (username, gameid)
);




----------------------------------
--- Notifications
----------------------------------

CREATE TABLE notification(
	id INTEGER NOT NULL PRIMARY KEY,
	username TEXT NOT NULL REFERENCES user(username) ON UPDATE CASCADE ON DELETE CASCADE,
	type INTEGER NOT NULL,
	UNIQUE (username, type)
);

CREATE TABLE notificationSent(
	id INTEGER NOT NULL PRIMARY KEY,
	username TEXT NOT NULL REFERENCES user(username) ON UPDATE CASCADE ON DELETE CASCADE,
	type INTEGER NOT NULL,
	campaignid INTEGER REFERENCES campaign(id) ON UPDATE CASCADE ON DELETE CASCADE,
	timestamp TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP
);
