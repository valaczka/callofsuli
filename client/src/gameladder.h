/*
 * ---- Call of Suli ----
 *
 * gameladder.h
 *
 * Created on: 2022. 12. 17.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GameLadder
 *
 *  This file is part of Call of Suli.
 *
 *  Call of Suli is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef GAMELADDER_H
#define GAMELADDER_H

#include "gameobject.h"

class GameLadder : public GameObject
{
	Q_OBJECT

	Q_PROPERTY(QRectF boundRect READ boundRect WRITE setBoundRect NOTIFY boundRectChanged)
	Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)
	Q_PROPERTY(int blockTop READ blockTop WRITE setBlockTop NOTIFY blockTopChanged)
	Q_PROPERTY(int blockBottom READ blockBottom WRITE setBlockBottom NOTIFY blockBottomChanged)

public:
	GameLadder(QQuickItem *parent = nullptr);
	virtual ~GameLadder();

	const QRectF &boundRect() const;
	void setBoundRect(const QRectF &newBoundRect);

	bool active() const;
	void setActive(bool newActive);

	int blockTop() const;
	void setBlockTop(int newBlockTop);

	int blockBottom() const;
	void setBlockBottom(int newBlockBottom);

signals:
	void boundRectChanged();
	void activeChanged();
	void blockTopChanged();
	void blockBottomChanged();

private:
	QRectF m_boundRect;
	bool m_active = false;
	int m_blockTop = -1;
	int m_blockBottom = -1;
};

#endif // GAMELADDER_H
