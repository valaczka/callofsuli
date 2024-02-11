/*
 * ---- Call of Suli ----
 *
 * conquestgameadjacencysetup.h
 *
 * Created on: 2024. 02. 11.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ConquestGameAdjacencySetup
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

#ifndef CONQUESTGAMEADJACENCYSETUP_H
#define CONQUESTGAMEADJACENCYSETUP_H

#include "conquestgame.h"

class ConquestGameAdjacencySetup : public ConquestGame
{
	Q_OBJECT

	Q_PROPERTY(QString currentLandId READ currentLandId WRITE setCurrentLandId NOTIFY currentLandIdChanged FINAL)
	Q_PROPERTY(QStringList currentAdjacency READ currentAdjacency NOTIFY currentAdjacencyChanged FINAL)

public:
	explicit ConquestGameAdjacencySetup(Client *client);

	void loadFromFile(const QString &world);

	Q_INVOKABLE void adjacencyAdd(const QString &id);
	Q_INVOKABLE void adjacencyRemove(const QString &id);
	Q_INVOKABLE void adjacencyToggle(const QString &id);

	Q_INVOKABLE void save() const;

	QString currentLandId() const;
	void setCurrentLandId(const QString &newCurrentLandId);

	QStringList currentAdjacency() const;

signals:
	void currentLandIdChanged();
	void currentAdjacencyChanged();

protected:
	virtual void jsonOrigDataCheck(const QJsonObject &obj) override;

private:
	QString m_currentLandId;
	QHash<QString, QStringList> m_matrix;
};

#endif // CONQUESTGAMEADJACENCYSETUP_H
