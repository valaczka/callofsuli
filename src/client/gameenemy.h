/*
 * ---- Call of Suli ----
 *
 * gameenemy.h
 *
 * Created on: 2020. 10. 23.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GameEnemy
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

#ifndef GAMEENEMY_H
#define GAMEENEMY_H

#include <QRectF>
#include <QObject>

class GameEnemy : public QObject
{
	Q_OBJECT

	Q_PROPERTY(int posY READ posY WRITE setPosY NOTIFY posYChanged)
	Q_PROPERTY(QRect boundRect READ boundRect WRITE setBoundRect NOTIFY boundRectChanged)
	Q_PROPERTY(int block READ block WRITE setBlock NOTIFY blockChanged)

	Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)


public:
	explicit GameEnemy(QObject *parent = nullptr);

	QRect boundRect() const { return m_boundRect; }
	int posY() const { return m_posY; }
	bool active() const { return m_active; }
	int block() const { return m_block; }

public slots:
	void setPosY(int posY);
	void setBoundRect(QRect boundRect);
	void setActive(bool active);
	void setBlock(int block);

signals:
	void boundRectChanged(QRect boundRect);
	void posYChanged(int posY);
	void activeChanged(bool active);
	void blockChanged(int block);

private:
	QRect m_boundRect;
	int m_posY;
	bool m_active;
	int m_block;
};

#endif // GAMEENEMY_H
