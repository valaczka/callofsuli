/*
 * ---- Call of Suli ----
 *
 * gameblock.h
 *
 * Created on: 2020. 10. 24.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GameBlock
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

#ifndef GAMEBLOCK_H
#define GAMEBLOCK_H

#include <QMap>
#include <QPoint>
#include <QObject>

#include "gameenemy.h"
#include "gameladder.h"

class GameBlock : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QList<GameEnemy *> enemies READ enemies WRITE setEnemies NOTIFY enemiesChanged)
	Q_PROPERTY(QMap<int, QPoint> playerPosition READ playerPosition NOTIFY playerPositionChanged)
	Q_PROPERTY(QList<GameLadder *> ladders READ ladders WRITE setLadders NOTIFY laddersChanged)


public:
	explicit GameBlock(QObject *parent = nullptr);
	~GameBlock();

	void addEnemy(GameEnemy *enemy);
	void addPlayerPosition(const int &blockFrom, const QPoint &point);
	void addLadder(GameLadder *ladder);

	QList<GameEnemy *> enemies() const { return m_enemies; }
	QMap<int, QPoint> playerPosition() const { return m_playerPosition; }

	QList<GameLadder *> ladders() const { return m_ladders; }

public slots:
	void setEnemies(QList<GameEnemy *> enemies);
	void setLadders(QList<GameLadder *> ladders);

signals:
	void enemiesChanged(QList<GameEnemy *> enemies);
	void playerPositionChanged(QMap<int, QPoint> playerPosition);
	void laddersChanged(QList<GameLadder *> ladders);

private:
	QList<GameEnemy *> m_enemies;
	QMap<int, QPoint> m_playerPosition;
	QList<GameLadder *> m_ladders;
};

#endif // GAMEBLOCK_H
