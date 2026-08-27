/*
 * ---- Call of Suli ----
 *
 * rpgnpc.h
 *
 * Created on: 2026. 07. 01.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgNpc
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

#ifndef RPGNPC_H
#define RPGNPC_H

#include "rpgentity.h"
#include "tiledeffect.h"
#include "tiledgamesfx.h"
#include <QQmlEngine>


class RpgMotorNpcControlled;

/**
 * @brief The RpgNpc class
 */

class RpgNpc : public RpgEntity
{
	Q_OBJECT
	QML_ELEMENT

	ADD_SCATTER_POINT

	Q_PROPERTY(RpgEntity *targetEntity READ targetEntity WRITE setTargetEntity NOTIFY targetEntityChanged FINAL)
	Q_PROPERTY(bool isFriend READ isFriend WRITE setIsFriend NOTIFY isFriendChanged FINAL)

public:
	RpgNpc(RpgGameItem *gameItem, const cpVect &center = cpvzero);
	virtual ~RpgNpc();

	static RpgNpc* createNpc(const Rpg::Npc &npc, RpgGameItem *gameItem, TiledScene *scene, const cpVect &pos);

	virtual void initialize() override;
	virtual void updateSprite() override;

	virtual std::unique_ptr<RpgMotorNpcControlled> getControlledMotor();

	void load(const RpgNpcDefinition &config);

	const RpgNpcDefinition &config() const { return m_config; }
	void setConfig(const RpgNpcDefinition &config);

	RpgEntity *targetEntity() const;
	void setTargetEntity(RpgEntity *newTargetEntity);

	bool isFriend() const;
	void setIsFriend(bool newIsFriend);

	QColor getColor() const;

	bool canAttack() const;
	void playSfxAttack();

signals:
	void targetEntityChanged();
	void isFriendChanged();

protected:
	void synchronize() override;
	void onAlive() override;
	void onDead() override;
	void updateColor() override;

private:
	void loadSfx();

private:
	TiledGameSfx m_sfxPain;
	TiledGameSfx m_sfxDead;
	TiledGameSfx m_sfxFootStep;
	TiledGameSfx m_sfxAttack;

	TiledEffectHealed m_effectHealed;

	RpgNpcDefinition m_config;
	RpgEntity *m_targetEntity = nullptr;

	QQuickItem *m_markerItem = nullptr;

	bool m_isFriend = false;

	friend class RpgMotorNpc;
	friend class RpgMotorNpcControlled;
};







/**
 * @brief The RpgMotorNpcEventIface class
 */

class RpgMotorNpcEventIface
{
public:
	RpgMotorNpcEventIface() = default;

	virtual void processEvent(const RpgStream::EventNpc &event) = 0;
};






/**
 * @brief The RpgMotorNpc class
 */

class RpgMotorNpc : public RpgMotorEntity, public RpgMotorNpcEventIface
{
public:
	RpgMotorNpc(RpgNpc *npc);

	virtual bool beforeWorldStep(const qint64 &tick, entt::entity &entity) override;
	virtual void updateBody(TiledObject *) override;

	virtual void processEvent(const RpgStream::EventNpc &event) override;

	static void updateBody(RpgNpc *npc, const RpgStream::NpcState &state, const bool &isEmplace);

protected:
	virtual void processEventAt(const qint64 &tick) { Q_UNUSED(tick); }

protected:
	RpgNpc *const m_npc;
	std::optional<RpgStream::NpcState> m_current;
	std::vector<RpgStream::EventNpc> m_incomingEventList;
};










/**
 * @brief The RpgMotorNpcControlled class
 */

class RpgMotorNpcControlled : public RpgDestinationMotor, public RpgMotorNpcEventIface
{
public:
	RpgMotorNpcControlled(RpgNpc *npc);

	virtual void updateBody(TiledObject *) override;
	virtual bool beforeWorldStep(const qint64 &tick, entt::entity &entity) override;
	virtual bool afterWorldStep(const qint64 &tick, RpgStream::FullState *state) override;

	const RpgStream::NpcState *saveCurrentState(const qint64 &tick);

	virtual void processEvent(const RpgStream::EventNpc &event) override;

protected:
	virtual void onShapeContactBegin(cpShape *self, cpShape *other) override;
	virtual void onShapeContactEnd(cpShape *self, cpShape *other) override;

	virtual void updateTarget();
	virtual int getMovementSpeed();
	virtual void updateMovement(const float &speed);
	virtual void updateMotor();
	virtual void onGroundCollision();

	virtual void saveState(RpgStream::NpcState &dest) { Q_UNUSED(dest); }

	virtual void processEventAt(const qint64 &tick) { Q_UNUSED(tick); }

	void applyKnockback();

protected:
	RpgNpc *const m_npc;
	Rpg::RpgNpcStatePull m_statePull;

	std::vector<RpgStream::EventNpc> m_incomingEventList;
	std::vector<RpgStream::EventNpc> m_eventList;

	cpVect m_currentKnockback = cpvzero;
	QSet<cpShape*> m_groundCollision;
	int m_groundCollisionCounter = 0;

	qint64 m_currentTick = 0;

	friend class RpgNpc;
};





#endif // RPGNPC_H
