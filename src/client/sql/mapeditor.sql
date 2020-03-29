PRAGMA foreign_keys = ON;

CREATE TABLE mapeditor (
	id INTEGER PRIMARY KEY,
	serverid INTEGER,
	mapid INTEGER,
	originalFile TEXT,
	uuid TEXT,
	timeCreated TEX
);
