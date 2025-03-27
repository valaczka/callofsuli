/*
 * ---- Call of Suli ----
 *
 * rpgsnapshotstorage.h
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

#ifndef RPGSNAPSHOTSTORAGE_H
#define RPGSNAPSHOTSTORAGE_H

#include "rpgconfig.h"

class RpgEngine;
class RpgEnginePlayer;



/**
 * @brief The RpgSnapshotStorage class
 */

class RpgSnapshotStorage : public RpgGameData::SnapshotStorage
{
public:
	RpgSnapshotStorage(RpgEngine *engine);

	void playerAdd(const RpgGameData::BaseData &base, const RpgGameData::Player &data);

	bool registerSnapshot(RpgEnginePlayer *player, const QCborMap &cbor);

private:
	bool registerPlayers(RpgEnginePlayer *player, const QCborMap &cbor);

	template <typename T, typename T2,
			  typename = std::enable_if<std::is_base_of<RpgGameData::Body, T>::value>::type,
			  typename = std::enable_if<std::is_base_of<RpgGameData::BaseData, T2>::value>::type>
	static RpgGameData::SnapshotList<T, T2>::iterator find(const T2 &key, RpgGameData::SnapshotList<T, T2> &list);

	template <typename T, typename T2,
			  typename = std::enable_if<std::is_base_of<RpgGameData::Body, T>::value>::type,
			  typename = std::enable_if<std::is_base_of<RpgGameData::BaseData, T2>::value>::type>
	static RpgGameData::SnapshotList<T, T2>::const_iterator constFind(const T2 &key, const RpgGameData::SnapshotList<T, T2> &list);


	template <typename T, typename T2,
			  typename = std::enable_if<std::is_base_of<RpgGameData::Body, T>::value>::type,
			  typename = std::enable_if<std::is_base_of<RpgGameData::BaseData, T2>::value>::type>
	static T getPreviousSnap(RpgGameData::SnapshotList<T, T2> &list, const QCborMap &cbor,
							 const T2 &key,
							 const QString &baseKey, const QString &dataKey,
							 RpgGameData::SnapshotList<T, T2>::iterator *itPtr = nullptr,
							 std::map<qint64, T>::iterator *tickPtr = nullptr);


	template <typename T>
	static T getPreviousSnap(std::map<qint64, T> &list, const T &snap, std::map<qint64, T>::iterator *nextPtr = nullptr);


	RpgEngine *m_engine = nullptr;
};



/**
 * @brief RpgSnapshotStorage::getPreviousSnap
 * @param list
 * @param key
 * @param src
 * @param itPtr
 * @param tickPtr
 * @return
 */

template<typename T, typename T2, typename T3, typename T4>
inline T RpgSnapshotStorage::getPreviousSnap(RpgGameData::SnapshotList<T, T2> &list, const QCborMap &cbor,
											 const T2 &key,
											 const QString &baseKey, const QString &dataKey,
											 RpgGameData::SnapshotList<T, T2>::iterator *itPtr,
											 std::map<qint64, T>::iterator *tickPtr)
{
	if (itPtr)
		*itPtr = list.end();

	T tmp;
	T2 pd;

	pd.fromCbor(cbor.value(baseKey));

	if (pd != key)
		return tmp;

	typename RpgGameData::SnapshotList<T, T2>::iterator pIt = find(pd, list);

	if (pIt == list.end()) {
		return tmp;
	}

	if (itPtr)
		*itPtr = pIt;

	const QCborValue v = cbor.value(dataKey);

	tmp.fromCbor(v);

	typename std::map<qint64, T>::iterator snapIt;
	T prev = getPreviousSnap(pIt->list, tmp, &snapIt);
	prev.fromCbor(v);

	if (tickPtr)
		*tickPtr = snapIt;

	return prev;
}




/**
 * @brief RpgSnapshotStorage::getFullSnap
 * @param list
 * @param snap
 * @param itPtr
 * @return
 */

template<typename T>
inline T RpgSnapshotStorage::getPreviousSnap(std::map<qint64, T> &list, const T &snap, std::map<qint64, T>::iterator *nextPtr)
{
	qint64 tick = snap.f;
	typename std::map<qint64, T>::iterator it = list.upper_bound(tick);			// Greater

	if (nextPtr)
		*nextPtr = it;

	if (it != list.cbegin() && it != list.cend())
		return std::prev(it)->second;
	else
		return snap;
}



/**
 * @brief RpgSnapshotStorage::find
 * @param key
 * @return
 */

template<typename T, typename T2, typename T3, typename T4>
inline RpgGameData::SnapshotList<T, T2>::iterator RpgSnapshotStorage::find(const T2 &key, RpgGameData::SnapshotList<T, T2> &list)
{
	typename RpgGameData::SnapshotList<T, T2>::iterator it = std::find_if(
																 list.begin(),
																 list.end(),
																 [&key](const RpgGameData::SnapshotData<T, T2> &d) {
		return d.data == key;
	});

	return it;
}




/**
 * @brief RpgSnapshotStorage::find
 * @param key
 * @param list
 * @return
 */

template<typename T, typename T2, typename T3, typename T4>
inline RpgGameData::SnapshotList<T, T2>::const_iterator RpgSnapshotStorage::constFind(const T2 &key, const RpgGameData::SnapshotList<T, T2> &list)
{
	typename RpgGameData::SnapshotList<T, T2>::const_iterator it = std::find_if(
																	   list.cbegin(),
																	   list.cend(),
																	   [&key](const RpgGameData::SnapshotData<T, T2> &d) {
		return d.data == key;
	});

	return it;
}

#endif // RPGSNAPSHOTSTORAGE_H
