/*
 * ---- Call of Suli ----
 *
 * conqueststate.h
 *
 * Created on: 2024. 01. 20.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ConquestLand
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

#ifndef CONQUESTLAND_H
#define CONQUESTLAND_H

#include "conquestgame.h"
#include <QQuickItem>

class ConquestLand : public QQuickItem
{
	Q_OBJECT

	Q_PROPERTY(QString world READ world WRITE setWorld NOTIFY worldChanged FINAL)
	Q_PROPERTY(int stateId READ stateId WRITE setStateId NOTIFY stateIdChanged FINAL)
	Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged FINAL)
	Q_PROPERTY(bool isValid READ isValid NOTIFY isValidChanged FINAL)
	Q_PROPERTY(qreal baseX READ baseX WRITE setBaseX NOTIFY baseXChanged FINAL)
	Q_PROPERTY(qreal baseY READ baseY WRITE setBaseY NOTIFY baseYChanged FINAL)
	Q_PROPERTY(bool isActive READ isActive WRITE setIsActive NOTIFY isActiveChanged FINAL)
	Q_PROPERTY(ConquestGame *game READ game WRITE setGame NOTIFY gameChanged FINAL)

	QML_ELEMENT

public:
	ConquestLand();

	QString world() const;
	void setWorld(const QString &newWorld);

	int stateId() const;
	void setStateId(int newStateId);

	QColor color() const;
	void setColor(const QColor &newColor);

	bool isValid() const;

	qreal baseX() const;
	void setBaseX(qreal newBaseX);

	qreal baseY() const;
	void setBaseY(qreal newBaseY);

	bool isActive() const;
	void setIsActive(bool newIsActive);

	ConquestGame *game() const;
	void setGame(ConquestGame *newGame);

signals:
	void worldChanged();
	void stateIdChanged();
	void colorChanged();
	void isValidChanged();
	void baseXChanged();
	void baseYChanged();
	void isActiveChanged();
	void gameChanged();

private:
	void reload();
	void setIsValid(const bool &valid);

	QString m_world;
	int m_stateId = -1;
	QColor m_color;
	bool m_isValid = false;
	qreal m_baseX = 0;
	qreal m_baseY = 0;
	bool m_isActive = false;
	ConquestGame *m_game = nullptr;
};

#endif // CONQUESTLAND_H
