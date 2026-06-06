/*
 * ---- Call of Suli ----
 *
 * rpgudpengine.cpp
 *
 * Created on: 2026. 06. 06.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgUdpEngine
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

#include "rpgudpengine.h"


RpgUdpEngine::RpgUdpEngine(QObject *parent)
	: AbstractUdpEngine{parent}
{

}



/**
 * @brief RpgUdpEngine::binaryDataReceived
 * @param list
 */

void RpgUdpEngine::binaryDataReceived(std::vector<UdpPacketRcv> &list)
{
	if (list.empty())
		return;

	LOG_CINFO("client") << "RCV" << list.size();

	for (const UdpPacketRcv &p : list) {
		LOG_CDEBUG("client") << "--" << p.data->type() << p.rtt;
	}
}
