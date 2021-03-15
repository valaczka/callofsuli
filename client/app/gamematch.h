/*
 * ---- Call of Suli ----
 *
 * gamedata.h
 *
 * Created on: 2020. 12. 22.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GameData
 *
 *  This file is part of Call of Suli.
 *
 *  Call of Suli is free software: you can redistribute it and/or modify
 *  it under the terms of the MIT license.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef GAMEMATCH_H
#define GAMEMATCH_H

#include <QObject>
#include "gamemap.h"

class GameMatch : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QByteArray missionUuid READ missionUuid WRITE setMissionUuid NOTIFY missionUuidChanged)
	Q_PROPERTY(QString playerCharacter READ playerCharacter WRITE setPlayerCharacter NOTIFY playerCharacterChanged)
	Q_PROPERTY(QString terrain READ terrain WRITE setTerrain NOTIFY terrainChanged)
	Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
	Q_PROPERTY(int level READ level WRITE setLevel NOTIFY levelChanged)
	Q_PROPERTY(int startHp READ startHp WRITE setStartHp NOTIFY startHpChanged)
	Q_PROPERTY(int startBlock READ startBlock WRITE setStartBlock NOTIFY startBlockChanged)
	Q_PROPERTY(QString bgImage READ bgImage WRITE setBgImage NOTIFY bgImageChanged)
	Q_PROPERTY(QString imageDbName READ imageDbName WRITE setImageDbName NOTIFY imageDbNameChanged)
	Q_PROPERTY(int duration READ duration WRITE setDuration NOTIFY durationChanged)

	Q_PROPERTY(int gameId READ gameId WRITE setGameId NOTIFY gameIdChanged)
	Q_PROPERTY(int xp READ xp WRITE setXP NOTIFY xpChanged)
	Q_PROPERTY(int baseXP READ baseXP WRITE setBaseXP NOTIFY baseXPChanged)


public:
	explicit GameMatch(GameMap *gameMap, QObject *parent = nullptr);
	explicit GameMatch(GameMap::MissionLevel *missionLevel, GameMap *gameMap, QObject *parent = nullptr);
	~GameMatch();

	GameMap::MissionLevel *missionLevel() const;

	QString playerCharacter() const { return m_playerCharacter; }
	QString terrain() const { return m_terrain; }
	int level() const { return m_level; }
	int startHp() const { return m_startHp; }
	int startBlock() const { return m_startBlock; }
	QString bgImage() const;
	QString imageDbName() const { return m_imageDbName; }
	QString name() const { return m_name; }
	int duration() const { return m_duration; }
	GameMap *gameMap() const { return m_gameMap; }
	QByteArray missionUuid() const { return m_missionUuid; }
	int gameId() const { return m_gameId; }
	int xp() const { return m_xp; }
	int baseXP() const { return m_baseXP; }

	int elapsedTime() const { return m_elapsedTime; }
	void setElapsedTime(int elapsedTime) { m_elapsedTime = elapsedTime; }

public slots:
	void addXP(const qreal &factor);
	void setDeleteGameMap(bool deleteGameMap);
	void setPlayerCharacter(QString playerCharacter);
	void setTerrain(QString terrain);
	void setLevel(int level);
	void setStartHp(int startHp);
	void setStartBlock(int startBlock);
	void setBgImage(QString bgImage);
	void setImageDbName(QString imageDbName);
	void setName(QString name);
	void setDuration(int duration);
	void setMissionUuid(QByteArray missionUuid);
	void setGameId(int gameId);
	void setXP(int xp);
	void setBaseXP(int baseXP);

signals:
	void gameLose(const QString &uuid, const int level);
	void gameWin(const QString &uuid, const int level);

	void playerCharacterChanged(QString playerCharacter);
	void terrainChanged(QString terrain);
	void levelChanged(int level);
	void startHpChanged(int startHp);
	void startBlockChanged(int startBlock);
	void bgImageChanged(QString bgImage);
	void imageDbNameChanged(QString imageDbName);
	void nameChanged(QString name);
	void durationChanged(int duration);
	void missionUuidChanged(QByteArray missionUuid);
	void gameIdChanged(int gameId);
	void xpChanged(int xp);
	void baseXPChanged(int baseXP);

private:
	GameMap *m_gameMap;
	GameMap::MissionLevel *m_missionLevel;
	bool m_deleteGameMap;

	QString m_playerCharacter;
	QString m_terrain;
	int m_level;
	int m_startHp;
	int m_startBlock;
	QString m_bgImage;
	QString m_imageDbName;
	QString m_name;
	int m_duration;
	QByteArray m_missionUuid;
	int m_gameId;
	int m_xp;
	int m_baseXP;
	int m_elapsedTime;
};



#endif // GAMEDATA_H
