/*
 * ---- Call of Suli ----
 *
 * gameenemysoldierprivate.h
 *
 * Created on: 2020. 10. 28.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GameEnemySoldierPrivate
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

#ifndef GAMEENEMYSOLDIERPRIVATE_H
#define GAMEENEMYSOLDIERPRIVATE_H

#include "gameenemy.h"

class GameEnemySoldier : public GameEnemy
{
	Q_OBJECT

	Q_PROPERTY(bool atBound READ atBound WRITE setAtBound NOTIFY atBoundChanged)
	Q_PROPERTY(int msecBeforeTurn READ msecBeforeTurn WRITE setMsecBeforeTurn NOTIFY msecBeforeTurnChanged)

public:
	GameEnemySoldier(QQuickItem *parent = 0);
	~GameEnemySoldier();

	void setQrcDir() override;
	void createFixtures() override;

	int msecBeforeTurn() const { return m_msecBeforeTurn; }
	bool atBound() const { return m_atBound; }

public slots:
	void setMsecBeforeTurn(int msecBeforeTurn);
	void setAtBound(bool atBound);

signals:
	void msecBeforeTurnChanged(int msecBeforeTurn);
	void atBoundChanged(bool atBound);

private slots:
	void onCosGameChanged(CosGame *);
	void onMovingTimerTimeout();

private:
	QTimer *m_movingTimer;
	int m_msecBeforeTurn;
	bool m_atBound;
	int m_turnElapsedMsec;
};

#endif // GAMEENEMYSOLDIERPRIVATE_H
