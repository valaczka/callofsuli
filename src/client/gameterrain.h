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

#include <QObject>

#include "gameenemydata.h"
#include "gameladder.h"
#include "gameblock.h"
#include "tiledpaintedlayer.h"

class GameTerrain : public QObject
{
	Q_OBJECT

public:
	explicit GameTerrain(QList<TiledPaintedLayer *> *tiledLayers = nullptr, QQuickItem *tiledLayersParent = nullptr, QObject *parent = nullptr);
	~GameTerrain();

	QList<GameEnemyData *> enemies() const { return m_enemies; }
	QList<GameLadder *> ladders() const { return m_ladders; }
	QMap<int, GameBlock *> blocks() const { return m_blocks; }
	QList<QRectF> groundObjects() const { return m_groundObjects; }

	QString tmxFile() const { return m_tmxFile; }
	Tiled::Map *map() const { return m_map; }

	Q_INVOKABLE bool loadTmxFile(const QString &filename);
	Q_INVOKABLE GameBlock *getBlock(const int &num);

	QList<TiledPaintedLayer *> *tiledLayers() const { return m_tiledLayers; }
	void setTiledLayers(QList<TiledPaintedLayer *> *tiledLayers);

	QQuickItem *tiledLayersParent() const { return m_tiledLayersParent; }
	void setTiledLayersParent(QQuickItem *tiledLayersParent);


private:
	bool loadMap();
	void loadLayers();
	void loadEnemyLayer(Tiled::Layer *layer);
	void loadGroundLayer(Tiled::Layer *layer);
	void loadPlayerLayer(Tiled::Layer *layer);
	void loadLadderLayer(Tiled::Layer *layer);

	QList<GameEnemyData *> m_enemies;
	QList<GameLadder *> m_ladders;
	QMap<int, GameBlock *> m_blocks;
	QList<QRectF> m_groundObjects;

	QString m_tmxFile;
	Tiled::Map *m_map;

	QList<TiledPaintedLayer *> *m_tiledLayers;
	QQuickItem *m_tiledLayersParent;

};

#endif // GAMETERRAIN_H
