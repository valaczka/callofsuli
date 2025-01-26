PRAGMA foreign_keys = OFF;

----------------------------------
--- Campaign
----------------------------------

ALTER TABLE taskSuccess ADD COLUMN result REAL NOT NULL DEFAULT 1.0;

ALTER TABLE campaignResult ADD COLUMN maxPts INTEGER;

ALTER TABLE campaignResult ADD COLUMN progress REAL;
