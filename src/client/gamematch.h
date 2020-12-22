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
#include "../common/gamemap.h"

class GameMatch : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QString playerCharacter READ playerCharacter WRITE setPlayerCharacter NOTIFY playerCharacterChanged)
	Q_PROPERTY(QString terrain READ terrain WRITE setTerrain NOTIFY terrainChanged)
	Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
	Q_PROPERTY(int level READ level WRITE setLevel NOTIFY levelChanged)
	Q_PROPERTY(int startHp READ startHp WRITE setStartHp NOTIFY startHpChanged)
	Q_PROPERTY(int startBlock READ startBlock WRITE setStartBlock NOTIFY startBlockChanged)
	Q_PROPERTY(QString bgImage READ bgImage WRITE setBgImage NOTIFY bgImageChanged)
	Q_PROPERTY(QString imageDbName READ imageDbName WRITE setImageDbName NOTIFY imageDbNameChanged)
	Q_PROPERTY(int duration READ duration WRITE setDuration NOTIFY durationChanged)


public:
	explicit GameMatch(GameMap *gameMap, QObject *parent = nullptr);
	~GameMatch();

	QString playerCharacter() const
	{
		return m_playerCharacter;
	}

	QString terrain() const
	{
		return m_terrain;
	}

	int level() const
	{
		return m_level;
	}

	int startHp() const
	{
		return m_startHp;
	}

	int startBlock() const
	{
		return m_startBlock;
	}

	QString bgImage() const
	{
		if (m_bgImage.isEmpty() || m_imageDbName.isEmpty())
			return "qrc:/internal/game/bg.png";
		else
			return "image://"+m_imageDbName+"/"+m_bgImage;
	}

	QString imageDbName() const
	{
		return m_imageDbName;
	}

	void setDeleteGameMap(bool deleteGameMap);



	QString name() const
	{
		return m_name;
	}

	int duration() const
	{
		return m_duration;
	}

public slots:
	void setPlayerCharacter(QString playerCharacter)
	{
		if (m_playerCharacter == playerCharacter)
			return;

		m_playerCharacter = playerCharacter;
		emit playerCharacterChanged(m_playerCharacter);
	}

	void setTerrain(QString terrain)
	{
		if (m_terrain == terrain)
			return;

		m_terrain = terrain;
		emit terrainChanged(m_terrain);
	}

	void setLevel(int level)
	{
		if (m_level == level)
			return;

		m_level = level;
		emit levelChanged(m_level);
	}

	void setStartHp(int startHp)
	{
		if (m_startHp == startHp)
			return;

		m_startHp = startHp;
		emit startHpChanged(m_startHp);
	}

	void setStartBlock(int startBlock)
	{
		if (m_startBlock == startBlock)
			return;

		m_startBlock = startBlock;
		emit startBlockChanged(m_startBlock);
	}

	void setBgImage(QString bgImage)
	{
		if (m_bgImage == bgImage)
			return;

		m_bgImage = bgImage;
		emit bgImageChanged(m_bgImage);
	}

	void setImageDbName(QString imageDbName)
	{
		if (m_imageDbName == imageDbName)
			return;

		m_imageDbName = imageDbName;
		emit imageDbNameChanged(m_imageDbName);
		emit bgImageChanged(bgImage());
	}



	void setName(QString name)
	{
		if (m_name == name)
			return;

		m_name = name;
		emit nameChanged(m_name);
	}

	void setDuration(int duration)
	{
		if (m_duration == duration)
			return;

		m_duration = duration;
		emit durationChanged(m_duration);
	}

signals:

	void playerCharacterChanged(QString playerCharacter);

	void terrainChanged(QString terrain);

	void levelChanged(int level);

	void startHpChanged(int startHp);

	void startBlockChanged(int startBlock);

	void bgImageChanged(QString bgImage);

	void imageDbNameChanged(QString imageDbName);


	void nameChanged(QString name);

	void durationChanged(int duration);

private:
	GameMap *m_gameMap;
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
};

#endif // GAMEDATA_H
