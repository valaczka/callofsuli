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

#include "libtiled/map.h"
#include "libtiled/objectgroup.h"


class GameScene;

class GameTerrain
{
	Q_GADGET

public:
	GameTerrain(const QString &filename = "");
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


	bool isValid() const;
	bool loadMapFromFile(QString filename);
	bool loadMap(const QString &terrain, const int &level);

	bool loadObjectLayers();

	int width() const;
	int height() const;

	Tiled::Map *map() const;

	const QVector<EnemyData> &enemies() const;
	const QVector<int> &blocks() const;
	const QVector<ObjectData> &objects() const;
	const QVector<PlayerPositionData> &playerPositions() const;
	QVector<ObjectData> objects(const ObjectType &type) const;

	QVector<ObjectData> fires() const { return objects(Fire); }
	QVector<ObjectData> fences() const { return objects(Fence); }
	QVector<ObjectData> teleports() const { return objects(Teleport); }


	PlayerPositionData defaultPlayerPosition() const;



private:

	void readEnemyLayer(Tiled::ObjectGroup *layer);
	void readObjectLayer(Tiled::ObjectGroup *layer);
	void readPlayerLayer(Tiled::ObjectGroup *layer);
	void readItemLayer(Tiled::ObjectGroup *layer);

	bool m_isValid = false;
	std::unique_ptr<Tiled::Map> m_map;

	QVector<EnemyData> m_enemies;
	QVector<ObjectData> m_objects;
	QVector<int> m_blocks;
	QVector<PlayerPositionData> m_playerPositions;

	int m_fireCount = 0;
	int m_fenceCount = 0;
	int m_teleportCount = 0;
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

#endif // GAMETERRAIN_H
