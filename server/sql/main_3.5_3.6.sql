PRAGMA foreign_keys = ON;

----------------------------------
--- Users
----------------------------------

CREATE TABLE extraRole(
	username TEXT NOT NULL REFERENCES user(username) ON UPDATE CASCADE ON DELETE CASCADE,
	role TEXT NOT NULL
);


----------------------------------
--- Maps
----------------------------------

CREATE TABLE mapTag(
	id INTEGER NOT NULL PRIMARY KEY,
	tag TEXT NOT NULL,
	username TEXT NOT NULL REFERENCES user(username) ON UPDATE CASCADE ON DELETE CASCADE,
	parentId INTEGER REFERENCES mapTag(id) ON UPDATE CASCADE ON DELETE CASCADE
);

CREATE TABLE mapTagBind(
	mapuuid TEXT NOT NULL REFERENCES mapOwner(mapuuid) ON UPDATE CASCADE ON DELETE CASCADE,
	tagid INTEGER REFERENCES mapTag(id) ON UPDATE CASCADE ON DELETE CASCADE,
	UNIQUE(mapuuid, tagid)
);


----------------------------------
--- Grades
----------------------------------

CREATE TABLE grading(
	id INTEGER NOT NULL PRIMARY KEY,
	owner TEXT NOT NULL REFERENCES user(username) ON UPDATE CASCADE ON DELETE CASCADE,
	name TEXT,
	data TEXT
);


----------------------------------
--- Exam
----------------------------------

CREATE TABLE exam(
	id INTEGER NOT NULL PRIMARY KEY,
	groupid INTEGER NOT NULL REFERENCES studentgroup(id) ON UPDATE CASCADE ON DELETE CASCADE,
	mode INTEGER NOT NULL DEFAULT 0,
	state INTEGER NOT NULL DEFAULT 0,
	mapuuid TEXT,
	description TEXT,
	timestamp INTEGER,
	engineData text
);

CREATE TABLE examContent(
	id INTEGER NOT NULL PRIMARY KEY,
	examid INTEGER NOT NULL REFERENCES exam(id) ON UPDATE CASCADE ON DELETE CASCADE,
	username TEXT NOT NULL REFERENCES user(username) ON UPDATE CASCADE ON DELETE CASCADE,
	data TEXT,
	result REAL,
	gradeid INTEGER REFERENCES grade(id) ON UPDATE CASCADE ON DELETE SET NULL,
	UNIQUE (examid, username)
);

CREATE TABLE examAnswer(
	id INTEGER NOT NULL PRIMARY KEY,
	contentid INTEGER NOT NULL REFERENCES examContent(id) ON UPDATE CASCADE ON DELETE CASCADE,
	answer TEXT,
	correction TEXT
);
