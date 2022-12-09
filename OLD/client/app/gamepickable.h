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
#include <QVariantMap>

class CosGame;

class GamePickable : public QObject
{
	Q_OBJECT
	Q_PROPERTY(PickableType type READ type WRITE setType NOTIFY typeChanged)
	Q_PROPERTY(CosGame * game READ game WRITE setGame NOTIFY gameChanged)
	Q_PROPERTY(QVariantMap data READ data WRITE setData NOTIFY dataChanged)

public:

	enum PickableType {
		PickableInvalid,
		PickableHealth,
		PickableTime,
		PickableShield,
		PickableWater,
		PickablePliers,
		PickableCamouflage,
		PickableTeleporter
	};

	Q_ENUM(PickableType)

	explicit GamePickable(QObject *parent = nullptr);
	virtual ~GamePickable();

	PickableType type() const { return m_type; }
	CosGame * game() const { return m_game; }
	QVariantMap data() const { return m_data; }

public slots:
	void pick();
	void setType(GamePickable::PickableType type);
	void setGame(CosGame * game);
	void setData(QVariantMap data);

signals:
	void picked();
	void typeChanged(GamePickable::PickableType type);
	void gameChanged(CosGame * game);
	void dataChanged(QVariantMap data);

private:
	PickableType m_type;
	CosGame * m_game;
	QVariantMap m_data;
};


/**
 * @brief The GameInventoryPickable struct
 */

struct GameInventoryPickable {
	GameInventoryPickable(GamePickable::PickableType type, QVariantMap data, int count) :
		type(type), data(data), count(count) {};

	GameInventoryPickable(GamePickable::PickableType type, int count) :
		type(type), data(), count(count) {};

	GameInventoryPickable(GamePickable::PickableType type) :
		type(type), data(), count(1) {};

	/*friend inline bool operator== (const GameInventoryPickable &b1, const GameInventoryPickable &b2) {
		return b1.type == b2.type &&
				b1.data == b2.data &&
				b1.count == b2.count;
	}*/

	GamePickable::PickableType type;
	QVariantMap data;
	int count;

};


#endif // GAMEPICKABLE_H
