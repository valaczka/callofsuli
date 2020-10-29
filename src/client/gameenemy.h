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
	Q_PROPERTY(bool notice READ notice WRITE setNotice NOTIFY noticeChanged)
	Q_PROPERTY(bool armed READ armed WRITE setArmed NOTIFY armedChanged)
	Q_PROPERTY(GameEnemyData * enemyData READ enemyData WRITE setEnemyData NOTIFY enemyDataChanged)

public:

	GameEnemy(QQuickItem *parent = 0);

	bool moving() const { return m_moving; }
	bool notice() const { return m_notice; }
	bool armed() const { return m_armed; }
	GameEnemyData * enemyData() const { return m_enemyData; }

public slots:
	void setMoving(bool moving);
	void setNotice(bool notice);
	void setArmed(bool armed);
	void setEnemyData(GameEnemyData * enemyData);

signals:
	void movingChanged(bool moving);
	void noticeChanged(bool notice);
	void armedChanged(bool armed);
	void enemyDataChanged(GameEnemyData * enemyData);

protected:
	bool m_moving;
	bool m_notice;
	bool m_armed;
	GameEnemyData * m_enemyData;

};

#endif // GAMEENEMYPRIVATE_H
