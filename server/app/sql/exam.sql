PRAGMA foreign_keys = ON;

CREATE TABLE exam(
	uuid TEXT NOT NULL PRIMARY KEY,
	mapuuid TEXT,
	owner TEXT,
	title TEXT,
	description TEXT,
	config TEXT,
	grading TEXT
);


CREATE TABLE content(
	examuuid TEXT NOT NULL REFERENCES exam(uuid) ON UPDATE CASCADE ON DELETE CASCADE,
	examid INTEGER NOT NULL,
	username TEXT NOT NULL,
	questions TEXT,
	correctAnswers TEXT,
	answers TEXT,
	result REAL,
	UNIQUE(examid, username)
);
