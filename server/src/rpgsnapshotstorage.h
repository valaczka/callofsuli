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

	void playerAdd(const RpgGameData::PlayerBaseData &base, const RpgGameData::Player &data);
	void enemyAdd(const RpgGameData::EnemyBaseData &base, const RpgGameData::Enemy &data);
	bool bulletAdd(const RpgGameData::BulletBaseData &base, const RpgGameData::Bullet &data);

	bool registerSnapshot(RpgEnginePlayer *player, const QCborMap &cbor);

	void render(const qint64 &tick);

private:
	bool registerPlayers(RpgEnginePlayer *player, const QCborMap &cbor);
	bool registerEnemies(const QCborMap &cbor);

	RpgGameData::Player actionPlayer(RpgGameData::PlayerBaseData &pdata, RpgGameData::Player &snap);
	RpgGameData::Bullet actionBullet(RpgGameData::Bullet &snap,
									 RpgGameData::SnapshotList<RpgGameData::Bullet, RpgGameData::BulletBaseData>::iterator snapIterator,
									 std::map<qint64, RpgGameData::Bullet>::iterator nextIterator);


	template <typename T, typename T2,
			  typename = std::enable_if< std::is_base_of<RpgGameData::Body, T>::value>::type,
			  typename = std::enable_if< std::is_base_of<RpgGameData::BaseData, T2>::value>::type>
	static RpgGameData::SnapshotList<T, T2>::iterator find(T2 &key, RpgGameData::SnapshotList<T, T2> &list);

	template <typename T, typename T2,
			  typename = std::enable_if< std::is_base_of<RpgGameData::Body, T>::value>::type,
			  typename = std::enable_if< std::is_base_of<RpgGameData::BaseData, T2>::value>::type>
	static RpgGameData::SnapshotList<T, T2>::const_iterator find(const T2 &key, const RpgGameData::SnapshotList<T, T2> &list);

	template <typename T, typename T2,
			  typename = std::enable_if< std::is_base_of<RpgGameData::Body, T>::value>::type,
			  typename = std::enable_if< std::is_base_of<RpgGameData::BaseData, T2>::value>::type>
	static RpgGameData::SnapshotList<T, T2>::iterator find(RpgGameData::BaseData &key, RpgGameData::SnapshotList<T, T2> &list);

	template <typename T, typename T2,
			  typename = std::enable_if< std::is_base_of<RpgGameData::Body, T>::value>::type,
			  typename = std::enable_if< std::is_base_of<RpgGameData::BaseData, T2>::value>::type>
	static RpgGameData::SnapshotList<T, T2>::const_iterator constFind(const RpgGameData::BaseData &key, RpgGameData::SnapshotList<T, T2> &list);

	template <typename T, typename T2,
			  typename = std::enable_if< std::is_base_of<RpgGameData::Body, T>::value>::type >
	static std::map<qint64, T>::iterator findState(std::map<qint64, T> &list, const T2 &state);

	template <typename T, typename T2,
			  typename = std::enable_if< std::is_base_of<RpgGameData::Body, T>::value>::type >
	static std::map<qint64, T>::const_iterator findState(const std::map<qint64, T> &list, const T2 &state);

	template <typename T2, typename = std::enable_if<std::is_base_of< RpgGameData::BaseData, T2>::value>::type>
	static bool checkBaseData(const T2 &key, const QCborMap &cbor, const QString &baseKey);

	template <typename T>
	static std::optional<T> getPreviousSnap(std::map<qint64, T> &list, const qint64 &tick,
											std::map<qint64, T>::iterator *nextPtr = nullptr);

	template <typename T>
	static std::optional<T> getPreviousSnap(std::map<qint64, T> &list, const T &snap,
											std::map<qint64, T>::iterator *nextPtr = nullptr);

	template <typename T, typename T2,
			  typename = std::enable_if< std::is_base_of<RpgGameData::Body, T>::value>::type,
			  typename = std::enable_if< std::is_base_of<RpgGameData::BaseData, T2>::value>::type>
	static std::optional<T> getPreviousSnap(RpgGameData::SnapshotData<T, T2> &data,
											const QCborValue &cbor,
											std::map<qint64, T>::iterator *nextPtr = nullptr);

	template <typename T, typename T2,
			  typename = std::enable_if< std::is_base_of<RpgGameData::Body, T>::value>::type,
			  typename = std::enable_if< std::is_base_of<RpgGameData::BaseData, T2>::value>::type>
	static std::optional<T> getPreviousSnap(RpgGameData::SnapshotData<T, T2> &data, const T &snap,
											std::map<qint64, T>::iterator *nextPtr = nullptr);

	template <typename T, typename T2,
			  typename = std::enable_if< std::is_base_of<RpgGameData::Body, T>::value>::type,
			  typename = std::enable_if< std::is_base_of<RpgGameData::BaseData, T2>::value>::type>
	std::optional<T> addToPreviousSnap(RpgGameData::SnapshotData<T, T2> &data,
									   const QCborValue &cbor,
									   std::map<qint64, T>::iterator *nextPtr,
									   const std::function<void(QCborMap *)> &fn = nullptr);

	template <typename T, typename T2,
			  typename = std::enable_if< std::is_base_of<RpgGameData::Body, T>::value>::type,
			  typename = std::enable_if< std::is_base_of<RpgGameData::BaseData, T2>::value>::type>
	void assignLastSnap(const T &src, RpgGameData::SnapshotData<T, T2> &dest);

	RpgEngine *m_engine = nullptr;

	std::vector<RpgGameData::BaseData> m_lastBulletId;
};



/**
 * @brief RpgSnapshotStorage::findState
 * @param list
 * @param state
 * @return
 */

template<typename T, typename T2, typename T3>
inline std::map<qint64, T>::const_iterator RpgSnapshotStorage::findState(const std::map<qint64, T> &list,
																   const T2 &state)
{
	return std::find_if(list.cbegin(), list.cend(),
						  [&state](const auto &ptr){
		return (ptr.second.st == state);
	});
}


/**
 * @brief RpgSnapshotStorage::findState
 * @param list
 * @param state
 * @return
 */

template<typename T, typename T2, typename T3>
inline std::map<qint64, T>::iterator RpgSnapshotStorage::findState(std::map<qint64, T> &list,
																   const T2 &state)
{
	return std::find_if(list.begin(), list.end(),
						  [&state](const auto &ptr){
		return (ptr.second.st == state);
	});
}







/**
 * @brief RpgSnapshotStorage::find
 * @param key
 * @param list
 * @return
 */

template<typename T, typename T2, typename T3, typename T4>
inline RpgGameData::SnapshotList<T, T2>::const_iterator RpgSnapshotStorage::constFind(const RpgGameData::BaseData &key,
																					  RpgGameData::SnapshotList<T, T2> &list)
{
	typename RpgGameData::SnapshotList<T, T2>::const_iterator it = std::find_if(
																	   list.cbegin(),
																	   list.cend(),
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
inline RpgGameData::SnapshotList<T, T2>::iterator RpgSnapshotStorage::find(RpgGameData::BaseData &key,
																		   RpgGameData::SnapshotList<T, T2> &list)
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
 * @brief RpgSnapshotStorage::assignLastSnap
 * @param src
 * @param dest
 */

template<typename T, typename T2, typename T3, typename T4>
inline void RpgSnapshotStorage::assignLastSnap(const T &src, RpgGameData::SnapshotData<T, T2> &dest)
{
	dest.list.insert_or_assign(src.f, src);
	dest.lastIn = src.f;
}




/**
 * @brief RpgSnapshotStorage::getPreviousSnap
 * @param data
 * @param cbor
 * @param nextPtr
 * @return
 */


template<typename T, typename T2, typename T3, typename T4>
inline std::optional<T> RpgSnapshotStorage::getPreviousSnap(RpgGameData::SnapshotData<T, T2> &data,
															const QCborValue &cbor,
															std::map<qint64, T>::iterator *nextPtr)
{
	T snap;
	snap.fromCbor(cbor, true);

	return getPreviousSnap(data, snap, nextPtr);
}





/**
 * @brief RpgSnapshotStorage::getPreviousSnap
 * @param data
 * @param snap
 * @param nextPtr
 * @return
 */

template<typename T, typename T2, typename T3, typename T4>
inline std::optional<T> RpgSnapshotStorage::addToPreviousSnap(RpgGameData::SnapshotData<T, T2> &data,
															  const QCborValue &cbor,
															  std::map<qint64, T>::iterator *nextPtr,
															  const  std::function<void(QCborMap*)> &fn)
{
	if (nextPtr)
		*nextPtr = data.list.end();

	QCborValue pcbor;

	if (fn) {
		QCborMap m = cbor.toMap();
		fn(&m);
		pcbor = m;
	} else {
		pcbor = cbor;
	}

	T snap;
	snap.fromCbor(pcbor);

	if (snap.f <= data.lastIn) {
		qWarning() << "OUT OF TIME" << snap.f << data.lastIn;
		return std::nullopt;
	}

	std::optional<T> prev = getPreviousSnap(data.list, snap, nextPtr);

	if (prev) {
		prev->fromCbor(pcbor);
		return prev;
	} else {
		return snap;
	}
}



/**
 * @brief RpgSnapshotStorage::checkBaseData
 * @param key
 * @param cbor
 * @param baseKey
 * @return
 */

template<typename T2, typename T3>
inline bool RpgSnapshotStorage::checkBaseData(const T2 &key, const QCborMap &cbor, const QString &baseKey)
{
	T2 pd;

	pd.fromCbor(cbor.value(baseKey));

	return pd == key;
}





/**
 * @brief RpgSnapshotStorage::getFullSnap
 * @param list
 * @param snap
 * @param itPtr
 * @return
 */

template<typename T>
inline std::optional<T> RpgSnapshotStorage::getPreviousSnap(std::map<qint64, T> &list, const T &snap, std::map<qint64, T>::iterator *nextPtr)
{
	return getPreviousSnap(list, snap.f, nextPtr);
}




template<typename T>
inline std::optional<T> RpgSnapshotStorage::getPreviousSnap(std::map<qint64, T> &list, const qint64 &tick, std::map<qint64, T>::iterator *nextPtr)
{
	typename std::map<qint64, T>::iterator it = list.upper_bound(tick);			// Greater

	if (nextPtr)
		*nextPtr = it;

	if (it != list.cbegin())
		return std::prev(it)->second;
	else
		return std::nullopt;
}


/**
 * @brief RpgSnapshotStorage::find
 * @param key
 * @return
 */

template<typename T, typename T2, typename T3, typename T4>
inline RpgGameData::SnapshotList<T, T2>::iterator RpgSnapshotStorage::find(T2 &key, RpgGameData::SnapshotList<T, T2> &list)
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
inline RpgGameData::SnapshotList<T, T2>::const_iterator RpgSnapshotStorage::find(const T2 &key, const RpgGameData::SnapshotList<T, T2> &list)
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
