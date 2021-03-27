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
#include "gamepickable.h"
#include <QSoundEffect>


class GamePlayer : public GameEntity
{
	Q_OBJECT

	Q_PROPERTY(LadderMode ladderMode READ ladderMode WRITE setLadderMode NOTIFY ladderModeChanged)
	Q_PROPERTY(GameLadder * ladder READ ladder WRITE setLadder NOTIFY ladderChanged)
	Q_PROPERTY(GameEnemy * enemy READ enemy WRITE setEnemy NOTIFY enemyChanged)
	Q_PROPERTY(int defaultHp READ defaultHp WRITE setDefaultHp NOTIFY defaultHpChanged)
	Q_PROPERTY(int shield READ shield WRITE setShield NOTIFY shieldChanged)

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
	virtual ~GamePlayer();

	void setQrcDir() override;
	void createFixtures() override;
	Q_INVOKABLE void ladderClimbUp();
	Q_INVOKABLE void ladderClimbDown();
	Q_INVOKABLE void ladderClimbFinish();
	Q_INVOKABLE void attackByGun();

	LadderMode ladderMode() const { return m_ladderMode; }
	GameLadder * ladder() const { return m_ladder; }
	GameEnemy * enemy() const { return m_enemy; }
	int defaultHp() const { return m_defaultHp; }
	int shield() const { return m_shield; }

public slots:
	QString playSoundEffect(const QString &effect);
	void onBodyBeginContact(Box2DFixture *other);
	void onBodyEndContact(Box2DFixture *other);

	void hurtByEnemy(GameEnemy *enemy, const bool &canProtect = false);
	void killByEnemy(GameEnemy *enemy);
	void attackSuccesful(GameEnemy *enemy);
	void attackFailed(GameEnemy *enemy);

	void setLadderMode(LadderMode ladderMode);
	void setLadder(GameLadder * ladder);
	void setEnemy(GameEnemy * enemy);
	void setDefaultHp(int defaultHp);
	void setShield(int shield);

signals:
	void killedByEnemy(GameEnemy *enemy);
	void diedByFall();
	void hurt(GameEnemy *enemy);
	void underAttack();
	void attack();
	void attackSucceed(GameEnemy *enemy);
	void ladderModeChanged(LadderMode ladderMode);
	void ladderChanged(GameLadder * ladder);
	void enemyChanged(GameEnemy * enemy);
	void hpChanged(int hp);
	void defaultHpChanged(int defaultHp);
	void shieldChanged(int shield);

private slots:
	void onCosGameChanged(CosGame *);
	void onRayCastReported(QMultiMap<qreal, QQuickItem *> items);
	void onEnemyDied();

private:
	LadderMode m_ladderMode;
	bool m_isLadderDirectionUp;
	GameLadder * m_ladder;
	GameEnemy * m_enemy;
	int m_defaultHp;
	int m_soundEffectRunNum;
	int m_soundEffectClimbNum;
	int m_soundEffectWalkNum;
	int m_soundEffectPainNum;
	int m_shield;
};


#endif // GAMEPLAYERPRIVATE_H
