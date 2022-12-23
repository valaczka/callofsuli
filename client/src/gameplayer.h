/*
 * ---- Call of Suli ----
 *
 * gameplayer.h
 *
 * Created on: 2022. 12. 20.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GamePlayer
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

#ifndef GAMEPLAYER_H
#define GAMEPLAYER_H

#include "gameenemy.h"
#include "gameentity.h"
#include "gameladder.h"

#ifndef Q_OS_WASM
#include <QSoundEffect>
#endif

class GamePlayer : public GameEntity
{
	Q_OBJECT

	Q_PROPERTY(GameEnemy* enemy READ enemy WRITE setEnemy NOTIFY enemyChanged)
	Q_PROPERTY(GameLadder* ladder READ ladder WRITE setLadder NOTIFY ladderChanged)
	Q_PROPERTY(LadderState ladderState READ ladderState NOTIFY ladderStateChanged)
	Q_PROPERTY(PlayerState playerState READ playerState NOTIFY playerStateChanged)
	Q_PROPERTY(qreal runSize READ runSize CONSTANT)
	Q_PROPERTY(MovingFlags movingFlags READ movingFlags WRITE setMovingFlags NOTIFY movingFlagsChanged)
	Q_PROPERTY(qreal deathlyFall READ deathlyFall WRITE setDeathlyFall NOTIFY deathlyFallChanged)
	Q_PROPERTY(qreal hurtFall READ hurtFall WRITE setHurtFall NOTIFY hurtFallChanged)

#ifndef Q_OS_WASM
	Q_PROPERTY(QSoundEffect *soundEffectShot READ soundEffectShot CONSTANT)
	Q_PROPERTY(QSoundEffect *soundEffectGeneral READ soundEffectGeneral CONSTANT)
#endif

public:
	explicit GamePlayer(QQuickItem *parent = nullptr);
	virtual ~GamePlayer();


	// Sprite states

	enum PlayerState {
		Invalid = 0,
		Idle,
		Walk,
		Run,
		Shot,
		ClimbUp,
		ClimbPause,
		ClimbDown,
		Operate,
		Burn,
		Fall,
		Dead
	};

	Q_ENUM(PlayerState);


	// Moving flags

	enum MovingFlag {
		Standby = 0,
		SlowModifier = 1,
		MoveLeft = 1 << 2,
		MoveRight = 1 << 3,
		MoveUp = 1 << 4,
		MoveDown = 1 << 5,
		JoystickInteraction = 1 << 10
	};


	Q_ENUM(MovingFlag)
	Q_DECLARE_FLAGS(MovingFlags, MovingFlag)
	Q_FLAG(MovingFlags)


	// Ladder state

	enum LadderState {
		LadderInactive,
		LadderUpAvailable,
		LadderDownAvailable,
		LadderActive,
		LadderTopSprite
	};

	Q_ENUM(LadderState)

#ifndef Q_OS_WASM
	QSoundEffect *soundEffectShot() const;
	QSoundEffect *soundEffectGeneral() const;
#endif

	static GamePlayer* create(GameScene *scene, const QString &type = "");

	Q_INVOKABLE void setMovingFlag(const MovingFlag &flag, const bool &on = true);
	Q_INVOKABLE void standbyMovingFlags();
	Q_INVOKABLE void moveTo(const QPointF &point, const bool &forced = false);
	Q_INVOKABLE void moveTo(const qreal &x, const qreal &y, const bool &forced = false) { moveTo(QPointF(x, y), forced); }
	Q_INVOKABLE void shot();
	Q_INVOKABLE void turnLeft();
	Q_INVOKABLE void turnRight();

	Q_INVOKABLE void playSoundEffect(const QString &effect, int from = -1);

	GameEnemy *enemy() const;
	void setEnemy(GameEnemy *newEnemy);

	const PlayerState &playerState() const;
	void setPlayerState(const PlayerState &newPlayerState);

	qreal runSize() const;
	qreal climbSize() const;

	const MovingFlags &movingFlags() const;
	void setMovingFlags(const MovingFlags &newMovingFlags);

	qreal deathlyFall() const;
	void setDeathlyFall(qreal newDeathlyFall);

	qreal hurtFall() const;
	void setHurtFall(qreal newHurtFall);

	GameLadder *ladder() const;
	void setLadder(GameLadder *newLadder);

	LadderState ladderState() const;
	void setLadderState(LadderState newLadderState);

	const QHash<QString, QPointer<GameObject> > &terrainObjects() const;
	GameObject *terrainObject(const QString &type) const;

public slots:
	void hurtByEnemy(GameEnemy *enemy, const bool &canProtect = false);
	void killByEnemy(GameEnemy *enemy);

	void setTerrainObject(const QString &type, GameObject *object);

protected:
	virtual void rayCastReport(const QMultiMap<qreal, GameEntity *> &items) override;


signals:
	void attack();
	void underAttack();

	void enemyChanged();
	void playerStateChanged();
	void movingFlagsChanged(GamePlayer::MovingFlags flags);
	void deathlyFallChanged();
	void hurtFallChanged();
	void ladderChanged();
	void ladderStateChanged();
	void terrainObjectChanged(const QString &type, GameObject *object);

private slots:
	void onEnemyKilled();
	void onMovingFlagsChanged();
	void onIsOnGroundChanged();
	void onSceneConnected();
	void onTimingTimerTimeout();
	void onBeginContact(Box2DFixture *other);
	void onEndContact(Box2DFixture *other);
	void onBaseGroundContacted();
	void onIsAliveChanged();

private:
	void ladderMove(const bool &up);
	void terrainObjectSignalAddToPool(const QString &type, GameObject *object);
	void terrainObjectSignalEmit();

#ifndef Q_OS_WASM
	QSoundEffect *m_soundEffectShot = nullptr;
	QSoundEffect *m_soundEffectGeneral = nullptr;
#endif

	QPointer<GameEnemy> m_enemy = nullptr;
	QPointer<GameLadder> m_ladder = nullptr;
	PlayerState m_playerState = Invalid;
	MovingFlags m_movingFlags = Standby;
	qreal m_fallStart = -1;
	qreal m_deathlyFall = 500;
	qreal m_hurtFall = 200;
	QHash<QString, QStringList> m_soundEffectHash;
	QHash<QString, int> m_soundEffectNum;
	LadderState m_ladderState = LadderInactive;
	bool m_ladderFall = false;
	int m_soundElapsedMsec = 0;
	QHash<QString, QPointer<GameObject>> m_terrainObjects;
	QHash<QString, QPointer<GameObject>> m_terrainObjectsPrevious;

};

Q_DECLARE_OPERATORS_FOR_FLAGS(GamePlayer::MovingFlags)


#endif // GAMEPLAYER_H
