/*
 * ---- Call of Suli ----
 *
 * gamepickable.h
 *
 * Created on: 2021. 01. 15.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GamePickable
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

#ifndef GAMEPICKABLE_H
#define GAMEPICKABLE_H

#include <QObject>
#include "gameenemydata.h"
#include "cosgame.h"

class GamePickable : public QObject
{
	Q_OBJECT
	Q_PROPERTY(GameEnemyData::PickableType type READ type WRITE setType NOTIFY typeChanged)
	Q_PROPERTY(CosGame * game READ game WRITE setGame NOTIFY gameChanged)
	Q_PROPERTY(QVariantMap data READ data WRITE setData NOTIFY dataChanged)

public:
	explicit GamePickable(QObject *parent = nullptr);
	virtual ~GamePickable();

	GameEnemyData::PickableType type() const { return m_type; }
	CosGame * game() const { return m_game; }
	QVariantMap data() const { return m_data; }

public slots:
	void pick();
	void setType(GameEnemyData::PickableType type);
	void setGame(CosGame * game);
	void setData(QVariantMap data);

signals:
	void picked();
	void typeChanged(GameEnemyData::PickableType type);
	void gameChanged(CosGame * game);
	void dataChanged(QVariantMap data);

private:
	GameEnemyData::PickableType m_type;
	CosGame * m_game;
	QVariantMap m_data;
};

#endif // GAMEPICKABLE_H
