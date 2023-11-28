#ifndef OBJECTSTATE_H
#define OBJECTSTATE_H

#include "Logger.h"
#include "qdebug.h"
#include "qpoint.h"
#include "qsize.h"
#include "qtypes.h"
#include <QIODevice>

#define OBJECT_STATE_BASE_VERSION 1

/**
 * @brief The ObjectStateObject class
 */

struct ObjectStateBase {
	virtual ~ObjectStateBase() = default;

	enum ObjectType {
		TypeInvalid = 0,
		TypeBase,
		TypeEntity,
		TypePlayer,
		TypeEnemy,
		TypeEnemySoldier,
		TypeEnemySniper
	};

	enum ServerState {
		StateInvalid = 0,
		StateActive,
		StateInactive,
		StateSleep
	};

	qint64 id = -1;					// object id
	ObjectType type = TypeBase;
	ServerState state = StateInvalid;
	qint64 tick = 0;

	QPointF position = {-1., -1.};
	QSizeF size = {-1., -1.};


	/**
	 * @brief toDataStream
	 * @param stream
	 */

	virtual void toDataStream(QDataStream &stream) const {
		stream << id;
		stream << type;
		stream << tick;
		stream << state;
		stream << position;
		stream << size;
	}


	virtual void toReadable(QByteArray *data) const {
		data->append(QStringLiteral("id: %1\n").arg(id).toUtf8());
		data->append(QStringLiteral("type: %1\n").arg(type).toUtf8());
		data->append(QStringLiteral("tick: %1\n").arg(tick).toUtf8());
		data->append(QStringLiteral("state: %1\n").arg(state).toUtf8());
		data->append(QStringLiteral("position: %1,%2\n").arg(position.x()).arg(position.y()).toUtf8());
		data->append(QStringLiteral("size: %1x%2\n").arg(size.width()).arg(size.height()).toUtf8());
	};

	/**
	 * @brief fromDataStream
	 * @param stream
	 * @param ptr
	 * @return
	 */

	virtual bool fromDataStream(QDataStream &stream) {
		stream >> id;
		stream >> type;
		stream >> tick;
		stream >> state;
		stream >> position;
		stream >> size;

		if (id == -1 || type == TypeInvalid) {
			LOG_CWARNING("app") << "Invalid stream exception";
			return false;
		}

		return true;
	}

	virtual ObjectStateBase* clone() const { return new ObjectStateBase(*this); }


	static ObjectStateBase* fromType(const ObjectType &type);
};




typedef std::vector<std::unique_ptr<ObjectStateBase> > ObjectStateVector;



/**
 * @brief The ObjectStateEntity class
 */

struct ObjectStateEntity : public ObjectStateBase {
	ObjectStateEntity() : ObjectStateBase() { type = TypeEntity; }

	qint32 hp = -1;
	qint32 maxHp = -1;
	bool facingLeft = false;

	virtual void toDataStream(QDataStream &stream) const override {
		ObjectStateBase::toDataStream(stream);

		stream << hp;
		stream << maxHp;
		stream << facingLeft;
	}

	virtual void toReadable(QByteArray *data) const override {
		ObjectStateBase::toReadable(data);
		data->append(QStringLiteral("hp: %1\n").arg(hp).toUtf8());
		data->append(QStringLiteral("maxHp: %1\n").arg(maxHp).toUtf8());
		data->append(QStringLiteral("facingLeft: %1\n").arg(facingLeft).toUtf8());
	};

	virtual bool fromDataStream(QDataStream &stream) override {
		if (!ObjectStateBase::fromDataStream(stream))
			return false;

		stream >> hp;
		stream >> maxHp;
		stream >> facingLeft;

		return true;
	}

	virtual ObjectStateBase* clone() const override { return new ObjectStateEntity(*this); }

};





/**
 * @brief The ObjectStateEnemy class
 */

struct ObjectStateEnemy : public ObjectStateEntity {
	ObjectStateEnemy() : ObjectStateEntity() { type = TypeEnemy; }

	enum EnemyState {
		Invalid = 0,
		Idle,
		Move,
		WatchPlayer,
		Attack,
		Dead
	};

	EnemyState enemyState = Invalid;
	double msecLeftToAttack = -1;

	virtual void toDataStream(QDataStream &stream) const override {
		ObjectStateEntity::toDataStream(stream);

		stream << enemyState;
		stream << msecLeftToAttack;
	}

	virtual void toReadable(QByteArray *data) const override {
		ObjectStateEntity::toReadable(data);
		data->append(QStringLiteral("enemyState: %1\n").arg(enemyState).toUtf8());
		data->append(QStringLiteral("msecLeftToAttack: %1\n").arg(msecLeftToAttack).toUtf8());
	};


	virtual bool fromDataStream(QDataStream &stream) override {
		if (!ObjectStateEntity::fromDataStream(stream))
			return false;

		stream >> enemyState;
		stream >> msecLeftToAttack;

		return true;
	}

	virtual ObjectStateBase* clone() const override { return new ObjectStateEnemy(*this); }
};




/**
 * @brief The ObjectStateEnemySoldier class
 */

struct ObjectStateEnemySoldier : public ObjectStateEnemy {
	ObjectStateEnemySoldier() : ObjectStateEnemy() { type = TypeEnemySoldier; }

	int turnElapsedMsec = 0;
	int attackElapsedMsec = 0;

	virtual void toDataStream(QDataStream &stream) const override {
		ObjectStateEnemy::toDataStream(stream);

		stream << turnElapsedMsec;
		stream << attackElapsedMsec;
	}

	virtual void toReadable(QByteArray *data) const override {
		ObjectStateEnemy::toReadable(data);
		data->append(QStringLiteral("turnElapsedMsec: %1\n").arg(turnElapsedMsec).toUtf8());
		data->append(QStringLiteral("attackElapsedMsec: %1\n").arg(attackElapsedMsec).toUtf8());
	};

	virtual bool fromDataStream(QDataStream &stream) override {
		if (!ObjectStateEnemy::fromDataStream(stream))
			return false;

		stream >> turnElapsedMsec;
		stream >> attackElapsedMsec;

		return true;
	}

	virtual ObjectStateBase* clone() const override { return new ObjectStateEnemySoldier(*this); }
};






/**
 * @brief The ObjectStateSnapshot class
 */

struct ObjectStateSnapshot {
	ObjectStateVector list;


	/**
	 * @brief append
	 * @param ptr
	 */

	void append(std::unique_ptr<ObjectStateBase> &ptr)
	{
		list.push_back(std::move(ptr));
	}

	/**
	 * @brief toByteArray
	 * @return
	 */

	QByteArray toByteArray() const
	{
		QByteArray s;
		QDataStream stream(&s, QIODevice::WriteOnly);
		stream.setVersion(QDataStream::Qt_5_15);

		quint32 version = OBJECT_STATE_BASE_VERSION;

		stream << (quint32) 0x434F53;			// COS
		stream << QByteArrayLiteral("OBJ");
		stream << version;
		stream << (quint64) list.size();

		for (const auto &b : list) {
			b->toDataStream(stream);
		}

		return s;
	}


	/**
	 * @brief toByteArray
	 * @param list
	 * @return
	 */

	static QByteArray toByteArray(const std::vector<ObjectStateBase*> &list)
	{
		QByteArray s;
		QDataStream stream(&s, QIODevice::WriteOnly);
		stream.setVersion(QDataStream::Qt_5_15);

		quint32 version = OBJECT_STATE_BASE_VERSION;

		stream << (quint32) 0x434F53;			// COS
		stream << QByteArrayLiteral("OBJ");
		stream << version;
		stream << (quint64) list.size();

		for (const auto &b : list) {
			b->toDataStream(stream);
		}

		return s;
	}


	/**
	 * @brief toReadable
	 * @return
	 */

	QByteArray toReadable() const
	{
		QByteArray s;

		s.append(QStringLiteral("Size: %1\n").arg(list.size()).toUtf8());
		s.append(QByteArrayLiteral("==============================================\n"));

		for (const auto &b : list) {
			b->toReadable(&s);
			s.append(QByteArrayLiteral("-----------------------------------------------\n"));
		}

		s.append(QByteArrayLiteral("==============================================\n"));

		return s;
	}


	/**
	 * @brief fromByteArray
	 * @param data
	 * @return
	 */

	static std::optional<ObjectStateSnapshot> fromByteArray(const QByteArray &data)
	{
		QDataStream stream(data);
		stream.setVersion(QDataStream::Qt_5_15);

		quint32 magic = 0;
		QByteArray str;

		stream >> magic >> str;

		if (magic != 0x434F53 || str != QByteArrayLiteral("OBJ")) {			// COS
			LOG_CWARNING("app") << "Invalid stream data";
			return std::nullopt;
		}

		quint32 version = 0;

		stream >> version;

		if (version != OBJECT_STATE_BASE_VERSION) {
			LOG_CERROR("app") << "Invalid stream version:" << version;
			return std::nullopt;
		}

		quint64 size = 0;

		ObjectStateSnapshot snap;

		stream >> size;

		snap.list.reserve(size);

		for (quint64 i=0; i<size; ++i) {
			stream.startTransaction();

			qint64 id = -1;
			ObjectStateBase::ObjectType type = ObjectStateBase::TypeInvalid;

			stream >> id >> type;

			stream.rollbackTransaction();

			std::unique_ptr<ObjectStateBase> _ptr(ObjectStateBase::fromType(type));

			if (_ptr && _ptr->fromDataStream(stream)) {
				snap.list.push_back(std::move(_ptr));
			}
		}

		snap.list.shrink_to_fit();

		return snap;
	}




	/**
	 * @brief moveTo
	 * @param src
	 * @param dest
	 * @return
	 */

	static void copyTo(ObjectStateBase *src, ObjectStateVector *dest)
	{
		Q_ASSERT(src);
		Q_ASSERT(dest);

		std::unique_ptr<ObjectStateBase> ptr(src->clone());
		dest->push_back(std::move(ptr));
	}


	/**
	 * @brief copyTo
	 * @param src
	 * @param dest
	 * @return
	 */

	static void copyTo(const std::unique_ptr<ObjectStateBase> &src, ObjectStateVector *dest)
	{
		Q_ASSERT(dest);

		std::unique_ptr<ObjectStateBase> ptr(src->clone());
		dest->push_back(std::move(ptr));
	}



	/**
	 * @brief moveTo
	 * @param src
	 * @param dest
	 */

	static void moveTo(std::unique_ptr<ObjectStateBase> &src, ObjectStateVector *dest)
	{
		Q_ASSERT(dest);

		std::unique_ptr<ObjectStateBase> ptr(src.release());
		dest->push_back(std::move(ptr));
	}

	/**
	 * @brief moveTo
	 * @param dest
	 */

	void moveTo(ObjectStateVector *dest)
	{
		Q_ASSERT(dest);

		dest->reserve(dest->size()+list.size());

		for (auto it=list.begin(); it != list.end(); ) {
			copyTo(*it, dest);
			it = list.erase(it);
		}

		dest->shrink_to_fit();
	}
};



/**
 * @brief ObjectStateBase::fromType
 * @param type
 * @return
 */


inline ObjectStateBase *ObjectStateBase::fromType(const ObjectType &type)
{
	ObjectStateBase *ptr = nullptr;

	switch (type) {
	case ObjectStateBase::TypeInvalid:
		LOG_CERROR("app") << "Invalid object state type";
		return nullptr;
		break;

	case ObjectStateBase::TypeBase:
		ptr = new ObjectStateBase();
		break;

	case ObjectStateBase::TypeEntity:
		ptr = new ObjectStateEntity();
		break;

	case ObjectStateBase::TypeEnemy:
		ptr = new ObjectStateEnemy();
		break;

	case ObjectStateBase::TypeEnemySoldier:
		ptr = new ObjectStateEnemySoldier();
		break;

	case ObjectStateBase::TypePlayer:
	case ObjectStateBase::TypeEnemySniper:
		LOG_CWARNING("app") << "Stream data skipped" << type;
		break;
	}

	return ptr;
}



#endif // OBJECTSTATE_H
