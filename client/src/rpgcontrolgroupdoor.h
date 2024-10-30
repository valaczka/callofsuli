/*
 * ---- Call of Suli ----
 *
 * rpgcontrolgroupdoor.h
 *
 * Created on: 2024. 10. 30.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgControlGroupDoor
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

#ifndef RPGCONTROLGROUPDOOR_H
#define RPGCONTROLGROUPDOOR_H

#include "rpgcontrolgroupstate.h"
#include "tiledtransport.h"


/**
 * @brief The DoorState class
 */

struct DoorState {
	int id = -1;
	QUrl image;
	QPointF relativePosition;
};



/**
 * @brief The RpgControlGroupDoor class
 */

class RpgControlGroupDoor : public RpgControlGroupState<DoorState>
{
public:
	RpgControlGroupDoor(RpgGame *game, TiledScene *scene, Tiled::GroupLayer *group, Tiled::MapRenderer *renderer);
	virtual ~RpgControlGroupDoor();

	enum OpenState {
		StateInvalid = 0,
		StateClosed,
		StateOpened,
		StateDamaged
	};

	TiledTransport *transport() const;

	bool doorOpen();
	bool doorClose();
	bool doorDamage();

	const OpenState &openState() const;
	void setOpenState(OpenState newOpenState);

	const bool &isOpaque() const;
	void setIsOpaque(bool newIsOpaque);

private:
	void update();

	QPointer<TiledTransport> m_transport;
	OpenState m_openState = StateClosed;
	bool m_isOpaque = true;
	static const QHash<QString, OpenState> m_stateHash;
};

#endif // RPGCONTROLGROUPDOOR_H
