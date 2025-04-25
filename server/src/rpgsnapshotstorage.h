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





template <typename T, typename T2,
		  typename = std::enable_if<std::is_base_of<RpgGameData::Body, T>::value>::type,
		  typename = std::enable_if<std::is_base_of<RpgGameData::BaseData, T2>::value>::type>
struct RendererData {
	T2 data;										// Alapadatok
	std::map<qint64, T> *ptr = nullptr;				// A storage snapshot-listája
	std::map<qint64, T>::iterator it;				// Ha van a tick-nél nem kisebb elem
	std::optional<T> snap;							// Ha van auth snap, akkor az elején az

	std::map<qint64, T> *tmpPtr = nullptr;			// A tmp snapshot-listája
	std::map<qint64, T>::iterator tmpIt;			// A listában az első nem kisebb elem (x10)

	void step(const qint64 &tick);
};


template <typename T, typename T2,
		  typename = std::enable_if<std::is_base_of<RpgGameData::Body, T>::value>::type,
		  typename = std::enable_if<std::is_base_of<RpgGameData::BaseData, T2>::value>::type>
using RendererList = std::vector<RendererData<T, T2> >;



struct Renderer {
	qint64 tick = -1;
	RendererList<RpgGameData::Player, RpgGameData::PlayerBaseData> players;
	RendererList<RpgGameData::Enemy, RpgGameData::EnemyBaseData> enemies;
	RendererList<RpgGameData::Bullet, RpgGameData::BulletBaseData> bullets;

	qint64 step();
};



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

	Renderer getRenderer(const qint64 &tick);

	template <typename T, typename T2,
			  typename = std::enable_if< std::is_base_of<RpgGameData::Body, T>::value>::type,
			  typename = std::enable_if< std::is_base_of<RpgGameData::BaseData, T2>::value>::type>
	void addRenderer(RendererList<T, T2> &dest, RpgGameData::SnapshotData<T, T2> *src,
					 RpgGameData::SnapshotData<T, T2> *tmpSrc,
					 const qint64 &tick);

	void renderPlayer(Renderer *renderer, RendererData<RpgGameData::Player, RpgGameData::PlayerBaseData> &player);

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
	static std::map<qint64, T>::iterator getPreviousSnap(std::map<qint64, T> &list, const qint64 &tick);


	template <typename T,
			  typename = std::enable_if< std::is_base_of<RpgGameData::Body, T>::value>::type>
	static void copy(std::map<qint64, T> &dest, const std::map<qint64, T> &src, const qint64 &firstTick);


	template <typename T, typename T2,
			  typename = std::enable_if< std::is_base_of<RpgGameData::Body, T>::value>::type,
			  typename = std::enable_if< std::is_base_of<RpgGameData::BaseData, T2>::value>::type>
	static void removeLessThan(RpgGameData::SnapshotList<T, T2> &list, const qint64 &tick);



	RpgEngine *m_engine = nullptr;

	RpgGameData::CurrentSnapshot m_tmpSnapshot;
	qint64 m_lastAuthTick = -1;

	std::vector<RpgGameData::BaseData> m_lastBulletId;



	QString m_tmpTxt;
};



/**
 * @brief RpgSnapshotStorage::addRenderer
 * @param dest
 * @param src
 */

template<typename T, typename T2, typename T3, typename T4>
inline void RpgSnapshotStorage::addRenderer(RendererList<T, T2> &dest, RpgGameData::SnapshotData<T, T2> *src,
											RpgGameData::SnapshotData<T, T2> *tmpSrc, const qint64 &tick)
{
	Q_ASSERT(src);

	RendererData<T, T2> d;
	d.data = src->data;
	d.ptr = &src->list;
	d.it = getPreviousSnap(src->list, tick);

	if (d.it == src->list.end())
		qDebug() << "NOT FOUND";
	else if (d.it->first <= tick)
		d.snap = d.it->second;

	d.tmpPtr = tmpSrc ? &tmpSrc->list : nullptr;

	if (tmpSrc)
		d.tmpIt = tmpSrc->list.lower_bound(tick*10);

	dest.push_back(d);
}






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
 * @brief RpgSnapshotStorage::getPreviousSnap
 * @param list
 * @param tick
 * @param nextPtr
 * @return
 */

template<typename T>
inline std::map<qint64, T>::iterator RpgSnapshotStorage::getPreviousSnap(std::map<qint64, T> &list, const qint64 &tick)
{
	if (list.empty())
		return list.end();

	typename std::map<qint64, T>::iterator it = list.lower_bound(tick);			// Greater or equal

	if (it == list.end())
		return std::prev(it);
	else if (it->first == tick)
		return it;
	else if (it != list.begin())
		return std::prev(it);
	else
		return it;
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




template<typename T, typename T2, typename T3, typename T4>
inline void RendererData<T, T2, T3, T4>::step(const qint64 &tick)
{
	if (!ptr)
		return;

	if (it != ptr->end()) {
		if (std::next(it)->first == tick) {
			it = std::next(it);
		}
	}

	if (tmpPtr && tmpIt != tmpPtr->end()) {
		tmpIt = tmpPtr->lower_bound(tick*10);
	}
}

#endif // RPGSNAPSHOTSTORAGE_H
