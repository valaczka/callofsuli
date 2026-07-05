/*
 * ---- Call of Suli ----
 *
 * rpgmapplaytutorial.h
 *
 * Created on: 2026. 07. 04.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgMapPlayTutorial
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

#ifndef RPGMAPPLAYTUTORIAL_H
#define RPGMAPPLAYTUTORIAL_H

#include "mapplay.h"

class RpgGame;


/**
 * @brief The RpgMapPlayTutorial class
 */

class RpgMapPlayTutorial : public MapPlay
{
	Q_OBJECT

public:
	explicit RpgMapPlayTutorial(Client *client, QObject *parent = nullptr);
	virtual ~RpgMapPlayTutorial();

	QQuickItem *load(const QUrl &url);

private:
	void onFinished(AbstractGame::FinishState state);

	RpgGame *m_game = nullptr;

};

#endif // RPGMAPPLAYTUTORIAL_H
