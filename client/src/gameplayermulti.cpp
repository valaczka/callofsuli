#include "gameplayermulti.h"


// State hash

const QHash<ObjectStateBase::PlayerState, GamePlayer::PlayerState> GamePlayerMulti::m_stateHash = {
    { ObjectStateBase::PlayerIdle, GamePlayer::Idle },
    { ObjectStateBase::PlayerWalk, GamePlayer::Walk },
    { ObjectStateBase::PlayerRun, GamePlayer::Run },
    { ObjectStateBase::PlayerShot, GamePlayer::Shot },
    { ObjectStateBase::PlayerClimbUp, GamePlayer::ClimbUp },
    { ObjectStateBase::PlayerClimbPause, GamePlayer::ClimbPause },
    { ObjectStateBase::PlayerClimbDown, GamePlayer::ClimbDown },
    { ObjectStateBase::PlayerMoveToOperate, GamePlayer::MoveToOperate },
    { ObjectStateBase::PlayerOperate, GamePlayer::Operate },
    { ObjectStateBase::PlayerBurn, GamePlayer::Burn },
    { ObjectStateBase::PlayerDead, GamePlayer::Dead },
    { ObjectStateBase::PlayerFall, GamePlayer::Fall },
    };


/**
 * @brief GamePlayerMulti::GamePlayerMulti
 * @param parent
 */

GamePlayerMulti::GamePlayerMulti(QQuickItem *parent)
    : GamePlayer(parent)
{

}



/**
 * @brief GamePlayerMulti::createState
 * @return
 */

ObjectStateBase GamePlayerMulti::createState(const QPointF &pos)
{
    ObjectStateBase state;

    state.type = ObjectStateBase::TypePlayer;

    state.fields = ObjectStateBase::FieldHp|ObjectStateBase::FieldMaxHp|ObjectStateBase::FieldPlayerState|
                   ObjectStateBase::FieldFacingLeft|ObjectStateBase::FieldPosition;

    state.hp = 1;
    state.maxHp = 1;
    state.playerState = ObjectStateBase::PlayerIdle;
    state.facingLeft = false;//QRandomGenerator::global()->generate() % 2;
    state.position = pos;

    return state;
}



/**
 * @brief GamePlayerMulti::onTimingTimerTimeoutMulti
 * @param hosted
 * @param msec
 * @param delayFactor
 */

void GamePlayerMulti::onTimingTimerTimeoutMulti(const bool &hosted, const int &msec, const qreal &delayFactor)
{
    if (hosted) {
        onTimingTimerTimeout(msec, delayFactor);
    } else {
        if (m_playerState == Shot) {
            if (m_lastCurrentSprite == QStringLiteral("shot"))
                return;
            else
                jumpToSprite(QStringLiteral("idle"));
        } else if (m_playerState == Idle && m_lastCurrentSprite != QStringLiteral("idle")) {
            jumpToSprite(QStringLiteral("idle"));
        }
    }
}



/**
 * @brief GamePlayerMulti::cacheCurrentState
 */

void GamePlayerMulti::cacheCurrentState()
{
    ActionGame *_game = game();

    if (!_game)
        return;

    if (!m_cachedStates.empty()) {
        ObjectStateBase &state = m_cachedStates.back();

        PlayerState p = m_ladderFall ? PlayerState::Idle : m_playerState;

        if (m_stateHash.value(state.playerState, Invalid) == p &&
            state.facingLeft == m_facingLeft &&
            state.hp == m_hp &&
            state.maxHp == m_maxHp) {
            state = getCurrentState();
            state.tick = _game->currentTick();
            return;
        }
    }

    ObjectStateBase state = getCurrentState();
    state.tick = _game->currentTick();

    m_cachedStates.push_back(state);
}



/**
 * @brief GamePlayerMulti::setStateFromSnapshot
 * @param ptr
 * @param currentTick
 * @param force
 */

void GamePlayerMulti::setStateFromSnapshot(const ObjectStateBase &ptr, const qint64 &currentTick, const bool &force)
{
    //LOG_CINFO("game")  << "SET STATE FROM" << ptr.toReadable().constData();

    if (ptr.type != ObjectStateBase::TypePlayer) {
        LOG_CERROR("game") << "Invalid ObjectState" << ptr.id << ptr.type;
        return;
    }

    if (!force) {
        const auto &s = GameObject::stateReconciliation(ptr);

        if (s.has_value()) {
            const ObjectStateBase &curr = getCurrentState();
            if (!stateReconciliation(*s, curr)) {
                setCurrentState(*s, true);
            }
        } else {
            setCurrentState(ptr, true);
        }
    } else {
        removeOldAuthoritativeStates(currentTick);
        m_authoritativeStates.insert({ptr.tick, ptr});

        LOG_CDEBUG("game")  << "APPEND+++";
        LOG_CTRACE("game") << "AUTH STATES.........................................";

        for (const auto &[tick, s] : m_authoritativeStates)  {
            LOG_CINFO("game")  << "   #" << tick << s.position << s.playerState;
        }

        LOG_CTRACE("game") << ".........................................";

        interpolateState(currentTick);
    }
}



/**
 * @brief GamePlayerMulti::getCurrentState
 * @return
 */

ObjectStateBase GamePlayerMulti::getCurrentState() const
{
    ObjectStateBase b = GameEntity::getCurrentState();
    b.type = ObjectStateBase::TypePlayer;

    b.fields.setFlag(ObjectStateBase::FieldPlayerState);

    if (m_ladderFall)
        b.playerState = ObjectStateBase::PlayerIdle;
    else
        b.playerState = m_stateHash.key(m_playerState, ObjectStateBase::PlayerInvalid);

    /*b.fields.setFlag(ObjectStateBase::FieldTurnElapsedMSec);
    b.fields.setFlag(ObjectStateBase::FieldAttackElapsedMSec);

    b.turnElapsedMsec = m_turnElapsedMsec;
    b.attackElapsedMsec = m_attackElapsedMsec;*/

    return b;
}




/**
 * @brief GamePlayerMulti::setCurrentState
 * @param state
 * @param force
 */

void GamePlayerMulti::setCurrentState(const ObjectStateBase &state, const bool &force)
{
    GameEntity::setCurrentState(state, force);

    if (force) {
        if (state.fields.testFlag(ObjectStateBase::FieldPlayerState)) {
            setPlayerState(m_stateHash.value(state.playerState, Invalid));
        }

        /*if (state.fields.testFlag(ObjectStateBase::FieldTurnElapsedMSec))
            setTurnElapsedMsec(state.turnElapsedMsec);

        if (state.fields.testFlag(ObjectStateBase::FieldAttackElapsedMSec))
            m_attackElapsedMsec = state.attackElapsedMsec;*/
    }
}



/**
 * @brief GamePlayerMulti::init
 */

void GamePlayerMulti::init(ActionGame *game)
{
    if (!m_isSelf) {
        setCategoryFixture(CATEGORY_PLAYER_OTHER);
        setCategoryRayCast(Box2DFixture::None);
        setCategoryCollidesWith(Box2DFixture::None);
        setRayCastEnabled(false);
    } else {
        setCategoryFixture(CATEGORY_PLAYER);
        setCategoryRayCast(CATEGORY_ENEMY);
        setCategoryCollidesWith(CATEGORY_GROUND|CATEGORY_ITEM|CATEGORY_OTHER);
        setRayCastEnabled(true);


        connect(this, &GameObject::sceneConnected, this, &GamePlayer::onSceneConnected);
        connect(this, &GameEntity::beginContact, this, &GamePlayer::onBeginContact);
        connect(this, &GameEntity::endContact, this, &GamePlayer::onEndContact);
        connect(this, &GameEntity::baseGroundContact, this, &GamePlayer::onBaseGroundContacted);
        connect(this, &GamePlayer::attack, this, &GamePlayer::playAttackSound);


        connect(this, &GamePlayer::movingFlagsChanged, this, &GamePlayer::onMovingFlagsChanged);

        connect(this, &GamePlayer::hurt, this, [this, game]() {
            game->resetKillStreak();
            QTimer::singleShot(450, this, [this](){ playSoundEffect(QStringLiteral("pain")); });
        });
        connect(this, &GamePlayer::allHpLost, this, [this](){
            setPlayerState(Dead);
            m_scene->playSoundPlayerVoice(QStringLiteral("qrc:/sound/sfx/dead.mp3"));
            emit killed(this);
        });

        connect(this, &GamePlayer::invisibleChanged, this, [this](){
            if (!m_invisible)
                m_scene->playSound("qrc:/sound/sfx/question.mp3");
        });


        connect(this, &GameEntity::isAliveChanged, this, &GamePlayer::onIsAliveChanged);

        connect(this, &GamePlayer::died, game, &ActionGame::onPlayerDied);
        connect(game, &ActionGame::runningChanged, this, &GamePlayer::onMovingFlagsChanged);
    }

}





/**
 * @brief GamePlayerMulti::interpolate
 * @param t
 * @param from
 * @param to
 * @return
 */

ObjectStateBase GamePlayerMulti::interpolate(const qreal &t, const ObjectStateBase &from, const ObjectStateBase &to)
{
    ObjectStateBase b = GameEntity::interpolate(t, from, to);

    if (from.fields.testFlag(ObjectStateBase::FieldPlayerState)) {
        if (t < 1.0)
            b.playerState = from.playerState;
        else
            b.playerState = to.playerState;

        if (m_playerState != ClimbDown && m_playerState != ClimbUp && m_playerState != ClimbPause) {
            if (t < 1.0) {
                if (from.playerState == ObjectStateBase::PlayerClimbUp)
                    jumpToSprite(QStringLiteral("climbup"));
                else if (from.playerState == ObjectStateBase::PlayerClimbDown)
                    jumpToSprite(QStringLiteral("climbdown"));
            } else {
                if (to.playerState == ObjectStateBase::PlayerClimbUp)
                    jumpToSprite(QStringLiteral("climbup"));
                else if (to.playerState == ObjectStateBase::PlayerClimbDown)
                    jumpToSprite(QStringLiteral("climbdown"));
            }
        } else if (m_playerState == ClimbPause && b.playerState == ObjectStateBase::PlayerClimbUp) {
            if (!QStringList({QStringLiteral("climbup"), QStringLiteral("climbup2"), QStringLiteral("climbup3")}).contains(m_lastCurrentSprite))
                jumpToSprite(QStringLiteral("climbup2"));
        } else if (m_playerState == ClimbPause && b.playerState == ObjectStateBase::PlayerClimbDown) {
            if (!QStringList({QStringLiteral("climbdown"), QStringLiteral("climbdown2"), QStringLiteral("climbdown3")}).contains(m_lastCurrentSprite))
                jumpToSprite(QStringLiteral("climbdown2"));
        } else if (b.playerState == ObjectStateBase::PlayerClimbPause) {
            jumpToSprite(QStringLiteral("climbpause"));
        }
    }

    return b;
}



/**
 * @brief GamePlayerMulti::performAttack
 */

void GamePlayerMulti::performAttack()
{
    setPlayerState(Shot);
    emit attack();
    jumpToSprite(QStringLiteral("shot"));		// Mindenképp kérjük

    cacheCurrentState();

    if (!m_enemy)
        return;

    ///m_scene->game()->tryAttack(this, m_enemy);
}



/**
 * @brief GamePlayerMulti::isSelf
 * @return
 */

bool GamePlayerMulti::isSelf() const
{
    return m_isSelf;
}

void GamePlayerMulti::setIsSelf(bool newIsSelf)
{
    if (m_isSelf == newIsSelf)
        return;
    m_isSelf = newIsSelf;
    emit isSelfChanged();
}


