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

#include "Logger.h"
#include "rpgconfig.h"

class RpgEngine;
class RpgEnginePlayer;
class RpgSnapshotStorage;

class Renderer;
class RendererObjectType;



/**
 * @brief The RendererType class
 */

class RendererType
{
public:
	RendererType() = default;
	virtual ~RendererType() {}

	virtual QString dump() const = 0;
	virtual void render(Renderer *renderer, RendererObjectType *self) = 0;

	enum RendererFlag {
		None =			0,
		ReadOnly =		1,
		Storage =		1 << 1,
		Temporary =		1 << 2,
		Modified =		1 << 3
	};

	Q_DECLARE_FLAGS(RendererFlags, RendererFlag)


	const RendererFlags &flags() const { return m_flags; }
	void addFlags(const RendererFlags &flags);

	bool hasContent() const { return (m_flags & (Storage|Temporary)); }

protected:
	template <typename B,
			  typename = std::enable_if< std::is_base_of<RpgGameData::Body, B>::value>::type>
	QString dumpAs(const B &data, const QList<B> &subData) const;

	QString dumpAs(const RpgGameData::Player &data, const QList<RpgGameData::Player> &subData) const;
	QString dumpAs(const RpgGameData::Enemy &data, const QList<RpgGameData::Enemy> &subData) const;

	RendererFlags m_flags = None;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(RendererType::RendererFlags)





/**
 * @brief The RendererObjectType class
 */

class RendererObjectType
{
public:
	RendererObjectType() = default;
	virtual ~RendererObjectType() { snap.clear(); }

	RendererType* snapAt(const int &index) const;

	const std::vector<std::unique_ptr<RendererType>>::const_iterator &iterator() const { return m_iterator; }

	RendererType *get() const {
		Q_ASSERT(m_iterator != snap.cend());
		return m_iterator->get();
	}

	RendererType *cprev() const {
		if (m_iterator == snap.begin())
			return nullptr;
		return std::prev(m_iterator)->get();
	}

	RendererType *cnext() const {
		if (m_iterator == snap.end() || std::next(m_iterator) == snap.end())
			return nullptr;
		return std::next(m_iterator)->get();
	}

	RendererType *prev() {
		if (m_iterator == snap.begin())
			return nullptr;
		--m_iterator;
		return m_iterator->get();
	}

	RendererType *next()  {
		if (m_iterator == snap.end())
			return nullptr;
		++m_iterator;
		if (m_iterator == snap.end())
			return nullptr;
		else
			return m_iterator->get();
	}

	virtual void render(Renderer *renderer) = 0;

	template <typename T,
			  typename = std::enable_if< std::is_base_of<RpgGameData::Body, T>::value>::type>
	void fillSnap(const int &size);

	template <typename T,
			  typename = std::enable_if< std::is_base_of<RpgGameData::Body, T>::value>::type>
	bool setSnap(const int &index, const T &data);

	template <typename T,
			  typename = std::enable_if< std::is_base_of<RpgGameData::Body, T>::value>::type>
	bool setSnap(const int &index, const T &data, const RendererType::RendererFlags &addFlags);

	template <typename T,
			  typename = std::enable_if< std::is_base_of<RpgGameData::Body, T>::value>::type>
	bool setAuthSnap(const T &data);

	template <typename T,
			  typename = std::enable_if< std::is_base_of<RpgGameData::Body, T>::value>::type>
	bool addSubSnap(const int &index, const T &data);

	virtual QString dump(const qint64 &start, const int &size) const = 0;

	std::vector<std::unique_ptr<RendererType>> snap;

private:
	std::vector<std::unique_ptr<RendererType>>::const_iterator m_iterator;
};




/**
 * @brief The RendererObject class
 */


template <typename T,
		  typename = std::enable_if< std::is_base_of<RpgGameData::BaseData, T>::value>::type>
class RendererObject : public RendererObjectType
{
public:
	RendererObject() = default;

	virtual QString dump(const qint64 &start, const int &size) const override;
	virtual void render(Renderer *renderer) override {
		get()->render(renderer, this);
	}

	T baseData;
};


/**
 * @brief The RendererItem class
 */


template <typename T,
		  typename = std::enable_if< std::is_base_of<RpgGameData::Body, T>::value>::type>
class RendererItem : public RendererType
{
public:
	RendererItem() = default;

	const T &data() const { return m_data; }
	bool setData(const T &data);

	const QList<T> &subData() const { return m_subData; }
	void addSubData(const T &data) { m_subData.append(data); }
	void clearSubData() {
		m_subData.clear();
		m_flags.setFlag(Temporary, false);
	}

	virtual QString dump() const override {
		return dumpAs(m_data, m_subData);
	}
	virtual void render(Renderer *renderer, RendererObjectType *self) override {
		if (!hasContent())
			return;

		renderAs(this, self, renderer);
	}

private:
	template <typename B,
			  typename = std::enable_if< std::is_base_of<RpgGameData::Body, B>::value>::type>
	void renderAs(RendererItem<B> *src, RendererObjectType *self, Renderer *renderer) const {
		Q_ASSERT(src);
		Q_ASSERT(self);
		Q_ASSERT(renderer);
		LOG_CERROR("engine") << "Missing specialization";
		return;
	}

	void renderAs(RendererItem<RpgGameData::Player> *src, RendererObjectType *self, Renderer *renderer) const;
	void renderAs(RendererItem<RpgGameData::Enemy> *src, RendererObjectType *self, Renderer *renderer) const;



private:
	T m_data;						// Adat
	QList<T> m_subData;				// feldolgozandó adatok

	friend class Renderer;
};




/**
 * @brief The ConflictSolver class
 */

class ConflictSolver
{
public:
	ConflictSolver(Renderer *renderer);

	enum Type {
		Invalid = 0,
		WeaponUsage,
		Damage
	};

	void add(const RpgGameData::Weapon::WeaponType &weaponType, const int &tick, RendererObjectType *src, RendererObjectType *dest);

	bool solve();

private:
	Renderer *const m_renderer;

	struct ConflictData {
		ConflictData(const Type &_type, const int &_tick, RendererObjectType *_src, RendererObjectType *_dest)
			: type(_type)
			, src(_src)
			, dest(_dest)
			, tick(_tick)
		{}

		virtual ~ConflictData() = default;
		virtual bool solve(ConflictSolver *solver) = 0;

		const Type type = Invalid;
		RendererObjectType *src = nullptr;
		RendererObjectType *dest = nullptr;
		int tick = -1;
		int delayedTo = -1;								// Csak ekkor lesz éles
		bool solved = false;
	};

	struct ConflictDataWeaponUsage : public ConflictData {
		ConflictDataWeaponUsage(const int &_tick, RendererObjectType *_src, RendererObjectType *_dest,
								const RpgGameData::Weapon::WeaponType &_wType)
			: ConflictData(WeaponUsage, _tick, _src, _dest)
			, weaponType(_wType)
		{
			delayedTo = _tick;
		}

		virtual bool solve(ConflictSolver *solver) override;

		RpgGameData::Weapon::WeaponType weaponType = RpgGameData::Weapon::WeaponInvalid;
	};


	template <typename T, typename ...Args,
			  typename = std::enable_if< std::is_base_of<ConflictData, T>::value>::type>
	T* addData(Args && ...args);

	std::vector<std::unique_ptr<ConflictData>> m_list;
};



/**
 * @brief The Renderer class
 */

class Renderer
{
public:
	Renderer(const qint64 &start, const int &size);
	virtual ~Renderer();

	const qint64 &startTick() const { return m_startTick; }
	const int &size() const { return m_size; }
	const int &current() const { return m_current; }

	bool render();
	void render(RendererItem<RpgGameData::Player> *dst, RendererObject<RpgGameData::PlayerBaseData> *src);
	void render(RendererItem<RpgGameData::Enemy> *dst, RendererObject<RpgGameData::EnemyBaseData> *src);

	bool step();

	QString dump() const;

	template <typename T, typename T2,
			  typename = std::enable_if< std::is_base_of<RpgGameData::Body, T>::value>::type,
			  typename = std::enable_if< std::is_base_of<RpgGameData::BaseData, T2>::value>::type>
	bool addObjects(const RpgGameData::SnapshotList<T, T2> &list);

	template <typename T, typename T2,
			  typename = std::enable_if< std::is_base_of<RpgGameData::Body, T>::value>::type,
			  typename = std::enable_if< std::is_base_of<RpgGameData::BaseData, T2>::value>::type>
	int saveObjects(RpgGameData::SnapshotList<T, T2> &dst);


	template <typename T,
			  typename = std::enable_if< std::is_base_of<RpgGameData::BaseData, T>::value>::type>
	RendererObject<T>* find(const T &baseData) const;


	template <typename T, typename T2,
			  typename = std::enable_if< std::is_base_of<RpgGameData::Body, T>::value>::type,
			  typename = std::enable_if< std::is_base_of<RpgGameData::BaseData, T2>::value>::type>
	bool loadSnaps(RpgGameData::SnapshotList<T, T2> &list, const bool &isStorage);


	template <typename T,
			  typename = std::enable_if< std::is_base_of<RpgGameData::Body, T>::value>::type>
	RendererItem<T> *snapAt(RendererObjectType *object, const int &index) const;

	template <typename T,
			  typename = std::enable_if< std::is_base_of<RpgGameData::Body, T>::value>::type>
	RendererItem<T> *get(RendererObjectType *object) const
	{
		Q_ASSERT(object);
		return dynamic_cast<RendererItem<T>*> (object->get());
	}

	template <typename T,
			  typename = std::enable_if< std::is_base_of<RpgGameData::Body, T>::value>::type>
	RendererItem<T> *prev(RendererObjectType *object) const
	{
		Q_ASSERT(object);
		return dynamic_cast<RendererItem<T>*> (object->prev());
	}

	template <typename T,
			  typename = std::enable_if< std::is_base_of<RpgGameData::Body, T>::value>::type>
	RendererItem<T> *next(RendererObjectType *object) const
	{
		Q_ASSERT(object);
		return dynamic_cast<RendererItem<T>*> (object->next());
	}

	template <typename T,
			  typename = std::enable_if< std::is_base_of<RpgGameData::Body, T>::value>::type>
	RendererItem<T> *cprev(RendererObjectType *object) const
	{
		Q_ASSERT(object);
		return dynamic_cast<RendererItem<T>*> (object->cprev());
	}

	template <typename T,
			  typename = std::enable_if< std::is_base_of<RpgGameData::Body, T>::value>::type>
	RendererItem<T> *cnext(RendererObjectType *object) const
	{
		Q_ASSERT(object);
		return dynamic_cast<RendererItem<T>*> (object->cnext());
	}


private:
	template <typename T, typename T2,
			  typename = std::enable_if< std::is_base_of<RpgGameData::Body, T>::value>::type,
			  typename = std::enable_if< std::is_base_of<RpgGameData::BaseData, T2>::value>::type>
	bool loadStorageSnaps(RendererObject<T2> *dst, RpgGameData::SnapshotData<T, T2> &ptr);

	template <typename T, typename T2,
			  typename = std::enable_if< std::is_base_of<RpgGameData::Body, T>::value>::type,
			  typename = std::enable_if< std::is_base_of<RpgGameData::BaseData, T2>::value>::type>
	bool loadTemporarySnaps(RendererObject<T2> *dst, RpgGameData::SnapshotData<T, T2> &ptr);


	/*bool loadStorageSnaps(RendererObject<RpgGameData::PlayerBaseData> *dst,
						  RpgGameData::SnapshotData<RpgGameData::Player, RpgGameData::PlayerBaseData> &ptr);

	bool loadTemporarySnaps(RendererObject<RpgGameData::PlayerBaseData> *dst,
							RpgGameData::SnapshotData<RpgGameData::Player, RpgGameData::PlayerBaseData> &ptr);

	bool loadStorageSnaps(RendererObject<RpgGameData::EnemyBaseData> *dst,
						  RpgGameData::SnapshotData<RpgGameData::Enemy, RpgGameData::EnemyBaseData> &ptr);

	bool loadTemporarySnaps(RendererObject<RpgGameData::EnemyBaseData> *dst,
							RpgGameData::SnapshotData<RpgGameData::Enemy, RpgGameData::EnemyBaseData> &ptr);*/


	void restore(RpgGameData::Player *dst, const RpgGameData::Player &data);
	void restore(RpgGameData::Enemy *dst, const RpgGameData::Enemy &data);

	template <typename T, typename T2,
			  typename = std::enable_if< std::is_base_of<RpgGameData::Body, T>::value>::type,
			  typename = std::enable_if< std::is_base_of<RpgGameData::BaseData, T2>::value>::type>
	std::optional<T> extendFromLast(RendererObject<T2> *src);

	const qint64 m_startTick;
	const int m_size;
	int m_current = 0;
	std::vector<std::unique_ptr<RendererObjectType>> m_objects;
	ConflictSolver m_solver;
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


	template <typename T>
	static std::map<qint64, T>::iterator getPreviousSnap(std::map<qint64, T> &list, const qint64 &tick);


	template <typename T,
			  typename = std::enable_if< std::is_base_of<RpgGameData::Body, T>::value>::type>
	static void copy(std::map<qint64, T> &dest, const std::map<qint64, T> &src, const qint64 &firstTick);


	template <typename T, typename T2,
			  typename = std::enable_if< std::is_base_of<RpgGameData::Body, T>::value>::type,
			  typename = std::enable_if< std::is_base_of<RpgGameData::BaseData, T2>::value>::type>
	static void removeLessThan(RpgGameData::SnapshotList<T, T2> &list, const qint64 &tick);

private:
	bool registerPlayers(RpgEnginePlayer *player, const QCborMap &cbor);
	bool registerEnemies(const RpgGameData::CurrentSnapshot &snapshot);

	std::unique_ptr<Renderer> getRenderer(const qint64 &tick);
	bool saveRenderer(Renderer *renderer);


	RpgGameData::Player actionPlayer(RpgGameData::PlayerBaseData &pdata, RpgGameData::Player &snap);
	RpgGameData::Bullet actionBullet(RpgGameData::Bullet &snap,
									 RpgGameData::SnapshotList<RpgGameData::Bullet, RpgGameData::BulletBaseData>::iterator snapIterator,
									 std::map<qint64, RpgGameData::Bullet>::iterator nextIterator);



	RpgEngine *m_engine = nullptr;

	RpgGameData::CurrentSnapshot m_tmpSnapshot;
	qint64 m_lastAuthTick = -1;

	std::vector<RpgGameData::BaseData> m_lastBulletId;
};








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

	typename std::map<qint64, T>::iterator it = list.upper_bound(tick);			// Greater

	if (it != list.begin())
		return std::prev(it);
	else
		return list.end();
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




/**
 * @brief RendererItem::renderAs
 * @param src
 * @param self
 * @param renderer
 */

template<typename T, typename T2>
inline void RendererItem<T, T2>::renderAs(RendererItem<RpgGameData::Player> *src, RendererObjectType *self, Renderer *renderer) const {
	RendererObject<RpgGameData::PlayerBaseData> *p = dynamic_cast<RendererObject<RpgGameData::PlayerBaseData>*>(self);
	Q_ASSERT(p);
	renderer->render(src, p);
}




/**
 * @brief RendererItem::renderAs
 * @param src
 * @param self
 * @param renderer
 */

template<typename T, typename T2>
inline void RendererItem<T, T2>::renderAs(RendererItem<RpgGameData::Enemy> *src, RendererObjectType *self, Renderer *renderer) const {
	RendererObject<RpgGameData::EnemyBaseData> *p = dynamic_cast<RendererObject<RpgGameData::EnemyBaseData>*>(self);
	Q_ASSERT(p);
	renderer->render(src, p);
}

#endif // RPGSNAPSHOTSTORAGE_H
