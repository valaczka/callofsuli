/*
 * ---- Call of Suli ----
 *
 * mapplaydemo.h
 *
 * Created on: 2023. 01. 01.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * MapPlayDemo
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

#ifndef MAPPLAYDEMO_H
#define MAPPLAYDEMO_H

#include "mapplay.h"
#include "utils.h"

class MapPlayDemo : public MapPlay
{
	Q_OBJECT

public:
	explicit MapPlayDemo(Client *client, QObject *parent = nullptr);
	virtual ~MapPlayDemo();

	bool load(const QString &map = QStringLiteral(":/internal/game/demo.map"));

	void solverLoad();
	void solverSave();

protected:
	virtual void onCurrentGamePrepared() override;
	virtual void onCurrentGameFinished() override;

private:
	const QString m_file = Utils::standardPath(QStringLiteral("demomap.json"));
};

#endif // MAPPLAYDEMO_H
