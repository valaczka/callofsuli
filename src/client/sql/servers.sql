PRAGMA foreign_keys = ON;

CREATE TABLE server(
	id INTEGER PRIMARY KEY,
	name TEXT,
	host TEXT,
	port INTEGER NOT NULL DEFAULT 1 CHECK(port>0),
	ssl BOOL NOT NULL DEFAULT FALSE,
	username TEXT,
	session TEXT,
	cert BLOB
);


CREATE TABLE autoconnect(
	serverid INTEGER REFERENCES server(id) ON UPDATE CASCADE ON DELETE CASCADE
);
