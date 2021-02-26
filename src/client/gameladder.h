/*
 * ---- Call of Suli ----
 *
 * gameladder.h
 *
 * Created on: 2020. 10. 25.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GameLadder
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

#ifndef GAMELADDER_H
#define GAMELADDER_H

#include <QRect>
#include <QObject>
#include "gameblock.h"

class GameLadder : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QRectF boundRect READ boundRect WRITE setBoundRect NOTIFY boundRectChanged)
	Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)
	Q_PROPERTY(GameBlock * blockTop READ blockTop WRITE setBlockTop NOTIFY blockTopChanged)
	Q_PROPERTY(GameBlock * blockBottom READ blockBottom WRITE setBlockBottom NOTIFY blockBottomChanged)

public:
	explicit GameLadder(QObject *parent = nullptr);

	QRectF boundRect() const { return m_boundRect; }
	bool active() const { return m_active; }
	GameBlock * blockTop() const { return m_blockTop; }
	GameBlock * blockBottom() const { return m_blockBottom; }

public slots:
	void setBoundRect(QRectF boundRect);
	void setActive(bool active);
	void setBlockTop(GameBlock * blockTop);
	void setBlockBottom(GameBlock * blockBottom);

signals:
	void boundRectChanged(QRectF boundRect);
	void activeChanged(bool active);
	void blockTopChanged(GameBlock * blockTop);
	void blockBottomChanged(GameBlock * blockBottom);

private:
	QRectF m_boundRect;
	bool m_active;
	GameBlock * m_blockTop;
	GameBlock * m_blockBottom;
};

#endif // GAMELADDER_H
