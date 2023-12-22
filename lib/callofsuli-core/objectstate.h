#ifndef OBJECTSTATE_H
#define OBJECTSTATE_H

#include "Logger.h"
#include "qdebug.h"
#include "qpoint.h"
#include "qsize.h"
#include "qtypes.h"
#include <QIODevice>
#include <QRectF>

#define OBJECT_STATE_BASE_VERSION 1

enum MultiPlayerGameState {
    StateInvalid = 0,
    StateConnecting,
    StateCreating,
    StatePreparing,
    StatePlaying,
    StateFinished,
    StateError
};




#define FIELD_TO_STREAM(fields, stream, x, y)   if (fields.testFlag(x)) stream << y;
#define FIELD_FROM_STREAM(fields, stream, x, y)   if (fields.testFlag(x)) stream >> y;
#define FIELD_DIFF(fields, obj, x, y)    fields.setFlag(x, (obj.y != y));
#define FIELD_PATCH(obj, pFields, x, y)     if (obj.fields.testFlag(x) && pFields.testFlag(x)) y = obj.y;

/**
 * @brief The ObjectStateObject class
 */

struct ObjectStateBase {
    Q_GADGET

public:
    ObjectStateBase() {}
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

    enum EnemyState {
        Invalid = 0,
        Idle,
        Move,
        WatchPlayer,
        Attack,
        Dead
    };


    enum PlayerState {
        PlayerInvalid = 0,
        PlayerIdle,
        PlayerWalk,
        PlayerRun,
        PlayerShot,
        PlayerClimbUp,
        PlayerClimbPause,
        PlayerClimbDown,
        PlayerMoveToOperate,
        PlayerOperate,
        PlayerBurn,
        PlayerFall,
        PlayerDead
    };


    enum Field {
        FieldNone = 0,

        /// Object fields
        FieldPosition = 1,
        FieldSize = 1 << 1,

        /// Entity fields
        FieldHp = 1 << 2,
        FieldMaxHp = 1 << 3,
        FieldFacingLeft = 1 << 4,
        FieldSubType = 1 << 5,

        /// Enemy fields
        FieldEnemyState = 1 << 6,
        FieldEnemyRect = 1 << 7,
        FieldMSecToAttack = 1 << 8,
        FieldWPlayerId = 1 << 9,

        /// EnemySoldier fields
        FieldTurnElapsedMSec = 1 << 10,
        FieldAttackElapsedMSec = 1 << 11,

        /// Player fields
        FieldPlayerState = 1 << 12,

        /// All
        FieldAll = FieldPosition | FieldSize |
                   FieldHp | FieldMaxHp | FieldFacingLeft | FieldSubType |
                   FieldEnemyState | FieldEnemyRect | FieldMSecToAttack | FieldWPlayerId |
                   FieldTurnElapsedMSec | FieldAttackElapsedMSec |
                   FieldPlayerState

    };

    Q_ENUM(Field);
    Q_DECLARE_FLAGS(Fields, Field)
    Q_FLAG(Fields)



    qint64 id = -1;					// object id
    ObjectType type = TypeInvalid;
    ServerState state = StateInvalid;
    qint64 tick = 0;

    Fields fields = FieldNone;

    QPointF position = {-1., -1.};		// *Bottom*Left corner
    QSizeF size = {-1., -1.};

    qint32 hp = -1;
    qint32 maxHp = -1;
    bool facingLeft = false;
    QByteArray subType;

    EnemyState enemyState = Invalid;
    QRectF enemyRect;
    double msecLeftToAttack = -1;
    int enemyWatchingPlayerId = -1;

    int turnElapsedMsec = 0;
    int attackElapsedMsec = 0;

    PlayerState playerState = PlayerInvalid;



    /**
     * @brief toDataStream
     * @param stream
     */

    void toDataStream(QDataStream &stream) const {
        stream << id;
        stream << type;
        stream << tick;
        stream << state;
        stream << fields;

        FIELD_TO_STREAM(fields, stream, FieldPosition, position);
        FIELD_TO_STREAM(fields, stream, FieldSize, size);
        FIELD_TO_STREAM(fields, stream, FieldHp, hp);
        FIELD_TO_STREAM(fields, stream, FieldMaxHp, maxHp);
        FIELD_TO_STREAM(fields, stream, FieldFacingLeft, facingLeft);
        FIELD_TO_STREAM(fields, stream, FieldSubType, subType);
        FIELD_TO_STREAM(fields, stream, FieldEnemyState, enemyState);
        FIELD_TO_STREAM(fields, stream, FieldEnemyRect, enemyRect);
        FIELD_TO_STREAM(fields, stream, FieldMSecToAttack, msecLeftToAttack);
        FIELD_TO_STREAM(fields, stream, FieldWPlayerId, enemyWatchingPlayerId);
        FIELD_TO_STREAM(fields, stream, FieldTurnElapsedMSec, turnElapsedMsec);
        FIELD_TO_STREAM(fields, stream, FieldAttackElapsedMSec, attackElapsedMsec);
        FIELD_TO_STREAM(fields, stream, FieldPlayerState, playerState);
    }


    /**
     * @brief toReadable
     * @return
     */

    QByteArray toReadable() const {
        QByteArray data;

        data.append(QStringLiteral("* id: %1\n").arg(id).toUtf8());
        data.append(QStringLiteral("* type: %1\n").arg(type).toUtf8());
        data.append(QStringLiteral("* tick: %1\n").arg(tick).toUtf8());
        data.append(QStringLiteral("* state: %1\n").arg(state).toUtf8());
        data.append(QStringLiteral("* fields: %1\n").arg(fields).toUtf8());

        if (fields.testFlag(FieldPosition)) data.append(QByteArrayLiteral("* "));
        data.append(QStringLiteral("position: %1,%2\n").arg(position.x()).arg(position.y()).toUtf8());

        if (fields.testFlag(FieldSize)) data.append(QByteArrayLiteral("* "));
        data.append(QStringLiteral("size: %1x%2\n").arg(size.width()).arg(size.height()).toUtf8());

        if (fields.testFlag(FieldHp)) data.append(QByteArrayLiteral("* "));
        data.append(QStringLiteral("hp: %1\n").arg(hp).toUtf8());

        if (fields.testFlag(FieldMaxHp)) data.append(QByteArrayLiteral("* "));
        data.append(QStringLiteral("maxHp: %1\n").arg(maxHp).toUtf8());

        if (fields.testFlag(FieldFacingLeft)) data.append(QByteArrayLiteral("* "));
        data.append(QStringLiteral("facingLeft: %1\n").arg(facingLeft).toUtf8());

        if (fields.testFlag(FieldSubType)) data.append(QByteArrayLiteral("* "));
        data.append(QStringLiteral("subType: %1\n").arg(subType).toUtf8());




        if (fields.testFlag(FieldEnemyState)) data.append(QByteArrayLiteral("* "));
        data.append(QStringLiteral("enemyState: %1\n").arg(enemyState).toUtf8());

        if (fields.testFlag(FieldEnemyRect)) data.append(QByteArrayLiteral("* "));
        data.append(QStringLiteral("enemyRect: %1x%2+%3,%4\n").arg(enemyRect.width())
                        .arg(enemyRect.height())
                        .arg(enemyRect.x())
                        .arg(enemyRect.y())
                        .toUtf8());

        if (fields.testFlag(FieldMSecToAttack)) data.append(QByteArrayLiteral("* "));
        data.append(QStringLiteral("msecLeftToAttack: %1\n").arg(msecLeftToAttack).toUtf8());

        if (fields.testFlag(FieldWPlayerId)) data.append(QByteArrayLiteral("* "));
        data.append(QStringLiteral("enemyWatchingPlayerId: %1\n").arg(enemyWatchingPlayerId).toUtf8());


        if (fields.testFlag(FieldTurnElapsedMSec)) data.append(QByteArrayLiteral("* "));
        data.append(QStringLiteral("turnElapsedMsec: %1\n").arg(turnElapsedMsec).toUtf8());

        if (fields.testFlag(FieldAttackElapsedMSec)) data.append(QByteArrayLiteral("* "));
        data.append(QStringLiteral("attackElapsedMsec: %1\n").arg(attackElapsedMsec).toUtf8());


        if (fields.testFlag(FieldPlayerState)) data.append(QByteArrayLiteral("* "));
        data.append(QStringLiteral("playerState: %1\n").arg(playerState).toUtf8());


        return data;
    };



    /**
     * @brief fromDataStream
     * @param stream
     * @param ptr
     * @return
     */

    bool fromDataStream(QDataStream &stream) {
        stream >> id;
        stream >> type;
        stream >> tick;
        stream >> state;
        stream >> fields;

        if (type == TypeInvalid) {
            LOG_CWARNING("app") << "Invalid stream exception";
            return false;
        }


        FIELD_FROM_STREAM(fields, stream, FieldPosition, position);
        FIELD_FROM_STREAM(fields, stream, FieldSize, size);
        FIELD_FROM_STREAM(fields, stream, FieldHp, hp);
        FIELD_FROM_STREAM(fields, stream, FieldMaxHp, maxHp);
        FIELD_FROM_STREAM(fields, stream, FieldFacingLeft, facingLeft);
        FIELD_FROM_STREAM(fields, stream, FieldSubType, subType);
        FIELD_FROM_STREAM(fields, stream, FieldEnemyState, enemyState);
        FIELD_FROM_STREAM(fields, stream, FieldEnemyRect, enemyRect);
        FIELD_FROM_STREAM(fields, stream, FieldMSecToAttack, msecLeftToAttack);
        FIELD_FROM_STREAM(fields, stream, FieldWPlayerId, enemyWatchingPlayerId);
        FIELD_FROM_STREAM(fields, stream, FieldTurnElapsedMSec, turnElapsedMsec);
        FIELD_FROM_STREAM(fields, stream, FieldAttackElapsedMSec, attackElapsedMsec);
        FIELD_FROM_STREAM(fields, stream, FieldPlayerState, playerState);

        return true;
    }


    /**
     * @brief diff
     * @param to
     * @return
     */

    std::optional<ObjectStateBase> diff(const ObjectStateBase &to) {
        if (to.type != type || to.id != id)
            return std::nullopt;

        ObjectStateBase d = to;
        d.fields = FieldNone;

        FIELD_DIFF(d.fields, to, FieldPosition, position);
        FIELD_DIFF(d.fields, to, FieldSize, size);
        FIELD_DIFF(d.fields, to, FieldHp, hp);
        FIELD_DIFF(d.fields, to, FieldMaxHp, maxHp);
        FIELD_DIFF(d.fields, to, FieldFacingLeft, facingLeft);
        FIELD_DIFF(d.fields, to, FieldSubType, subType);
        FIELD_DIFF(d.fields, to, FieldEnemyState, enemyState);
        FIELD_DIFF(d.fields, to, FieldEnemyRect, enemyRect);
        FIELD_DIFF(d.fields, to, FieldMSecToAttack, msecLeftToAttack);
        FIELD_DIFF(d.fields, to, FieldWPlayerId, enemyWatchingPlayerId);
        FIELD_DIFF(d.fields, to, FieldTurnElapsedMSec, turnElapsedMsec);
        FIELD_DIFF(d.fields, to, FieldAttackElapsedMSec, attackElapsedMsec);
        FIELD_DIFF(d.fields, to, FieldPlayerState, playerState);

        return d;
    };



    /**
     * @brief patch
     * @param patch
     * @return
     */

    bool patch(const ObjectStateBase &patch, const ObjectStateBase::Fields &field = ObjectStateBase::FieldAll) {
        if (patch.type != type || patch.id != id)
            return false;

        tick = patch.tick;
        state = patch.state;

        FIELD_PATCH(patch, field, FieldPosition, position);
        FIELD_PATCH(patch, field, FieldSize, size);
        FIELD_PATCH(patch, field, FieldHp, hp);
        FIELD_PATCH(patch, field, FieldMaxHp, maxHp);
        FIELD_PATCH(patch, field, FieldFacingLeft, facingLeft);
        FIELD_PATCH(patch, field, FieldSubType, subType);
        FIELD_PATCH(patch, field, FieldEnemyState, enemyState);
        FIELD_PATCH(patch, field, FieldEnemyRect, enemyRect);
        FIELD_PATCH(patch, field, FieldMSecToAttack, msecLeftToAttack);
        FIELD_PATCH(patch, field, FieldWPlayerId, enemyWatchingPlayerId);
        FIELD_PATCH(patch, field, FieldTurnElapsedMSec, turnElapsedMsec);
        FIELD_PATCH(patch, field, FieldAttackElapsedMSec, attackElapsedMsec);
        FIELD_PATCH(patch, field, FieldPlayerState, playerState);

        return true;
    }
};


Q_DECLARE_OPERATORS_FOR_FLAGS(ObjectStateBase::Fields)







/**
 * @brief The ObjectStateSnapshot class
 */

struct ObjectStateSnapshot {
    QVector<ObjectStateBase> list;


    /**
     * @brief append
     * @param ptr
     */

    void append(const ObjectStateBase &ptr)
    {
        list.append(ptr);
    }

    /**
     * @brief toByteArray
     * @return
     */

    QByteArray toByteArray() const
    {
        return toByteArray(list);
    }


    /**
     * @brief toByteArray
     * @param list
     * @return
     */

    static QByteArray toByteArray(const QList<ObjectStateBase> &list)
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
            b.toDataStream(stream);
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
            s.append(b.toReadable());
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
            ObjectStateBase b;

            if (b.fromDataStream(stream))
                snap.list.append(b);
        }

        snap.list.shrink_to_fit();

        return snap;
    }

};


#endif // OBJECTSTATE_H
