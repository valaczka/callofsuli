PRAGMA foreign_keys = ON;

ALTER TABLE auth ADD COLUMN oauthToken TEXT;

UPDATE system SET versionMajor=3, versionMinor=0;
