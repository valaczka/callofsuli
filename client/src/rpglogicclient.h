/*
 * ---- Call of Suli ----
 *
 * rpglogicclient.h
 *
 * Created on: 2026. 05. 12.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgLogicClient
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

#ifndef RPGLOGICCLIENT_H
#define RPGLOGICCLIENT_H

#include "chipmunk/chipmunk_types.h"
#include <rpglogic.h>

class RpgObject;

namespace Rpg {


/**
 * @brief The RpgEntityStatePull class
 */



typedef BaseStatePull<RpgStream::PlayerState, 10> RpgPlayerStatePull;




/**
 * @brief The RpgLogicClient class
 */

class RpgLogicClient : public RpgLogic
{
public:
	RpgLogicClient(const quint32 &lastAuthDiff, const quint32 &jitterDiff);

	void removeFromMapper(RpgObject *object);
	QPoint getChunkFromVector(const cpVect &point, cpVect *centerPtr = nullptr);


};




/**
 * @brief The RpgLogicClientSingle class
 */

class RpgLogicClientSingle : public RpgLogicClient
{
public:
	RpgLogicClientSingle() : RpgLogicClient(0, 0) {}
};


/**
 * @brief The RpgLogicClientMulti class
 */

class RpgLogicClientMulti : public RpgLogicClient
{
public:
	RpgLogicClientMulti() : RpgLogicClient(12, 6) { qWarning() << "MULTI"; }
};

}		// end of namespace

#endif // RPGLOGICCLIENT_H
