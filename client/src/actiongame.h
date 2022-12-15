/*
 * ---- Call of Suli ----
 *
 * actiongame.h
 *
 * Created on: 2022. 12. 15.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ActionGame
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

#ifndef ACTIONGAME_H
#define ACTIONGAME_H

#include "abstractgame.h"
#include <QObject>

class ActionGame : public AbstractGame
{
	Q_OBJECT

	Q_PROPERTY(QString backgroundImage READ backgroundImage CONSTANT)
	Q_PROPERTY(QQuickItem* player READ player WRITE setPlayer NOTIFY playerChanged)

public:
	ActionGame(Client *client);
	virtual ~ActionGame();

	QQuickItem *player() const;
	void setPlayer(QQuickItem *newPlayer);

	const QString &backgroundImage() const;

protected:
	virtual QQuickItem *loadPage() override;

signals:
	void playerChanged();

private:
	QQuickItem *m_player = nullptr;
	QString m_backgroundImage;
};


//Q_DECLARE_METATYPE(ActionGame*)

#endif // ACTIONGAME_H
