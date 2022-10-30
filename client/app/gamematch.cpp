/*
 * ---- Call of Suli ----
 *
 * gamedata.cpp
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

#include "gamematch.h"
#include "cosclient.h"

uint GameMatch::Statistics::m_index = 0;

GameMatch::GameMatch(GameMap *gameMap, QObject *parent)
	: QObject(parent)
	, m_gameMap(gameMap)
	, m_missionLevel(nullptr)
	, m_deleteGameMap(false)
	, m_level(0)
	, m_missionUuid()
	, m_gameId(-1)
	, m_xp(0)
	, m_baseXP(0)
	, m_elapsedTime(-1)
	, m_deathmatch(false)
	, m_statData()
	, m_water(0)
	, m_pliers(0)
	, m_mode(ModeNormal)
	, m_skipPreview(false)
	, m_isFlawless(false)
	, m_camouflage(0)
	, m_teleporter(0)
{
	setPlayerCharacter("default");
}


/**
 * @brief GameMatch::GameMatch
 * @param missionLevel
 * @param parent
 */

GameMatch::GameMatch(GameMapMissionLevel *missionLevel, GameMap *gameMap, QObject *parent)
	: QObject(parent)
	, m_gameMap(gameMap)
	, m_missionLevel(missionLevel)
	, m_deleteGameMap(false)
	, m_level(0)
	, m_missionUuid()
	, m_gameId(-1)
	, m_xp(0)
	, m_baseXP(0)
	, m_elapsedTime(-1)
	, m_deathmatch(false)
	, m_water(0)
	, m_pliers(0)
	, m_mode(ModeNormal)
	, m_skipPreview(false)
	, m_isFlawless(false)
	, m_camouflage(0)
	, m_teleporter(0)
{
	Q_ASSERT(missionLevel);

	setPlayerCharacter("default");

	setMissionUuid(missionLevel->mission()->uuid());
	setName(missionLevel->mission()->name());
	setLevel(missionLevel->level());
	setTerrain(missionLevel->terrain());
	setStartHp(missionLevel->startHP());
	setDuration(missionLevel->duration());
	setIsFlawless(true);

	QString image = missionLevel->image();

	if (!image.isEmpty())
		setBgImage(image);

	const QString &uuidLevel = QString("%1#%2").arg(m_missionUuid).arg(m_level);

	QStringList l = Client::clientInstance()->getServerSetting("game/skipPreview", QStringList()).toStringList();
	if (l.contains(uuidLevel))
		setSkipPreview(true);

}


/**
 * @brief GameMatch::GameMatch
 * @param missionLevel
 * @param gameMap
 * @param parent
 */

GameMatch::GameMatch(GameMapEditorMissionLevel *missionLevel, GameMap *gameMap, QObject *parent)
	: QObject(parent)
	, m_gameMap(gameMap)
	, m_missionLevel(nullptr)
	, m_deleteGameMap(false)
	, m_level(0)
	, m_missionUuid()
	, m_gameId(-1)
	, m_xp(0)
	, m_baseXP(0)
	, m_elapsedTime(-1)
	, m_deathmatch(false)
	, m_water(0)
	, m_pliers(0)
	, m_mode(ModeNormal)
	, m_skipPreview(false)
	, m_isFlawless(false)
	, m_camouflage(0)
	, m_teleporter(0)
{
	setPlayerCharacter("default");

	setMissionUuid(missionLevel->editorMission()->uuid());
	setName(missionLevel->editorMission()->name());
	setLevel(missionLevel->level());
	setTerrain(missionLevel->terrain());
	setStartHp(missionLevel->startHP());
	setDuration(missionLevel->duration());
	setIsFlawless(true);

	QString image = missionLevel->image();

	if (!image.isEmpty())
		setBgImage(image);

	const QString &uuidLevel = QString("%1#%2").arg(m_missionUuid).arg(m_level);

	QStringList l = Client::clientInstance()->getServerSetting("game/skipPreview", QStringList()).toStringList();
	if (l.contains(uuidLevel))
		setSkipPreview(true);
}



/**
 * @brief GameMatch::~GameMatch
 */

GameMatch::~GameMatch()
{
	if (m_deleteGameMap && m_gameMap)
		delete m_gameMap;
}


/**
 * @brief GameMatch::missionLevel
 * @return
 */

GameMapMissionLevel *GameMatch::missionLevel() const
{
	if (m_missionLevel)
		return m_missionLevel;

	if (!m_gameMap)
		return nullptr;

	if (m_missionUuid.isEmpty() || !m_level)
		return nullptr;

	return m_gameMap->missionLevel(m_missionUuid, m_level);
}


/**
 * @brief GameMatch::bgImage
 * @return
 */

QString GameMatch::bgImage() const
{
	if (m_bgImage.isEmpty() || m_imageDbName.isEmpty())
		return "qrc:/internal/game/bg.png";
	else if (m_bgImage.startsWith("qrc:/"))
		return m_bgImage;
	else
		return "image://"+m_imageDbName+"/"+m_bgImage;
}


/**
 * @brief GameMatch::addStatistics
 * @param data
 */

void GameMatch::addStatistics(const GameMatch::Statistics &data)
{
	m_statData.append(data);
}


/**
 * @brief GameMatch::addStatistics
 * @param map
 * @param objective
 * @param success
 * @param elapsed
 */

void GameMatch::addStatistics(const QString &objective, const bool &success, const int &elapsed)
{
	m_statData.append(Statistics(objective, success, elapsed));
}


/**
 * @brief GameMatch::takeStatistics
 * @return
 */

QJsonArray GameMatch::takeStatistics()
{
	QJsonArray r;

	foreach (Statistics s, m_statData) {
		QJsonObject o;
		o["map"] = m_gameMap ? m_gameMap->uuid() : "";
		o["objective"] = s.objective;
		o["success"] = s.success;
		o["elapsed"] = s.elapsed;
		r.append(o);
	}

	m_statData.clear();

	return r;
}


/**
 * @brief GameMatch::previewCompleted
 */

void GameMatch::previewCompleted()
{
	if (m_missionUuid.isEmpty())
		return;

	const QString &uuidLevel = QString("%1#%2").arg(m_missionUuid).arg(m_level);

	QStringList l = Client::clientInstance()->getServerSetting("game/skipPreview", QStringList()).toStringList();

	if (!l.contains(uuidLevel)) {
		l.append(uuidLevel);
		Client::clientInstance()->setServerSetting("game/skipPreview", l);
		setSkipPreview(true);
	}
}



/**
 * @brief GameMatch::check
 * @return
 */

bool GameMatch::check(QString *errorString)
{
	GameMapMissionLevel *ml = missionLevel();

	if (!ml) {
		if (errorString)
			*errorString = tr("Érvénytelen küldetés!");
		return false;
	}

	if (!Client::terrainMap().contains(ml->terrain())) {
		if (errorString)
			*errorString = tr("Érvénytelen harcmező!");
		return false;
	}

	foreach(GameMapChapter *chapter, ml->chapters()) {
		foreach(GameMapObjective *objective, chapter->objectives()) {
			QString om = objective->module();

			if (!Client::moduleObjectiveList().contains(om)) {
				if (errorString)
					*errorString = tr("Érvénytelen modul: %1").arg(om);
				return false;
			}

			if (objective->storage()) {
				QString sm = objective->storage()->module();

				if (!Client::moduleStorageList().contains(sm)) {
					if (errorString)
						*errorString = tr("Érvénytelen modul: %1").arg(sm);
					return false;
				}
			}
		}
	}

	return true;
}


/**
 * @brief GameMatch::addXP
 * @param factor
 */

void GameMatch::addXP(const qreal &factor)
{
	setXP(m_xp+m_baseXP*factor);
}


/**
 * @brief GameMatch::setDeleteGameMap
 * @param deleteGameMap
 */

void GameMatch::setDeleteGameMap(bool deleteGameMap)
{
	m_deleteGameMap = deleteGameMap;
}

void GameMatch::setPlayerCharacter(QString playerCharacter)
{
	if (m_playerCharacter == playerCharacter)
		return;

	m_playerCharacter = playerCharacter;
	emit playerCharacterChanged(m_playerCharacter);
}

void GameMatch::setTerrain(QString terrain)
{
	if (m_terrain == terrain)
		return;

	m_terrain = terrain;
	emit terrainChanged(m_terrain);
}

void GameMatch::setLevel(int level)
{
	if (m_level == level)
		return;

	m_level = level;
	emit levelChanged(m_level);
}

void GameMatch::setStartHp(int startHp)
{
	if (m_startHp == startHp)
		return;

	m_startHp = startHp;
	emit startHpChanged(m_startHp);
}


void GameMatch::setBgImage(QString bgImage)
{
	if (m_bgImage == bgImage)
		return;

	m_bgImage = bgImage;
	emit bgImageChanged(m_bgImage);
}

void GameMatch::setImageDbName(QString imageDbName)
{
	if (m_imageDbName == imageDbName)
		return;

	m_imageDbName = imageDbName;
	emit imageDbNameChanged(m_imageDbName);
	emit bgImageChanged(bgImage());
}

void GameMatch::setName(QString name)
{
	if (m_name == name)
		return;

	m_name = name;
	emit nameChanged(m_name);
}

void GameMatch::setDuration(int duration)
{
	if (m_duration == duration)
		return;

	m_duration = duration;
	emit durationChanged(m_duration);
}

void GameMatch::setMissionUuid(QString missionUuid)
{
	if (m_missionUuid == missionUuid)
		return;

	m_missionUuid = missionUuid;
	emit missionUuidChanged(m_missionUuid);
}

void GameMatch::setGameId(int gameId)
{
	if (m_gameId == gameId)
		return;

	m_gameId = gameId;
	emit gameIdChanged(m_gameId);
}

void GameMatch::setXP(int xp)
{
	if (m_xp == xp)
		return;

	m_xp = xp;
	emit xpChanged(m_xp);
}

void GameMatch::setBaseXP(int baseXP)
{
	if (m_baseXP == baseXP)
		return;

	m_baseXP = baseXP;
	emit baseXPChanged(m_baseXP);
}

void GameMatch::setDeathmatch(bool deathmatch)
{
	if (m_deathmatch == deathmatch)
		return;

	m_deathmatch = deathmatch;
	emit deathmatchChanged(m_deathmatch);
}

const GameMatch::GameMode &GameMatch::mode() const
{
	return m_mode;
}

void GameMatch::setMode(const GameMode &newMode)
{
	if (m_mode == newMode)
		return;
	m_mode = newMode;
	emit modeChanged();
}







int GameMatch::water() const
{
	return m_water;
}

void GameMatch::setWater(int newWater)
{
	if (m_water == newWater)
		return;
	m_water = newWater;
	emit waterChanged();
}

int GameMatch::pliers() const
{
	return m_pliers;
}

void GameMatch::setPliers(int newPliers)
{
	if (m_pliers == newPliers)
		return;
	m_pliers = newPliers;
	emit pliersChanged();
}

bool GameMatch::skipPreview() const
{
	return m_skipPreview;
}

void GameMatch::setSkipPreview(bool newSkipPreview)
{
	if (m_skipPreview == newSkipPreview)
		return;
	m_skipPreview = newSkipPreview;
	emit skipPreviewChanged();
}

bool GameMatch::isFlawless() const
{
	return m_isFlawless;
}

void GameMatch::setIsFlawless(bool newIsFlawless)
{
	if (m_isFlawless == newIsFlawless)
		return;
	m_isFlawless = newIsFlawless;
	emit isFlawlessChanged();
}

int GameMatch::camouflage() const
{
	return m_camouflage;
}

void GameMatch::setCamouflage(int newGlasses)
{
	if (m_camouflage == newGlasses)
		return;
	m_camouflage = newGlasses;
	emit camouflageChanged();
}

int GameMatch::teleporter() const
{
	return m_teleporter;
}

void GameMatch::setTeleporter(int newTeleporter)
{
	if (m_teleporter == newTeleporter)
		return;
	m_teleporter = newTeleporter;
	emit teleporterChanged();
}
