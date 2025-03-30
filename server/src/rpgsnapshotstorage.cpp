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

			RpgGameData::BaseData pdata = *player;

			if (!checkBaseData(pdata, m, QStringLiteral("pd"))) {
				LOG_CWARNING("engine") << "Invalid player" << m;
				continue;
			}

			auto it = find(pdata, m_players);

			if (it == m_players.end()) {
				LOG_CWARNING("engine") << "Invalid player data" << pdata.o << pdata.s << pdata.id;
				continue;
			}

			const QCborArray array = m.value(QStringLiteral("p")).toArray();

			for (const QCborValue &pv : array) {
				std::map<qint64, RpgGameData::Player>::iterator snapIt;
				std::optional<RpgGameData::Player> snap = addToPreviousSnap(*it, pv, &snapIt);

				if (!snap) {
					LOG_CWARNING("engine") << "---" << pv;
					continue;
				}

				assignLastSnap(snap.value(), *it);

				success = true;

				if (snap->st == RpgGameData::Player::PlayerHit) {
					LOG_CDEBUG("engine") << "+++ HIT" << snap->f << (it != m_players.end() ? it - m_players.begin() : -1)
										 << snap->st << snap->p << snap->a;
				}
			}
		}
	}

	return success;
}

