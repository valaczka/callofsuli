PRAGMA foreign_keys = OFF;

----------------------------------
--- Campaign
----------------------------------

ALTER TABLE campaign ADD COLUMN archived BOOL NOT NULL DEFAULT FALSE;

----------------------------------
--- Wallet (remove bullets)
----------------------------------

CREATE TEMPORARY TABLE tmpBulletTable AS
	SELECT username, name, SUM(amount) AS amount FROM wallet
		WHERE expiry IS NULL OR expiry>datetime('now') AND type=4
		GROUP BY username, name;

DELETE FROM wallet WHERE type=4;

INSERT INTO wallet(username, type, name, amount)
	SELECT username, 3, 'longbow', amount FROM tmpBulletTable WHERE name='fireball';

INSERT INTO wallet(username, type, name, amount)
	SELECT username, 3, 'shortbow', amount FROM tmpBulletTable WHERE name='arrow';

UPDATE wallet SET expiry=NULL where type=3;

