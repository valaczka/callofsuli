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
#include <QJsonArray>
#include "gamemap.h"
#include "gamemapeditor.h"

class GameMatch : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QString missionUuid READ missionUuid WRITE setMissionUuid NOTIFY missionUuidChanged)
	Q_PROPERTY(QString playerCharacter READ playerCharacter WRITE setPlayerCharacter NOTIFY playerCharacterChanged)
	Q_PROPERTY(QString terrain READ terrain WRITE setTerrain NOTIFY terrainChanged)
	Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
	Q_PROPERTY(int level READ level WRITE setLevel NOTIFY levelChanged)
	Q_PROPERTY(int startHp READ startHp WRITE setStartHp NOTIFY startHpChanged)
	Q_PROPERTY(QString bgImage READ bgImage WRITE setBgImage NOTIFY bgImageChanged)
	Q_PROPERTY(QString imageDbName READ imageDbName WRITE setImageDbName NOTIFY imageDbNameChanged)
	Q_PROPERTY(int duration READ duration WRITE setDuration NOTIFY durationChanged)
	Q_PROPERTY(bool deathmatch READ deathmatch WRITE setDeathmatch NOTIFY deathmatchChanged)
	Q_PROPERTY(bool isFlawless READ isFlawless WRITE setIsFlawless NOTIFY isFlawlessChanged)

	Q_PROPERTY(int gameId READ gameId WRITE setGameId NOTIFY gameIdChanged)
	Q_PROPERTY(int xp READ xp WRITE setXP NOTIFY xpChanged)
	Q_PROPERTY(int baseXP READ baseXP WRITE setBaseXP NOTIFY baseXPChanged)

	Q_PROPERTY(int water READ water WRITE setWater NOTIFY waterChanged)
	Q_PROPERTY(int pliers READ pliers WRITE setPliers NOTIFY pliersChanged)
	Q_PROPERTY(int camouflage READ camouflage WRITE setCamouflage NOTIFY camouflageChanged)
	Q_PROPERTY(int teleporter READ teleporter WRITE setTeleporter NOTIFY teleporterChanged)

	Q_PROPERTY(GameMode mode READ mode WRITE setMode NOTIFY modeChanged)

	Q_PROPERTY(bool skipPreview READ skipPreview WRITE setSkipPreview NOTIFY skipPreviewChanged)


public:
	explicit GameMatch(GameMap *gameMap, QObject *parent = nullptr);
	explicit GameMatch(GameMapMissionLevel *missionLevel, GameMap *gameMap, QObject *parent = nullptr);
	explicit GameMatch(GameMapEditorMissionLevel *missionLevel, GameMap *gameMap, QObject *parent = nullptr);
	virtual ~GameMatch();

	enum GameMode {
		ModeNormal,
		ModeLite,
		ModeExam
	};

	Q_ENUM(GameMode);

	struct Statistics {
		uint index;
		QString objective;
		bool success;
		int elapsed;

		Statistics(const QString &m_objective, const bool &m_success, const int &m_elapsed) :
			index(++m_index),
			objective(m_objective),
			success(m_success),
			elapsed(m_elapsed)
		{}

		friend inline bool operator== (const Statistics &b1, const Statistics &b2) {
			return (b1.index == b2.index);
		}

	private:
		static uint m_index;
	};

	GameMapMissionLevel *missionLevel() const;

	QString playerCharacter() const { return m_playerCharacter; }
	QString terrain() const { return m_terrain; }
	int level() const { return m_level; }
	int startHp() const { return m_startHp; }
	QString bgImage() const;
	QString imageDbName() const { return m_imageDbName; }
	QString name() const { return m_name; }
	int duration() const { return m_duration; }
	GameMap *gameMap() const { return m_gameMap; }
	QString missionUuid() const { return m_missionUuid; }
	bool deathmatch() const { return m_deathmatch; }
	int gameId() const { return m_gameId; }
	int xp() const { return m_xp; }
	int baseXP() const { return m_baseXP; }

	int elapsedTime() const { return m_elapsedTime; }
	void setElapsedTime(int elapsedTime) { m_elapsedTime = elapsedTime; }

	void addStatistics(const Statistics &data);
	void addStatistics(const QString &objective, const bool &success, const int &elapsed);
	QJsonArray takeStatistics();

	void previewCompleted();

	int water() const;
	void setWater(int newWater);

	int pliers() const;
	void setPliers(int newPliers);

	const GameMode &mode() const;
	void setMode(const GameMode &newMode);

	bool skipPreview() const;
	void setSkipPreview(bool newSkipPreview);

	bool isFlawless() const;
	void setIsFlawless(bool newIsFlawless);

	int camouflage() const;
	void setCamouflage(int newGlasses);

	int teleporter() const;
	void setTeleporter(int newTeleporter);

public slots:
	bool check(QString *errorString);
	void addXP(const qreal &factor);
	void setDeleteGameMap(bool deleteGameMap);
	void setPlayerCharacter(QString playerCharacter);
	void setTerrain(QString terrain);
	void setLevel(int level);
	void setStartHp(int startHp);
	void setBgImage(QString bgImage);
	void setImageDbName(QString imageDbName);
	void setName(QString name);
	void setDuration(int duration);
	void setMissionUuid(QString missionUuid);
	void setGameId(int gameId);
	void setXP(int xp);
	void setBaseXP(int baseXP);
	void setDeathmatch(bool deathmatch);

signals:
	void gameLose();
	void gameWin();

	void playerCharacterChanged(QString playerCharacter);
	void terrainChanged(QString terrain);
	void levelChanged(int level);
	void startHpChanged(int startHp);
	void bgImageChanged(QString bgImage);
	void imageDbNameChanged(QString imageDbName);
	void nameChanged(QString name);
	void durationChanged(int duration);
	void missionUuidChanged(QString missionUuid);
	void gameIdChanged(int gameId);
	void xpChanged(int xp);
	void baseXPChanged(int baseXP);
	void deathmatchChanged(bool deathmatch);
	void waterChanged();
	void pliersChanged();
	void modeChanged();
	void skipPreviewChanged();
	void isFlawlessChanged();
	void camouflageChanged();
	void teleporterChanged();

private:
	GameMap *m_gameMap;
	GameMapMissionLevel *m_missionLevel;
	bool m_deleteGameMap;

	QString m_playerCharacter;
	QString m_terrain;
	int m_level;
	int m_startHp;
	QString m_bgImage;
	QString m_imageDbName;
	QString m_name;
	int m_duration;
	QString m_missionUuid;
	int m_gameId;
	int m_xp;
	int m_baseXP;
	int m_elapsedTime;
	bool m_deathmatch;
	QVector<Statistics> m_statData;
	int m_water;
	int m_pliers;
	GameMode m_mode;
	bool m_skipPreview;
	bool m_isFlawless;
	int m_camouflage;
	int m_teleporter;
};



#endif // GAMEDATA_H
