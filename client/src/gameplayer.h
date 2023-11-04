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
	Q_PROPERTY(MovingFlags movingFlags READ movingFlags WRITE setMovingFlags NOTIFY movingFlagsChanged)
	Q_PROPERTY(qreal deathlyFall READ deathlyFall WRITE setDeathlyFall NOTIFY deathlyFallChanged)
	Q_PROPERTY(qreal hurtFall READ hurtFall WRITE setHurtFall NOTIFY hurtFallChanged)
	Q_PROPERTY(int shield READ shield WRITE setShield NOTIFY shieldChanged)
	Q_PROPERTY(bool invisible READ invisible WRITE setInvisible NOTIFY invisibleChanged)
	Q_PROPERTY(GameObject* operatingObject READ operatingObject WRITE setOperatingObject NOTIFY operatingObjectChanged)
	Q_PROPERTY(int invisibleTime READ invisibleTime NOTIFY invisibleTimeChanged)


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
		MoveToOperate,
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

	static GamePlayer* create(GameScene *scene, const QString &type = "");

	Q_INVOKABLE void setMovingFlag(const GamePlayer::MovingFlag &flag, const bool &on = true);
	Q_INVOKABLE void standbyMovingFlags();
	Q_INVOKABLE void moveTo(const QPointF &point, const bool &forced = false);
	Q_INVOKABLE void moveTo(const qreal &x, const qreal &y, const bool &forced = false) { moveTo(QPointF(x, y), forced); }
	Q_INVOKABLE void shot();
	Q_INVOKABLE void turnLeft();
	Q_INVOKABLE void turnRight();
	Q_INVOKABLE void operate(GameObject *object);
	Q_INVOKABLE void startInvisibility(const int &msec);

	Q_INVOKABLE void playSoundEffect(const QString &effect, int from = -1);

	GameEnemy *enemy() const;
	void setEnemy(GameEnemy *newEnemy);

	const PlayerState &playerState() const;
	void setPlayerState(const PlayerState &newPlayerState);

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
	Q_INVOKABLE GameObject *terrainObject(const QString &type) const;

	int shield() const;
	void setShield(int newShield);

	bool invisible() const;
	void setInvisible(bool newInvisible);

	GameObject *operatingObject() const;
	void setOperatingObject(GameObject *newOperatingObject);

	void setInvisibleTime(const int &msec);
	const int &invisibleTime() const;

	void onTimingTimerTimeout(const int &msec, const qreal &delayFactor) override;

public slots:
	void hurtByEnemy(GameEnemy *, const bool &canProtect = false);
	void killByEnemy(GameEnemy *);

	void setTerrainObject(const QString &type, GameObject *object);
	void onMovingFlagsChanged();

protected:
	virtual void rayCastReport(const QMultiMap<qreal, GameEntity *> &items) override;
	virtual void hpProgressValueSetup() override;

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
	void shieldChanged();
	void invisibleChanged();
	void operatingObjectChanged();
	void invisibleTimeChanged();

private slots:
	void onEnemyKilled();
	void onSceneConnected();
	void onBeginContact(Box2DFixture *other);
	void onEndContact(Box2DFixture *other);
	void onBaseGroundContacted();
	void onIsAliveChanged();
	void onHpOrShieldChanged();

private:
	void ladderMove(const bool &up, const qreal &delayFactor);

#if !defined(Q_OS_WASM) && QT_VERSION < 0x060000
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
	qreal m_runSize = 0;
	qreal m_climbSize = 0;
	QHash<QString, QStringList> m_soundEffectHash;
	QHash<QString, int> m_soundEffectNum;
	LadderState m_ladderState = LadderInactive;
	bool m_ladderFall = false;
	int m_soundElapsedMsec = 0;
	QHash<QString, QPointer<GameObject>> m_terrainObjects;
	int m_shield = 0;
	bool m_invisible = false;
	QPointer<GameObject> m_operatingObject = nullptr;
	int m_invisibleTime = 0;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(GamePlayer::MovingFlags)


#endif // GAMEPLAYER_H
