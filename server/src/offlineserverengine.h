/*
 * ---- Call of Suli ----
 *
 * offlineserverengine.h
 *
 * Created on: 2026. 02. 07.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * OfflineServerEngine
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

#ifndef OFFLINESERVERENGINE_H
#define OFFLINESERVERENGINE_H

#include <credential.h>
#include <offlineengine.h>
#include "serverservice.h"

class OfflineServerEngine : public OfflineEngine
{
public:
	OfflineServerEngine(ServerService *service);

	std::optional<PermitFull> createPermit(const QString &username, const int &campaign, const QByteArray &device,
										   const qint64 &clientClock);
	QByteArray signPermit(const PermitContent &permit) const;
	std::optional<PermitContent> verifyPermit(const QByteArray &data) const;


private:
	bool generateHash(PermitContent &permit, const QString &username, const int &campaign, const QByteArray &device);


	ServerService *const m_service;
	AuthKeySigner m_signer;
};

#endif // OFFLINESERVERENGINE_H
