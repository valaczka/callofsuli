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

#include "gameobject.h"


class GameEnemyData;
class GameLadder;
class GameEnemy;

class GameBlock : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QList<GameEnemyData *> enemies READ enemies WRITE setEnemies NOTIFY enemiesChanged)
	Q_PROPERTY(QList<GameLadder *> ladders READ ladders WRITE setLadders NOTIFY laddersChanged)
	Q_PROPERTY(QList<QPointF> playerPositions READ playerPositions WRITE setPlayerPositions NOTIFY playerPositionsChanged)
	Q_PROPERTY(bool completed READ completed WRITE setCompleted NOTIFY completedChanged)


public:
	explicit GameBlock(QObject *parent = nullptr);
	virtual ~GameBlock();

	void addEnemy(GameEnemyData *enemy);
	void addPlayerPosition(const QPointF &point);
	Box2DBox *addPlayerPosition(const QPoint &point, QQuickItem *parent);
	void addLadder(GameLadder *ladder);

	QList<GameEnemyData *> enemies() const { return m_enemies; }
	QList<GameLadder *> ladders() const { return m_ladders; }
	QList<QPointF> playerPositions() const { return m_playerPositions; }

	bool completed() const { return m_completed; }


public slots:
	void setEnemies(QList<GameEnemyData *> enemies);
	void setLadders(QList<GameLadder *> ladders);
	void setCompleted(bool completed);
	void setPlayerPositions(QList<QPointF> playerPositions);
	void recalculateActiveEnemies();
	void activateLadders();


signals:
	void enemiesChanged(QList<GameEnemyData *> enemies);
	void laddersChanged(QList<GameLadder *> ladders);
	void playerPositionsChanged(QList<QPointF> playerPositions);
	void completedChanged(bool completed);


private:
	QList<GameEnemyData *> m_enemies;
	QList<GameLadder *> m_ladders;
	QList<QPointF> m_playerPositions;
	bool m_completed;
};

#endif // GAMEBLOCK_H
