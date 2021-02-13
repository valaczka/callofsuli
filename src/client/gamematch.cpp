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
{
	qDebug() << "GAME MATCH CREATED" << this;
	setPlayerCharacter("character3");
}


/**
 * @brief GameMatch::GameMatch
 * @param missionLevel
 * @param parent
 */

GameMatch::GameMatch(GameMap::MissionLevel *missionLevel, GameMap *gameMap, QObject *parent)
	: QObject(parent)
	, m_gameMap(gameMap)
	, m_missionLevel(missionLevel)
	, m_deleteGameMap(false)
	, m_level(0)
	, m_missionUuid()
	, m_gameId(-1)
	, m_xp(0)
	, m_baseXP(0)
{
	Q_ASSERT(missionLevel);

	qDebug() << "GAME MATCH CREATED" << this;

	setPlayerCharacter("character3");

	setMissionUuid(missionLevel->mission()->uuid());
	setName(missionLevel->mission()->name());
	setLevel(missionLevel->level());
	setTerrain(missionLevel->terrain());
	setStartHp(missionLevel->startHP());
	setDuration(missionLevel->duration());
	setStartBlock(missionLevel->startBlock());

	QString imageFolder = missionLevel->imageFolder();
	QString imageFile = missionLevel->imageFile();

	if (!imageFolder.isEmpty() && !imageFile.isEmpty())
		setBgImage(imageFolder+"/"+imageFile);

}


/**
 * @brief GameMatch::~GameMatch
 */

GameMatch::~GameMatch()
{
	if (m_deleteGameMap && m_gameMap)
		delete m_gameMap;

	qDebug() << "GAME MATCH DESTROYED" << this;
}


/**
 * @brief GameMatch::missionLevel
 * @return
 */

GameMap::MissionLevel* GameMatch::missionLevel() const
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
	else
		return "image://"+m_imageDbName+"/"+m_bgImage;
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

void GameMatch::setStartBlock(int startBlock)
{
	if (m_startBlock == startBlock)
		return;

	m_startBlock = startBlock;
	emit startBlockChanged(m_startBlock);
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

void GameMatch::setMissionUuid(QByteArray missionUuid)
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


