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
		Modified =		1 << 3,
		FromAuth =		1 << 4						// carry esetén növelnünk kell az auth storage snap számát
	};

	Q_DECLARE_FLAGS(RendererFlags, RendererFlag)


	const RendererFlags &flags() const { return m_flags; }
	void addFlags(const RendererFlags &flags);
	void removeFlags() { m_flags = None; }

	bool hasContent() const { return (m_flags & (Storage|Temporary)); }


protected:
	template <typename B,
			  typename = std::enable_if< std::is_base_of<RpgGameData::Body, B>::value>::type>
	QString dumpAs(const B &data, const QList<B> &subData) const;

	QString dumpAs(const RpgGameData::Player &data, const QList<RpgGameData::Player> &subData) const;
	QString dumpAs(const RpgGameData::Enemy &data, const QList<RpgGameData::Enemy> &subData) const;
	QString dumpAs(const RpgGameData::Bullet &data, const QList<RpgGameData::Bullet> &subData) const;

	QString dumpAs(const RpgGameData::ControlLight &data, const QList<RpgGameData::ControlLight> &subData) const;
	QString dumpAs(const RpgGameData::ControlContainer &data, const QList<RpgGameData::ControlContainer> &subData) const;
	QString dumpAs(const RpgGameData::ControlCollection &data, const QList<RpgGameData::ControlCollection> &subData) const;
	QString dumpAs(const RpgGameData::Pickable &data, const QList<RpgGameData::Pickable> &subData) const;
	QString dumpAs(const RpgGameData::ControlGate &data, const QList<RpgGameData::ControlGate> &subData) const;
	QString dumpAs(const RpgGameData::ControlTeleport &data, const QList<RpgGameData::ControlTeleport> &subData) const;

	RendererFlags m_flags = None;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(RendererType::RendererFlags)




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
	void dataOverride(const T &data);

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
	void renderAs(RendererItem<RpgGameData::Bullet> *src, RendererObjectType *self, Renderer *renderer) const;
	void renderAs(RendererItem<RpgGameData::Pickable> *src, RendererObjectType *self, Renderer *renderer) const;


	// Rendered by ControlUnique solver

	void renderAs(RendererItem<RpgGameData::ControlLight> *, RendererObjectType *, Renderer *) const {}
	void renderAs(RendererItem<RpgGameData::ControlContainer> *, RendererObjectType *, Renderer *) const {}
	void renderAs(RendererItem<RpgGameData::ControlCollection> *, RendererObjectType *, Renderer *) const {}
	void renderAs(RendererItem<RpgGameData::ControlGate> *, RendererObjectType *, Renderer *) const {}
	void renderAs(RendererItem<RpgGameData::ControlTeleport> *, RendererObjectType *, Renderer *) const {}



private:
	T m_data;						// Adat
	QList<T> m_subData;				// feldolgozandó adatok

	friend class Renderer;
};






/**
 * @brief The RendererObjectType class
 */

class RendererObjectType
{
public:
	RendererObjectType(Logger *logger)
		: m_logger(logger)
	{}

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
	virtual void postRender(Renderer *renderer) = 0;

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
	void overrideAuthSnap(const T &data);

	template <typename T,
			  typename = std::enable_if< std::is_base_of<RpgGameData::Body, T>::value>::type>
	bool addSubSnap(const int &index, const T &data);

	template <typename T,
			  typename = std::enable_if< std::is_base_of<RpgGameData::Body, T>::value>::type>
	bool carrySubSnap(const T &data);


	virtual QString dump(const qint64 &start, const int &size) const = 0;

	virtual bool isBaseEqual(const RpgGameData::BaseData &base) const = 0;

	virtual RpgGameData::BaseData asBaseData() const = 0;

	std::vector<std::unique_ptr<RendererType>> snap;

	void* userData() const { return m_userData; }
	void setUserData(void *data) { m_userData = data; }

	Logger* _logger() const { return m_logger; }

protected:
	void *m_userData = nullptr;
	Logger *const m_logger;

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
	RendererObject(Logger *logger)
		: RendererObjectType(logger)
	{}

	virtual bool isBaseEqual(const RpgGameData::BaseData &base) const override final {
		return baseData.isBaseEqual(base);
	}

	virtual RpgGameData::BaseData asBaseData() const override final { return baseData; }

	virtual QString dump(const qint64 &start, const int &size) const override;
	virtual void render(Renderer *renderer) override {
		get()->render(renderer, this);
	}
	void postRender(Renderer *renderer) override;

	T baseData;
};




/**
 * @brief The ConflictSolver class
 */

class ConflictSolver
{

private:
	Renderer *const m_renderer;

	struct ConflictData
	{
		ConflictData(const int &_tick)
			: tick(_tick)
		{}

		virtual ~ConflictData() = default;
		virtual bool solve(ConflictSolver *solver) = 0;
		virtual void generateEvent(ConflictSolver *solver, RpgEngine *engine) { Q_UNUSED(solver); Q_UNUSED(engine); }

		int tick = 0;
		bool solved = false;
	};


	/**
	 * @brief The ConflictWeaponUsage class
	 */

	template <typename T, typename T2,
			  typename = std::enable_if< std::is_base_of<RpgGameData::Body, T>::value>::type,
			  typename = std::enable_if< std::is_base_of<RpgGameData::BaseData, T2>::value>::type>
	struct ConflictWeaponUsage : public ConflictData
	{
		ConflictWeaponUsage(const int &_tick, RendererObject<T2> *_src,
							const RpgGameData::Weapon::WeaponType &_wType, const int &_wSubType)
			: ConflictData(_tick)
			, src(_src)
			, weaponType(_wType)
			, weaponSubType(_wSubType)
		{}

		virtual bool solve(ConflictSolver *solver) override;

		bool solveData(ConflictSolver *solver);

		RendererObject<T2> *const src;
		RpgGameData::Weapon::WeaponType weaponType = RpgGameData::Weapon::WeaponInvalid;
		int weaponSubType = 0;
	};








	/**
	 * @brief The ConflictRelocate class
	 */

	struct ConflictRelocate : public ConflictData
	{
		ConflictRelocate(const int &_tick, RendererObject<RpgGameData::PlayerBaseData> *_src,
						 const int &_scene, const QPointF &_pos, const float &_angle)
			: ConflictData(_tick)
			, src(_src)
			, scene(_scene)
			, position(_pos)
			, angle(_angle)
		{}

		virtual bool solve(ConflictSolver *solver) override;

		RendererObject<RpgGameData::PlayerBaseData> *const src;
		const int scene;
		const QPointF position;
		const float angle;
	};





	/**
	 * @brief The ConflictAttack class
	 */

	template <typename T, typename T2, typename T3, typename T4,
			  typename = std::enable_if< std::is_base_of<RpgGameData::Body, T>::value>::type,
			  typename = std::enable_if< std::is_base_of<RpgGameData::Body, T2>::value>::type,
			  typename = std::enable_if< std::is_base_of<RpgGameData::BaseData, T3>::value>::type,
			  typename = std::enable_if< std::is_base_of<RpgGameData::BaseData, T4>::value>::type>
	struct ConflictAttack : public ConflictData
	{
		ConflictAttack(const int &_tick,
					   RendererObject<T3> *_src,
					   RendererObject<T4> *_dest,
					   const RpgGameData::Weapon::WeaponType &_wType,
					   const int &_wStype)
			: ConflictData(_tick)
			, src(_src)
			, dest(_dest)
			, weaponType(_wType)
			, weaponSubType(_wStype)
		{}

		virtual bool solve(ConflictSolver *solver) override;


		RendererObject<T3> *const src;
		RendererObject<T4> *const dest;
		RpgGameData::Weapon::WeaponType weaponType = RpgGameData::Weapon::WeaponInvalid;
		int weaponSubType = 0;
	};





	/**
	 * @brief The ConflictControlIface class
	 */

	class ConflictUniqueIface
	{
	public:
		enum State {
			StateInvalid,
			StateHold,
			StateSuccess,
			StateFailed
		};

		struct ReleaseState
		{
			RendererObject<RpgGameData::PlayerBaseData> *player = nullptr;
			State state = StateInvalid;
			int xp = 0;

			bool operator==(const ReleaseState &other) {
				return other.player == player && other.state == state && other.xp == xp;
			}
		};

		ConflictUniqueIface(const int &tick, RendererObject<RpgGameData::PlayerBaseData> *player = nullptr);
		virtual ~ConflictUniqueIface() = default;

		virtual RendererObjectType *uniqueObject() const = 0;

		virtual void add(const int &tick, RendererObject<RpgGameData::PlayerBaseData> *src);
		virtual bool releaseSuccess(const int &tick, RendererObject<RpgGameData::PlayerBaseData> *src, const int &xp);
		virtual bool releaseFailed(const int &tick, RendererObject<RpgGameData::PlayerBaseData> *src);

		ReleaseState renderCurrentState(Renderer *renderer, const int &maxTick) const;

	protected:
		virtual void updateTick(const int &_tick) = 0;
		virtual bool getInitialState(Renderer *renderer, ReleaseState *destPtr) const = 0;

		bool setPlayer(Renderer *renderer, const RpgGameData::BaseData &base, ReleaseState *destPtr) const;

		QMap<int, ReleaseState> m_release;
	};


	/**
	 * @brief The ConflictDataUnique class
	 */


	template <typename T,
			  typename = std::enable_if< std::is_base_of<RpgGameData::ControlBaseData, T>::value>::type>
	class ConflictDataUnique : public ConflictData, public ConflictUniqueIface
	{
	public:
		ConflictDataUnique(const int &tick, RendererObject<T> *unique,
						   RendererObject<RpgGameData::PlayerBaseData> *player = nullptr)
			: ConflictData(tick)
			, ConflictUniqueIface(tick, player)
			, m_unique(unique)
		{
			solved = true;
		}

		virtual RendererObjectType *uniqueObject() const override { return m_unique; };
		virtual bool solve(ConflictSolver */*solver*/) override { return true; }

		virtual ~ConflictDataUnique() = default;

		static ConflictDataUnique<T> *find(const T &data, const std::vector<std::unique_ptr<ConflictData>> &list) {
			for (const auto &it : list) {
				if (ConflictDataUnique<T> *c = dynamic_cast<ConflictDataUnique<T>* >(it.get()); c && c->m_unique->baseData == data)
					return c;
			}

			return nullptr;
		}

	protected:
		virtual void updateTick(const int &_tick) override final {
			if (_tick < tick)
				tick = _tick;
		}

		RendererObject<T> *const m_unique;
	};






	/**
	 * @brief The ConflictContainer class
	 */

	class ConflictContainer : public ConflictDataUnique<RpgGameData::ControlContainerBaseData>
	{
	public:
		ConflictContainer(const int &_tick,
						  RendererObject<RpgGameData::ControlContainerBaseData> *_dst,
						  RendererObject<RpgGameData::PlayerBaseData> *_src);

		virtual void generateEvent(ConflictSolver *solver, RpgEngine *engine) override;

	protected:
		virtual bool getInitialState(Renderer *renderer, ReleaseState *destPtr) const override;
	};





	/**
	 * @brief The ConflictCollection class
	 */

	class ConflictCollection : public ConflictDataUnique<RpgGameData::ControlCollectionBaseData>
	{
	public:
		ConflictCollection(const int &_tick,
						   RendererObject<RpgGameData::ControlCollectionBaseData> *_dst,
						   RendererObject<RpgGameData::PlayerBaseData> *_src);

		virtual void generateEvent(ConflictSolver *solver, RpgEngine *engine) override;

	protected:
		virtual bool getInitialState(Renderer *renderer, ReleaseState *destPtr) const override;
	};




	/**
	 * @brief The ConflictPickable class
	 */

	class ConflictPickable : public ConflictDataUnique<RpgGameData::PickableBaseData>
	{
	public:
		ConflictPickable(const int &_tick,
						 RendererObject<RpgGameData::PickableBaseData> *_dst,
						 RendererObject<RpgGameData::PlayerBaseData> *_src);

		virtual void generateEvent(ConflictSolver *solver, RpgEngine *engine) override;

	protected:
		virtual bool getInitialState(Renderer *renderer, ReleaseState *destPtr) const override;
	};





	/**
	 * @brief The ConflictGate class
	 */

	class ConflictGate : public ConflictDataUnique<RpgGameData::ControlGateBaseData>
	{
	public:
		ConflictGate(const int &_tick,
					 RendererObject<RpgGameData::ControlGateBaseData> *_dst,
					 RendererObject<RpgGameData::PlayerBaseData> *_src);

		virtual void generateEvent(ConflictSolver *solver, RpgEngine *engine) override;

	protected:
		virtual bool getInitialState(Renderer *renderer, ReleaseState *destPtr) const override;
	};





	/**
	 * @brief The ConflictTeleport class
	 */

	class ConflictTeleport : public ConflictDataUnique<RpgGameData::ControlTeleportBaseData>
	{
	public:
		ConflictTeleport(const int &_tick,
						 RendererObject<RpgGameData::ControlTeleportBaseData> *_dst,
						 RendererObject<RpgGameData::PlayerBaseData> *_src);

		virtual void generateEvent(ConflictSolver *solver, RpgEngine *engine) override;

	protected:
		virtual bool getInitialState(Renderer *renderer, ReleaseState *destPtr) const override;
	};







	template <typename T, typename ...Args,
			  typename = std::enable_if< std::is_base_of<ConflictData, T>::value>::type>
	T* addData(Args && ...args);


	template <typename T, typename T2, typename ...Args,
			  typename = std::enable_if< std::is_base_of<ConflictData, T>::value>::type,
			  typename = std::enable_if< std::is_base_of<ConflictUniqueIface, T>::value>::type,
			  typename = std::enable_if< std::is_base_of<RpgGameData::ControlBaseData, T2>::value>::type>
	T* addUniqueData(const int &tick, RendererObject<T2> *unique, Args && ...args)
	{
		Q_ASSERT(unique);

		if (T *c = dynamic_cast<T*>(T::find(unique->baseData, m_list))) {
			c->add(tick, std::forward<Args>(args)...);
			return c;
		} else {
			return addData<T>(tick, unique, std::forward<Args>(args)...);
		}
	}





public:
	ConflictSolver(Renderer *renderer);

	void add(const int &tick,
			 RendererObject<RpgGameData::PlayerBaseData> *src,
			 const RpgGameData::Weapon::WeaponType &weaponType,
			 const int &subType)
	{
		addData<ConflictWeaponUsage<RpgGameData::Player, RpgGameData::PlayerBaseData> >(tick, src, weaponType, subType);
	}

	void add(const int &tick,
			 RendererObject<RpgGameData::EnemyBaseData> *src,
			 const RpgGameData::Weapon::WeaponType &weaponType,
			 const int &subType)
	{
		addData<ConflictWeaponUsage<RpgGameData::Enemy, RpgGameData::EnemyBaseData> >(tick, src, weaponType, subType);
	}

	void add(const int &tick,
			 RendererObject<RpgGameData::PlayerBaseData> *src,
			 RendererObject<RpgGameData::EnemyBaseData> *dest,
			 const RpgGameData::Weapon::WeaponType &weaponType,
			 const int &subType)
	{
		addData<ConflictAttack<RpgGameData::Player, RpgGameData::Enemy,
				RpgGameData::PlayerBaseData, RpgGameData::EnemyBaseData> >(tick, src, dest, weaponType, subType);
	}

	void add(const int &tick,
			 RendererObject<RpgGameData::EnemyBaseData> *src,
			 RendererObject<RpgGameData::PlayerBaseData> *dest,
			 const RpgGameData::Weapon::WeaponType &weaponType,
			 const int &subType)
	{
		addData<ConflictAttack<RpgGameData::Enemy, RpgGameData::Player,
				RpgGameData::EnemyBaseData, RpgGameData::PlayerBaseData> >(tick, src, dest, weaponType, subType);
	}

	void add(const int &tick,
			 RendererObject<RpgGameData::PlayerBaseData> *src,
			 const int &scene, const QPointF &pos, const float &angle)
	{
		addData<ConflictRelocate>(tick, src, scene, pos, angle);
	}


	ConflictUniqueIface* addUnique(const int &tick, RendererObjectType *src, RendererObjectType *dest);

	bool solve();

	void generateEvents(RpgEngine *engine, const int &tick);

	Logger *_logger() const;


private:
	std::vector<std::unique_ptr<ConflictData>> m_list;
};







/**
 * @brief The Renderer class
 */

class Renderer
{
public:
	Renderer(const qint64 &start, const int &size, Logger *logger);
	virtual ~Renderer();

	const qint64 &startTick() const { return m_startTick; }
	const int &size() const { return m_size; }
	const int &current() const { return m_current; }

	bool render();
	void render(RendererItem<RpgGameData::Player> *dst, RendererObject<RpgGameData::PlayerBaseData> *src);
	void render(RendererItem<RpgGameData::Enemy> *dst, RendererObject<RpgGameData::EnemyBaseData> *src);
	void render(RendererItem<RpgGameData::Bullet> *dst, RendererObject<RpgGameData::BulletBaseData> *src);

	void render(RendererItem<RpgGameData::Pickable> *dst, RendererObject<RpgGameData::PickableBaseData> *src);

	bool step();

	QString dump() const;

	void generateSolverEvents(RpgEngine *engine, const int &tick) { m_solver.generateEvents(engine, tick); }



	template <typename T,
			  typename = std::enable_if< std::is_base_of<RpgGameData::BaseData, T>::value>::type>
	void postRender(RendererObject<T> *object) { Q_UNUSED(object); }

	void postRender(RendererObject<RpgGameData::ControlCollectionBaseData> *object);




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

	template <typename T,
			  typename = std::enable_if< std::is_base_of<RpgGameData::BaseData, T>::value>::type>
	RendererObject<T>* findByBase(const RpgGameData::BaseData &baseData) const;

	RendererObjectType* findByBase(const RpgGameData::BaseData &baseData) const;


	template <typename T, typename T2,
			  typename = std::enable_if< std::is_base_of<RpgGameData::Body, T>::value>::type,
			  typename = std::enable_if< std::is_base_of<RpgGameData::BaseData, T2>::value>::type>
	bool loadSnaps(RpgGameData::SnapshotList<T, T2> &list, const bool &isStorage);

	template <typename T, typename T2,
			  typename = std::enable_if< std::is_base_of<RpgGameData::Body, T>::value>::type,
			  typename = std::enable_if< std::is_base_of<RpgGameData::BaseData, T2>::value>::type>
	bool loadAuthSnaps(const RpgGameData::SnapshotList<T, T2> &src);


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
	RendererItem<T> *rewind(RendererObjectType *src) const;

	template <typename T,
			  typename = std::enable_if< std::is_base_of<RpgGameData::Body, T>::value>::type>
	RendererItem<T> *forward(RendererObjectType *src) const;

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



	template <typename T, typename T2,
			  typename = std::enable_if< std::is_base_of<RpgGameData::Body, T>::value>::type,
			  typename = std::enable_if< std::is_base_of<RpgGameData::BaseData, T2>::value>::type>
	std::optional<T> extendFromLast(RendererObject<T2> *src);


	template <typename T,
			  typename = std::enable_if< std::is_base_of<RpgGameData::BaseData, T>::value>::type>
	static QString dumpBaseDataAs(const RendererObject<T> *obj);

	static QString dumpBaseDataAs(const RendererObject<RpgGameData::PlayerBaseData> *obj);
	static QString dumpBaseDataAs(const RendererObject<RpgGameData::PickableBaseData> *obj);
	static QString dumpBaseDataAs(const RendererObject<RpgGameData::ControlTeleportBaseData> *obj);
	static QString dumpBaseDataAs(const RendererObject<RpgGameData::ControlCollectionBaseData> *obj);

	Logger *_logger() const { return m_logger; }

private:
	template <typename T, typename T2,
			  typename = std::enable_if< std::is_base_of<RpgGameData::Body, T>::value>::type,
			  typename = std::enable_if< std::is_base_of<RpgGameData::BaseData, T2>::value>::type>
	bool loadStorageSnaps(RendererObject<T2> *dst, RpgGameData::SnapshotData<T, T2> &ptr);

	template <typename T, typename T2,
			  typename = std::enable_if< std::is_base_of<RpgGameData::Body, T>::value>::type,
			  typename = std::enable_if< std::is_base_of<RpgGameData::BaseData, T2>::value>::type>
	bool loadTemporarySnaps(RendererObject<T2> *dst, RpgGameData::SnapshotData<T, T2> &ptr);


	void restore(RpgGameData::Player *dst, const RpgGameData::Player &data);
	void restore(RpgGameData::Enemy *dst, const RpgGameData::Enemy &data);
	void restore(RpgGameData::Bullet *dst, const RpgGameData::Bullet &data);

	void restore(RpgGameData::ControlLight *dst, const RpgGameData::ControlLight &data);
	void restore(RpgGameData::ControlContainer *dst, const RpgGameData::ControlContainer &data);
	void restore(RpgGameData::ControlCollection *dst, const RpgGameData::ControlCollection &data);
	void restore(RpgGameData::Pickable *dst, const RpgGameData::Pickable &data);
	void restore(RpgGameData::ControlGate *dst, const RpgGameData::ControlGate &data);
	void restore(RpgGameData::ControlTeleport *dst, const RpgGameData::ControlTeleport &data);


	const qint64 m_startTick;
	const int m_size;
	int m_current = 0;
	std::vector<std::unique_ptr<RendererObjectType>> m_objects;
	ConflictSolver m_solver;
	Logger *const m_logger;
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

	void lightAdd(const RpgGameData::ControlBaseData &base, const RpgGameData::ControlLight &data);
	void containerAdd(const RpgGameData::ControlContainerBaseData &base, const RpgGameData::ControlContainer &data);
	void collectionAdd(const RpgGameData::ControlCollectionBaseData &base, const RpgGameData::ControlCollection &data);
	RpgGameData::PickableBaseData pickableAdd(const RpgGameData::PickableBaseData::PickableType &type,
											  const int &scene, const QPointF &pos);
	void gateAdd(const RpgGameData::ControlGateBaseData &base, const RpgGameData::ControlGate &data);
	void teleportAdd(const RpgGameData::ControlTeleportBaseData &base, const RpgGameData::ControlTeleport &data);

	int lastLifeCycleId(const RpgGameData::BaseData &base, std::vector<RpgGameData::BaseData>::iterator *ptr = nullptr);
	int lastLifeCycleId(const int &owner, std::vector<RpgGameData::BaseData>::iterator *ptr = nullptr);
	bool setLastLifeCycleId(const RpgGameData::BaseData &base);
	bool setLastLifeCycleId(const std::vector<RpgGameData::BaseData>::iterator &iterator, const RpgGameData::BaseData &base);

	template <typename T, typename T2,
			  typename = std::enable_if< std::is_base_of<RpgGameData::Body, T>::value>::type,
			  typename = std::enable_if< std::is_base_of<RpgGameData::BaseData, T2>::value>::type>
	bool append(const T2 &key, const T &data);

	bool append(const RpgGameData::EnemyBaseData &key, const RpgGameData::Enemy &data);

	bool registerSnapshot(RpgEnginePlayer *player, const QCborMap &cbor, const qint64 &diff);

	QString render(const qint64 &tick);
	void renderEnd(const QString &txt);

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
	qint64 droppedSnapOverride(const std::pair<const qint64, T> &src, const qint64 &firtTick,
							   RpgEnginePlayer *player, const qint64 &diff) const;

	qint64 droppedSnapOverride(const std::pair<const qint64, RpgGameData::Player> &src, const qint64 &firstTick,
							   RpgEnginePlayer *player, const qint64 &diff) const;

	qint64 droppedSnapOverride(const std::pair<const qint64, RpgGameData::Enemy> &src, const qint64 &firstTick,
							   RpgEnginePlayer *player, const qint64 &diff) const;

	template <typename T,
			  typename = std::enable_if< std::is_base_of<RpgGameData::Body, T>::value>::type>
	void copy(std::map<qint64, T> &dest, const std::map<qint64, T> &src, const qint64 &firstTick,
			  RpgEnginePlayer *player, const qint64 &diff) const;


	template <typename T, typename T2,
			  typename = std::enable_if< std::is_base_of<RpgGameData::Body, T>::value>::type,
			  typename = std::enable_if< std::is_base_of<RpgGameData::BaseData, T2>::value>::type>
	static void removeLessThan(RpgGameData::SnapshotList<T, T2> &list, const qint64 &tick);

	template <typename T, typename T2,
			  typename = std::enable_if< std::is_base_of<RpgGameData::Body, T>::value>::type,
			  typename = std::enable_if< std::is_base_of<RpgGameData::LifeCycle, T>::value>::type,
			  typename = std::enable_if< std::is_base_of<RpgGameData::BaseData, T2>::value>::type>
	static QList<T2> removeOutdated(RpgGameData::SnapshotList<T, T2> &list, const qint64 &tick);

	template <typename T, typename T2,
			  typename = std::enable_if< std::is_base_of<RpgGameData::Body, T>::value>::type,
			  typename = std::enable_if< std::is_base_of<RpgGameData::BaseData, T2>::value>::type>
	static void removeList(RpgGameData::SnapshotList<T, T2> &list, const QList<T2> &ids);

	Logger *_logger() const;

private:
	bool registerPlayers(RpgEnginePlayer *player, const QCborMap &cbor, const qint64 &diff);
	bool registerEnemies(const RpgGameData::CurrentSnapshot &snapshot, RpgEnginePlayer *player, const qint64 &diff);
	bool registerBullets(const RpgGameData::CurrentSnapshot &snapshot, RpgEnginePlayer *player, const qint64 &diff);

	std::unique_ptr<Renderer> getRenderer(const qint64 &tick);
	int saveRenderer(Renderer *renderer, const uint &pass);


	template <typename T, typename T2,
			  typename = std::enable_if< std::is_base_of<RpgGameData::Body, T>::value>::type,
			  typename = std::enable_if< std::is_base_of<RpgGameData::BaseData, T2>::value>::type>
	int generateEvents(const RpgGameData::SnapshotData<T, T2> &snapshot, const qint64 &tick);



	RpgEngine *m_engine = nullptr;

	RpgGameData::CurrentSnapshot m_tmpSnapshot;
	qint64 m_lastAuthTick = -1;

	std::vector<RpgGameData::BaseData> m_lastLifeCycleId;
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



/**
 * @brief RendererItem::renderAs
 * @param src
 * @param self
 * @param renderer
 */

template<typename T, typename T2>
inline void RendererItem<T, T2>::renderAs(RendererItem<RpgGameData::Bullet> *src, RendererObjectType *self, Renderer *renderer) const {
	RendererObject<RpgGameData::BulletBaseData> *p = dynamic_cast<RendererObject<RpgGameData::BulletBaseData>*>(self);
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
inline void RendererItem<T, T2>::renderAs(RendererItem<RpgGameData::Pickable> *src, RendererObjectType *self, Renderer *renderer) const
{
	RendererObject<RpgGameData::PickableBaseData> *p = dynamic_cast<RendererObject<RpgGameData::PickableBaseData>*>(self);
	Q_ASSERT(p);
	renderer->render(src, p);
}



/**
 * @brief RendererObject::postRender
 * @param renderer
 */

template<typename T, typename T2>
inline void RendererObject<T, T2>::postRender(Renderer *renderer) {
	Q_ASSERT(renderer);
	renderer->postRender(this);
}







/**
 * @brief Renderer::dumpBaseDataAs
 * @param obj
 * @return
 */

template<typename T, typename T2>
inline QString Renderer::dumpBaseDataAs(const RendererObject<T> *obj)
{
	Q_ASSERT(obj);

	return QStringLiteral("Object %1 %2 %3\n-------------------------------------------\n")
			.arg(obj->baseData.o)
			.arg(obj->baseData.s)
			.arg(obj->baseData.id);
}



/**
 * @brief RpgSnapshotStorage::append
 * @param key
 * @param data
 * @return
 */

template<typename T, typename T2, typename T3, typename T4>
inline bool RpgSnapshotStorage::append(const T2 &/*key*/, const T &/*data*/)
{
	LOG_CERROR("engine") << "Missing specialization";
	return false;
}


#endif // RPGSNAPSHOTSTORAGE_H
