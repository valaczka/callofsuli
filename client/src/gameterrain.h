/*
 * ---- Call of Suli ----
 *
 * gameterrain.h
 *
 * Created on: 2020. 11. 23.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GameTerrain
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

#ifndef GAMETERRAIN_H
#define GAMETERRAIN_H

#include "qobjectdefs.h"
#include "qpoint.h"

#include "gamepickable.h"

class GameScene;

class GameTerrain
{
	Q_GADGET

public:
	GameTerrain();
	virtual ~GameTerrain();


	// Enemies

	enum EnemyType {
		EnemyInvalid,
		EnemySoldier,
		EnemySniper,
		EnemyOther
	};

	Q_ENUM(EnemyType);

	struct EnemyData;


	// Objects


	enum ObjectType {
		Invalid,
		Fire,
		Fence,
		Teleport
	};

	struct ObjectData;


	// Player position

	struct PlayerPositionData;


	// Pickables

	struct PickableData;


	bool isValid() const;

	const QVector<EnemyData> &enemies() const;
	const QVector<int> &blocks() const;
	const QVector<ObjectData> &objects() const;
	const QVector<PlayerPositionData> &playerPositions() const;
	const QVector<PickableData> &pickables() const;

	QVector<ObjectData> objects(const ObjectType &type) const;

	QVector<ObjectData> fires() const { return objects(Fire); }
	QVector<ObjectData> fences() const { return objects(Fence); }
	QVector<ObjectData> teleports() const { return objects(Teleport); }

	PlayerPositionData defaultPlayerPosition() const;

	static void reloadAvailableTerrains();
	static const QVector<GameTerrain> &availableTerrains();
	static bool terrainAvailable(const QString &name, const int &level);
	static bool terrainAvailable(const QString &missionLevelName);
	static GameTerrain terrain(const QString &missionLevelName);

	QJsonObject toJsonObject() const;
	static GameTerrain fromJsonObject(const QJsonObject &object);

	const QString &name() const;
	void setName(const QString &newName);

	int level() const;
	void setLevel(int newLevel);

	const QString &displayName() const;
	void setDisplayName(const QString &newDisplayName);

	const QString &backgroundImage() const;
	void setBackgroundImage(const QString &newBackgroundImage);

	const QString &backgroundMusic() const;
	void setBackgroundMusic(const QString &newBackgroundMusic);

protected:
	static QVector<GameTerrain> m_availableTerrains;

	bool m_isValid = false;
	QString m_name;
	QString m_displayName;
	QString m_backgroundImage;
	QString m_backgroundMusic;
	int m_level = 0;

	QVector<EnemyData> m_enemies;
	QVector<ObjectData> m_objects;
	QVector<int> m_blocks;
	QVector<PlayerPositionData> m_playerPositions;
	QVector<PickableData> m_pickables;
};



/**
 * @brief The GameTerrain::EnemyData class
 */

struct GameTerrain::EnemyData {
	Q_GADGET

	Q_PROPERTY(QRectF rect MEMBER rect)
	Q_PROPERTY(EnemyType type MEMBER type)
	Q_PROPERTY(int block MEMBER block)

public:
	EnemyType type = EnemyInvalid;
	QRectF rect;
	int block = -1;
};



/**
 * @brief The GameScene::PreviewData struct
 */

struct GameTerrain::ObjectData {
	Q_GADGET

	Q_PROPERTY(QPointF point MEMBER point)
	Q_PROPERTY(ObjectType type MEMBER type)

public:
	QPointF point;
	ObjectType type = Invalid;
};


/**
 * @brief The GameTerrain::PlayerPositionData class
 */

struct GameTerrain::PlayerPositionData {
	Q_GADGET

	Q_PROPERTY(QPointF point MEMBER point)
	Q_PROPERTY(int block MEMBER block)
	Q_PROPERTY(bool start MEMBER start)

public:
	QPointF point;
	int block = -1;
	bool start = false;
};






/**
 * @brief The GameScene::PreviewData struct
 */

struct GameTerrain::PickableData {
public:
	QPointF point;
	GamePickable::GamePickableData data;
};


#endif // GAMETERRAIN_H
