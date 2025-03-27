/*
 * ---- Call of Suli ----
 *
 * rpgsnapshotstorage.cpp
 *
 * Created on: 2025. 03. 27.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgSnapshotStorage
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

#include "rpgsnapshotstorage.h"
#include "Logger.h"
#include "rpgengine.h"


RpgSnapshotStorage::RpgSnapshotStorage(RpgEngine *engine)
	: RpgGameData::SnapshotStorage()
	, m_engine(engine)
{

}



/**
 * @brief RpgSnapshotStorage::playerAdd
 * @param base
 * @param data
 * @return
 */

void RpgSnapshotStorage::playerAdd(const RpgGameData::BaseData &base, const RpgGameData::Player &data)
{
	QMutexLocker locker(&m_mutex);

	RpgGameData::SnapshotData<RpgGameData::Player, RpgGameData::BaseData> snapdata;

	snapdata.data = base;
	snapdata.list.insert_or_assign(data.f, data);

	m_players.push_back(snapdata);
}



/**
 * @brief RpgSnapshotStorage::registerSnapshot
 * @param cbor
 * @return
 */

bool RpgSnapshotStorage::registerSnapshot(RpgEnginePlayer *player, const QCborMap &cbor)
{
	if (!player)
		return false;

	QMutexLocker locker(&m_mutex);


	if (!registerPlayers(player, cbor))
		return false;



	return true;
}



/**
 * @brief RpgSnapshotStorage::registerPlayers
 * @param player
 * @param cbor
 * @return
 */

bool RpgSnapshotStorage::registerPlayers(RpgEnginePlayer *player, const QCborMap &cbor)
{
	Q_ASSERT(player);

	QMutexLocker locker(&m_mutex);

	bool success = false;

	if (const auto &it = cbor.find(QStringLiteral("pp")); it != cbor.cend()) {
		const QCborArray list = it->toArray();

		for (const QCborValue &v : list) {
			const QCborMap m = v.toMap();

			RpgGameData::SnapshotList<RpgGameData::Player, RpgGameData::BaseData>::iterator it;
			std::map<qint64, RpgGameData::Player>::iterator snapIt;

			RpgGameData::Player snap = getPreviousSnap(m_players, m, (RpgGameData::BaseData) *player,
													   QStringLiteral("pd"), QStringLiteral("p"),
													   &it, &snapIt);

			if (snap.f < 0)
				continue;

			success = true;

			LOG_CDEBUG("engine") << "+++" << snap.f << (it != m_players.end() ? it - m_players.begin() : -1)
								 << (it != m_players.end() && snapIt != it->list.end() ? -99 : -1)
								 << (it != m_players.end() ? it->list.size() : -2);

			it->list.insert_or_assign(snap.f, snap);

		}
	}

	return success;
}

