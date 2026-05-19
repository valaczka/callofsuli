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

#include "rpgentity.h"
#include "rpgmp.h"
#include "rpgtower.h"
#include "tiledeffect.h"
#include "tiledgamesfx.h"
#include <QQmlEngine>


/**
 * @brief The RpgPlayer class
 */

class RpgPlayer : public RpgEntity
{
	Q_OBJECT
	QML_ELEMENT

	Q_PROPERTY(float chunkRadius READ chunkRadius WRITE setChunkRadius NOTIFY chunkRadiusChanged FINAL)
	Q_PROPERTY(QPoint currentChunk READ currentChunk NOTIFY currentChunkChanged FINAL)
	Q_PROPERTY(QPointF currentChunkCenter READ currentChunkCenter NOTIFY currentChunkCenterChanged FINAL)

	Q_PROPERTY(int mp READ mp WRITE setMp NOTIFY mpChanged FINAL)
	Q_PROPERTY(int maxMp READ maxMp NOTIFY maxMpChanged FINAL)

	Q_PROPERTY(RpgTower* tower READ tower WRITE setTower NOTIFY towerChanged FINAL)

public:
	RpgPlayer(RpgGameItem *gameItem, const cpVect &center = cpvzero);

	virtual void initialize() override;
	virtual void updateSprite() override;

	void load(const RpgPlayerDefinition &config);

	void setConfig(const RpgPlayerDefinition &config);

	Q_INVOKABLE bool isRunning() const;
	Q_INVOKABLE bool isWalking() const;


	QPoint currentChunk() const;
	void setCurrentChunk(QPoint newCurrentChunk);

	float chunkRadius() const;
	void setChunkRadius(float newChunkRadius);

	QPointF currentChunkCenter() const;
	void setCurrentChunkCenter(QPointF newCurrentChunkCenter);

	int mp() const;
	void setMp(int newMp);

	int maxMp() const;

	RpgStream::Team team() const;
	void setTeam(RpgStream::Team newTeam);

	RpgTower *tower() const;
	void setTower(RpgTower *newTower);

signals:
	void currentChunkChanged();
	void chunkRadiusChanged();
	void currentChunkCenterChanged();
	void mpChanged();
	void maxMpChanged();
	void towerChanged();

protected:
	void onAlive() override;
	void onDead() override;

private:
	void loadSfx();
	void onCurrentSpriteChanged();
	void updateColor();

private:
	float m_chunkRadius = 0.;
	QPoint m_currentChunk;
	QPointF m_currentChunkCenter;
	int m_mp = 0;
	RpgStream::Team m_team = RpgStream::TeamNone;

	TiledGameSfx m_sfxPain;
	TiledGameSfx m_sfxFootStep;
	TiledGameSfx m_sfxAccept;
	TiledGameSfx m_sfxDecline;

	TiledEffectHealed m_effectHealed;
	TiledEffectShield m_effectShield;
	TiledEffectRing m_effectRing;

	RpgPlayerDefinition m_config;

	QQuickItem *m_markerItem = nullptr;
	RpgTower *m_tower = nullptr;

	friend class RpgMotorPlayer;
	friend class RpgMotorPlayerControlled;
};





/**
 * @brief The RpgMotorPlayer class - not controlled
 */

class RpgMotorPlayer : public RpgMotorEntity
{
public:
	RpgMotorPlayer(RpgPlayer *player);

	virtual bool beforeWorldStep(const qint64 &tick, entt::entity &entity) override;
	virtual void updateBody(TiledObject *) override;

protected:
	RpgPlayer *const m_player;
	std::optional<RpgStream::PlayerState> m_current;
};





/**
 * @brief The RpgMotorPlayer class
 */

class RpgMotorPlayerControlled : public RpgDestinationMotor
{
public:
	RpgMotorPlayerControlled(RpgPlayer *player);

	virtual void updateBody(TiledObject *) override;
	virtual bool beforeWorldStep(const qint64 &tick, entt::entity &entity) override;
	virtual bool afterWorldStep(const qint64 &tick, RpgStream::FullState *state) override;

	TiledGame::JoystickState currentJoystickState() const;
	void setCurrentJoystickState(const TiledGame::JoystickState &newCurrentJoystickState);

	void eventTest();
	void useCurrentControl();

protected:
	virtual void onShapeContactBegin(cpShape *self, cpShape *other) override;
	virtual void onShapeContactEnd(cpShape *self, cpShape *other) override;

	void eventMpPick(RpgMp *mp);

protected:
	RpgPlayer *const m_player;
	Rpg::RpgPlayerStatePull m_statePull;

	TiledGame::JoystickState m_currentJoystickState;
	std::vector<RpgStream::EventPlayer> m_eventList;

};




#endif // RPGPLAYER_H
