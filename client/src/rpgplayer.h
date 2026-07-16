/*
 * ---- Call of Suli ----
 *
 * rpgplayer.h
 *
 * Created on: 2026. 05. 12.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgPlayer
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

#ifndef RPGPLAYER_H
#define RPGPLAYER_H

#include "rpggameitem.h"
#include "rpgentity.h"
#include "rpgmp.h"
#include "tiledeffect.h"
#include "tiledgamesfx.h"
#include <QQmlEngine>


class RpgPlayerPrivate;

/**
 * @brief The RpgPlayer class
 */

class RpgPlayer : public RpgEntity
{
	Q_OBJECT
	QML_ELEMENT

	ADD_SCATTER_POINT

	Q_PROPERTY(QPoint currentChunk READ currentChunk NOTIFY currentChunkChanged FINAL)
	Q_PROPERTY(QPointF currentChunkCenter READ currentChunkCenter NOTIFY currentChunkCenterChanged FINAL)

	Q_PROPERTY(int mp READ mp WRITE setMp NOTIFY mpChanged FINAL)
	Q_PROPERTY(int maxMp READ maxMp NOTIFY maxMpChanged FINAL)

	Q_PROPERTY(int bullet READ bullet WRITE setBullet NOTIFY bulletChanged FINAL)
	Q_PROPERTY(int maxBullet READ maxBullet NOTIFY maxBulletChanged FINAL)

	Q_PROPERTY(bool hasDefender READ hasDefender NOTIFY hasDefenderChanged FINAL)

	Q_PROPERTY(bool hasUtility READ hasUtility NOTIFY hasUtilityChanged FINAL)
	Q_PROPERTY(bool canUseUtility READ canUseUtility WRITE setCanUseUtility NOTIFY canUseUtilityChanged FINAL)

	Q_PROPERTY(RpgEntity *targetEntity READ targetEntity WRITE setTargetEntity NOTIFY targetEntityChanged FINAL)
	Q_PROPERTY(RpgEntity *utilityEntity READ utilityEntity WRITE setUtilityEntity NOTIFY utilityEntityChanged FINAL)

public:
	RpgPlayer(RpgGameItem *gameItem, const cpVect &center = cpvzero);
	virtual ~RpgPlayer();

	virtual void initialize() override;
	virtual void updateSprite() override;

	void load(const RpgPlayerDefinition &config);

	const RpgPlayerDefinition &config() const;
	void setConfig(const RpgPlayerDefinition &config);

	Q_INVOKABLE bool isRunning() const;
	Q_INVOKABLE bool isWalking() const;

	Q_INVOKABLE void useCurrentUtility();

	QPoint currentChunk() const;
	void setCurrentChunk(QPoint newCurrentChunk);

	QPointF currentChunkCenter() const;
	void setCurrentChunkCenter(QPointF newCurrentChunkCenter);

	int mp() const;
	void setMp(int newMp);

	int maxMp() const;
	int maxBullet() const;

	bool hasDefender() const;
	void setDefender(const RpgStream::BaseDefenderObject::Type &type, const bool &hasDefender);
	const RpgStream::BaseDefenderObject::Type &currentDefender() const { return m_defender; }

	RpgEntity *targetEntity() const;
	void setTargetEntity(RpgEntity *newTargetEntity);

	TiledObjectBody *targetControl() const;
	void setTargetControl(TiledObjectBody *newTargetControl);

	int bullet() const;
	void setBullet(int newBullet);

	void setUtility(const RpgStream::PlayerConfig::Utility &type, const bool &hasUtility);
	const RpgStream::PlayerConfig::Utility &currentUtility() const { return m_utility; }
	bool hasUtility() const;

	bool canUseUtility() const;
	void setCanUseUtility(bool newCanUseUtility);

	RpgEntity *utilityEntity() const;
	void setUtilityEntity(RpgEntity *newUtilityEntity);


signals:
	void currentChunkChanged();
	void currentChunkCenterChanged();
	void mpChanged();
	void maxMpChanged();
	void targetControlChanged();
	void targetEntityChanged();
	void bulletChanged();
	void maxBulletChanged();
	void hasDefenderChanged();
	void hasUtilityChanged();
	void canUseUtilityChanged();
	void utilityEntityChanged();

protected:
	void synchronize() override;
	void onAlive() override;
	void onDead() override;
	void updateColor() override;

private:
	void loadSfx();
	void onCurrentSpriteChanged();

private:
	RpgPlayerPrivate *d = nullptr;
	QPoint m_currentChunk;
	QPointF m_currentChunkCenter;
	int m_mp = 0;

	int m_bullet = 0;

	RpgStream::BaseDefenderObject::Type m_defender = RpgStream::BaseDefenderObject::None;
	bool m_hasDefender = false;

	RpgStream::PlayerConfig::Utility m_utility = RpgStream::PlayerConfig::UtilityNone;
	bool m_hasUtility = false;
	bool m_canUseUtility = false;

	TiledGameSfx m_sfxPain;
	TiledGameSfx m_sfxDead;
	TiledGameSfx m_sfxFootStep;
	TiledGameSfx m_sfxAccept;
	TiledGameSfx m_sfxDecline;

	TiledEffectHealed m_effectHealed;
	TiledEffectShield m_effectShield;
	TiledEffectRing m_effectRing;

	RpgPlayerDefinition m_config;

	QQuickItem *m_markerItem = nullptr;
	RpgEntity *m_targetEntity = nullptr;
	TiledObjectBody *m_targetControl = nullptr;


	friend class RpgPlayerPrivate;
	friend class RpgMotorPlayer;
	friend class RpgMotorPlayerControlled;

	RpgEntity *m_utilityEntity = nullptr;
};





/**
 * @brief The RpgMotorPlayerEventIface class
 */

class RpgMotorPlayerEventIface
{
public:
	RpgMotorPlayerEventIface() = default;

	virtual void processEvent(const RpgStream::EventPlayer &event) = 0;
};




/**
 * @brief The RpgMotorPlayer class - not controlled
 */

class RpgMotorPlayer : public RpgMotorEntity, public RpgMotorPlayerEventIface
{
public:
	RpgMotorPlayer(RpgPlayer *player);

	virtual bool beforeWorldStep(const qint64 &tick, entt::entity &entity) override;
	virtual void updateBody(TiledObject *) override;

	virtual void processEvent(const RpgStream::EventPlayer &event) override;

	static void updateBody(RpgPlayer *player, const RpgStream::PlayerState &state, const bool &isEmplace);
	static void onAttack(RpgPlayer *player);
	static void onUseUtility(RpgPlayer *player);

protected:
	virtual void processEventAt(const qint64 &tick);

	RpgPlayer *const m_player;
	std::optional<RpgStream::PlayerState> m_current;
	std::vector<RpgStream::EventPlayer> m_incomingEventList;
};





/**
 * @brief The RpgMotorPlayer class
 */

class RpgMotorPlayerControlled : public RpgDestinationMotor, public RpgMotorPlayerEventIface
{
public:
	RpgMotorPlayerControlled(RpgPlayer *player);

	virtual void updateBody(TiledObject *) override;
	virtual bool beforeWorldStep(const qint64 &tick, entt::entity &entity) override;
	virtual bool afterWorldStep(const qint64 &tick, RpgStream::FullState *state) override;

	const RpgStream::PlayerState *saveCurrentState(const qint64 &tick);

	TiledGame::JoystickState currentJoystickState() const;
	void setCurrentJoystickState(const TiledGame::JoystickState &newCurrentJoystickState);

	TiledGame::JoystickState controlJoystickState() const;
	void setControlJoystickState(const TiledGame::JoystickState &newControlJoystickState);

	TiledGame::JoystickState targetJoystickState() const;
	void setTargetJoystickState(const TiledGame::JoystickState &newTargetJoystickState);

	void questionFinished(const bool &success);

	void attackCurrentTarget();
	void useCurrentControl();
	void putDefender(const bool &click);

	void useCurrentUtility();
	void updateUseUtility();
	float utilityRequireTarget(cpBitmask *categoryPtr = nullptr) const;

	void changeMpToBullet();
	void changeMpToDefender();
	void changeMpToUtility();

	void replaceDefender(const RpgStream::BaseDefenderObject::Type &type);
	void replaceUtility(const RpgStream::PlayerConfig::Utility &type);

	virtual void processEvent(const RpgStream::EventPlayer &event) override;

	const std::optional<cpVect> &targetAhead() const;

protected:
	virtual void onShapeContactBegin(cpShape *self, cpShape *other) override;
	virtual void onShapeContactEnd(cpShape *self, cpShape *other) override;

	virtual void processEventAt(const qint64 &tick) { Q_UNUSED(tick); }

	void eventMpPick(RpgMp *mp);


protected:
	RpgPlayer *const m_player;
	RpgPlayerPrivate *const d;
	Rpg::RpgPlayerStatePull m_statePull;

	std::vector<RpgStream::EventPlayer> m_incomingEventList;

	TiledGame::JoystickState m_currentJoystickState;
	TiledGame::JoystickState m_controlJoystickState;
	TiledGame::JoystickState m_targetJoystickState;


private:
	TiledObjectBody* findNearestControl(const float &maxDist,
										const cpBitmask &category = RpgGameItem::FixtureControl | RpgGameItem::FixtureDefender);
	TiledObjectBody* findNearestControl(const cpVect &rayDest,
										const cpBitmask &category = RpgGameItem::FixtureControl | RpgGameItem::FixtureDefender);
	bool checkControl(TiledObjectBody *control) const;

	std::optional<float> m_targetAngle;
	std::optional<cpVect> m_targetAhead;

	friend class RpgPlayerPrivate;
};




#endif // RPGPLAYER_H
