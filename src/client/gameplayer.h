/*
 * ---- Call of Suli ----
 *
 * gameplayerprivate.h
 *
 * Created on: 2020. 10. 22.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GamePlayerPrivate
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

#ifndef GAMEPLAYERPRIVATE_H
#define GAMEPLAYERPRIVATE_H

#include "gameentity.h"


class GamePlayer : public GameEntity
{
	Q_OBJECT

	Q_PROPERTY(LadderMode ladderMode READ ladderMode WRITE setLadderMode NOTIFY ladderModeChanged)
	Q_PROPERTY(GameLadder * ladder READ ladder WRITE setLadder NOTIFY ladderChanged)
	Q_PROPERTY(GameEnemy * enemy READ enemy WRITE setEnemy NOTIFY enemyChanged)
	Q_PROPERTY(bool hasGun READ hasGun WRITE setHasGun NOTIFY hasGunChanged)

public:
	enum LadderMode {
		LadderUnavaliable,
		LadderUpAvailable,
		LadderDownAvailable,
		LadderClimb,
		LadderClimbFinish
	};

	Q_ENUM(LadderMode)

	GamePlayer(QQuickItem *parent = 0);
	~GamePlayer();

	void setQrcDir() override;
	void createFixtures() override;
	Q_INVOKABLE void ladderClimbUp();
	Q_INVOKABLE void ladderClimbDown();
	Q_INVOKABLE void ladderClimbFinish();
	Q_INVOKABLE void attackByGun();

	LadderMode ladderMode() const { return m_ladderMode; }
	GameLadder * ladder() const { return m_ladder; }
	GameEnemy * enemy() const { return m_enemy; }
	bool hasGun() const { return m_hasGun; }

public slots:
	void onBodyBeginContact(Box2DFixture *other);
	void onBodyEndContact(Box2DFixture *other);

	void killedByEnemy(GameEnemy *enemy);
	void attackSuccesful(GameEnemy *enemy);

	void setLadderMode(LadderMode ladderMode);
	void setLadder(GameLadder * ladder);
	void setEnemy(GameEnemy * enemy);
	void setHasGun(bool hasGun);

signals:
	void killed();
	void ladderModeChanged(LadderMode ladderMode);
	void ladderChanged(GameLadder * ladder);
	void enemyChanged(GameEnemy * enemy);
	void hasGunChanged(bool hasGun);

private slots:
	void onCosGameChanged(CosGame *);
	void onRayCastReported(QMultiMap<qreal, QQuickItem *> items);
	void onEnemyDied();

private:
	LadderMode m_ladderMode;
	GameLadder * m_ladder;
	GameEnemy * m_enemy;
	bool m_hasGun;
};

#endif // GAMEPLAYERPRIVATE_H