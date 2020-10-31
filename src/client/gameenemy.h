/*
 * ---- Call of Suli ----
 *
 * gameenemyprivate.h
 *
 * Created on: 2020. 10. 28.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GameEnemyPrivate
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

#ifndef GAMEENEMYPRIVATE_H
#define GAMEENEMYPRIVATE_H

#include "gameentity.h"

class GameEnemyData;

class GameEnemy : public GameEntity
{
	Q_OBJECT

	Q_PROPERTY(bool moving READ moving WRITE setMoving NOTIFY movingChanged)
	Q_PROPERTY(bool armed READ armed WRITE setArmed NOTIFY armedChanged)
	Q_PROPERTY(bool aimedByPlayer READ aimedByPlayer WRITE setAimedByPlayer NOTIFY aimedByPlayerChanged)
	Q_PROPERTY(GameEnemyData * enemyData READ enemyData WRITE setEnemyData NOTIFY enemyDataChanged)
	Q_PROPERTY(qreal castAttackFraction READ castAttackFraction WRITE setCastAttackFraction NOTIFY castAttackFractionChanged)
	Q_PROPERTY(int msecBeforeAttack READ msecBeforeAttack WRITE setMsecBeforeAttack NOTIFY msecBeforeAttackChanged)
	Q_PROPERTY(int msecLeftAttack READ msecLeftAttack NOTIFY msecLeftAttackChanged)
	Q_PROPERTY(GamePlayer * player READ player WRITE setPlayer NOTIFY playerChanged)
	Q_PROPERTY(bool attackRunning READ attackRunning WRITE setAttackRunning NOTIFY attackRunningChanged)


public:
	GameEnemy(QQuickItem *parent = 0);
	~GameEnemy();

	bool moving() const { return m_moving; }
	bool armed() const { return m_armed; }
	GameEnemyData * enemyData() const { return m_enemyData; }
	qreal castAttackFraction() const { return m_castAttackFraction; }
	GamePlayer * player() const { return m_player; }
	bool attackRunning() const { return m_attackRunning; }
	int msecBeforeAttack() const { return m_msecBeforeAttack; }
	int msecLeftAttack() const;
	bool aimedByPlayer() const { return m_aimedByPlayer; }

public slots:
	void tryAttack(GamePlayer *player);
	void setMoving(bool moving);
	void setArmed(bool armed);
	void setEnemyData(GameEnemyData * enemyData);
	void setCastAttackFraction(qreal castAttackFraction);
	void setPlayer(GamePlayer * player);
	void setAttackRunning(bool attackRunning);
	void setMsecBeforeAttack(int msecBeforeAttack);
	void setAimedByPlayer(bool aimedByPlayer);

protected slots:
	void onGameChanged();
	virtual void onGameDataReady(const QVariantMap &map) { Q_UNUSED(map); }
	virtual void attackTimerTimeout();
	void onRayCastReported(QMultiMap<qreal, QQuickItem *> items);

private slots:
	void onPlayerDied();

signals:
	void attackPlayer();
	void killed();
	void movingChanged(bool moving);
	void armedChanged(bool armed);
	void enemyDataChanged(GameEnemyData * enemyData);
	void castAttackFractionChanged(qreal castAttackFraction);
	void playerChanged(QQuickItem * player);
	void attackRunningChanged(bool attackRunning);
	void msecBeforeAttackChanged(int msecBeforeAttack);
	void msecLeftAttackChanged();
	void aimedByPlayerChanged(bool aimedByPlayer);

protected:
	bool m_moving;
	bool m_armed;
	GameEnemyData * m_enemyData;
	QTimer *m_attackTimer;
	int m_attackTimerElapsed;

	qreal m_castAttackFraction;
	GamePlayer * m_player;
	bool m_attackRunning;
	int m_msecBeforeAttack;
	bool m_aimedByPlayer;

};

#endif // GAMEENEMYPRIVATE_H
