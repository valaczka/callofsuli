/*
 * ---- Call of Suli ----
 *
 * gamesceneprivate.h
 *
 * Created on: 2020. 10. 25.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GameScenePrivate
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

#ifndef GAMESCENEPRIVATE_H
#define GAMESCENEPRIVATE_H

#include <QObject>
#include <QQuickItem>

#include "tmxmap.h"
#include "tiledpaintedlayer.h"

class CosGame;

class GameScene : public QQuickItem
{
	Q_OBJECT

	Q_PROPERTY(Tiled::Map * map READ map NOTIFY mapChanged)
	Q_PROPERTY(QList<TiledPaintedLayer *> tiledLayers READ tiledLayers NOTIFY tiledLayersChanged)
	Q_PROPERTY(CosGame * game READ game WRITE setGame NOTIFY gameChanged)

public:
	GameScene(QQuickItem *parent = 0);
	~GameScene();
	Tiled::Map * map() const { return m_map; }
	QList<TiledPaintedLayer *> tiledLayers() const { return m_tiledLayers; }
	CosGame * game() const { return m_game; }

	bool loadMap(const QString &source);
	void loadLayers();
	void loadGroundLayer(Tiled::Layer *layer);
	void loadEnemyLayer(Tiled::Layer *layer);
	void loadPlayerLayer(Tiled::Layer *layer);
	void loadLadderLayer(Tiled::Layer *layer);


public slots:
	void setTiledLayers(QList<TiledPaintedLayer *> tiledLayers);
	void setGame(CosGame * game);
	void loadScene(const QString &tmxFileName);

signals:
	void mapChanged(Tiled::Map * map);
	void tiledLayersChanged(QList<TiledPaintedLayer *> tiledLayers);
	void gameChanged(CosGame * game);

	void sceneLoadStarted(const QString &tmxFileName);
	void sceneLoaded();
	void sceneLoadFailed();



private:
	Tiled::Map * m_map;
	QList<TiledPaintedLayer *> m_tiledLayers;
	CosGame * m_game;
	bool m_sceneLoaded;
};




#endif // GAMESCENEPRIVATE_H
