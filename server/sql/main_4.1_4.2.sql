PRAGMA foreign_keys = ON;

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
	timestamp TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
	UNIQUE(username, type, name)
);


CREATE TABLE currency(
	id INTEGER NOT NULL PRIMARY KEY,
	username TEXT NOT NULL REFERENCES user(username) ON UPDATE CASCADE ON DELETE CASCADE,
	amount INTEGER,
	gameid INTEGER REFERENCES game(id) ON UPDATE CASCADE ON DELETE SET NULL,
	timestamp TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP
);
