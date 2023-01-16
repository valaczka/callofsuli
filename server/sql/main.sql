PRAGMA foreign_keys = ON;

----------------------------------
--- System
----------------------------------

CREATE TABLE system(
	versionMajor INTEGER,
	versionMinor INTEGER,
	serverName TEXT,
	config TEXT
);


----------------------------------
--- Class
----------------------------------

CREATE TABLE class(
	id INTEGER NOT NULL PRIMARY KEY,
	name TEXT
);


CREATE TABLE classCode(
	classid INTEGER REFERENCES class(id) ON UPDATE CASCADE ON DELETE CASCADE,
	code TEXT,
	UNIQUE(classid)
);


----------------------------------
--- Users & Auth
----------------------------------

CREATE TABLE user(
	username TEXT NOT NULL PRIMARY KEY,
	familyName TEXT,
	givenName TEXT,
	active BOOL NOT NULL DEFAULT false,
	classid INTEGER REFERENCES class(id) ON UPDATE CASCADE ON DELETE SET NULL,
	isTeacher BOOL NOT NULL DEFAULT false,
	isAdmin BOOL NOT NULL DEFAULT false,
	isPanel BOOL NOT NULL DEFAULT false,
	nickname TEXT,
	character TEXT,
	picture TEXT,
	UNIQUE (username)
);

CREATE TABLE auth(
	username TEXT NOT NULL REFERENCES user(username) ON UPDATE CASCADE ON DELETE CASCADE,
	password TEXT,
	salt TEXT,
	oauth TEXT,
	UNIQUE (username)
);


----------------------------------
--- Ranks
----------------------------------

CREATE TABLE rank(
	id INTEGER NOT NULL PRIMARY KEY,
	level INTEGER NOT NULL CHECK(level>=0),
	sublevel INTEGER CHECK(sublevel>=0),
	xp INTEGER CHECK(xp>=0),
	name TEXT NOT NULL
);
