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
#include "tiledgamesfx.h"
#include <QQmlEngine>


/**
 * @brief The RpgNpc class
 */

class RpgNpc : public RpgEntity
{
	Q_OBJECT
	QML_ELEMENT

	ADD_SCATTER_POINT

	Q_PROPERTY(RpgEntity *targetEntity READ targetEntity WRITE setTargetEntity NOTIFY targetEntityChanged FINAL)

public:
	RpgNpc(RpgGameItem *gameItem, const cpVect &center = cpvzero);
	virtual ~RpgNpc();

	virtual void initialize() override;
	virtual void updateSprite() override;

	void load(const RpgNpcDefinition &config);

	void setConfig(const RpgNpcDefinition &config);

	RpgEntity *targetEntity() const;
	void setTargetEntity(RpgEntity *newTargetEntity);

signals:
	void targetEntityChanged();

protected:
	void synchronize() override;
	void onAlive() override;
	void onDead() override;
	void updateColor() override;

private:
	void loadSfx();

private:
	TiledGameSfx m_sfxPain;
	TiledGameSfx m_sfxFootStep;

	RpgNpcDefinition m_config;
	RpgEntity *m_targetEntity = nullptr;

	QQuickItem *m_markerItem = nullptr;

	friend class RpgMotorNpc;
	friend class RpgMotorNpcControlled;
};







/**
 * @brief The RpgMotorNpc class
 */

class RpgMotorNpc : public RpgMotorEntity
{
public:
	RpgMotorNpc(RpgNpc *npc);

	virtual bool beforeWorldStep(const qint64 &tick, entt::entity &entity) override;
	virtual void updateBody(TiledObject *) override;

	static void updateBody(RpgNpc *npc, const RpgStream::NpcState &state, const bool &isEmplace);

protected:
	RpgNpc *const m_npc;
	std::optional<RpgStream::NpcState> m_current;
};










/**
 * @brief The RpgMotorNpcControlled class
 */

class RpgMotorNpcControlled : public RpgDestinationMotor
{
public:
	RpgMotorNpcControlled(RpgNpc *npc);

	virtual void updateBody(TiledObject *) override;
	virtual bool beforeWorldStep(const qint64 &tick, entt::entity &entity) override;
	virtual bool afterWorldStep(const qint64 &tick, RpgStream::FullState *state) override;

	const RpgStream::NpcState *saveCurrentState(const qint64 &tick);

protected:
	virtual void onShapeContactBegin(cpShape *self, cpShape *other) override;
	virtual void onShapeContactEnd(cpShape *self, cpShape *other) override;

	virtual void updateTarget();
	virtual int getMovementSpeed();
	virtual void updateMovement(const float &speed);
	virtual void updateMotor();

	virtual void saveState(RpgStream::NpcState &dest) { Q_UNUSED(dest); }

	void applyKnockback();

protected:
	RpgNpc *const m_npc;
	Rpg::RpgNpcStatePull m_statePull;

	std::vector<RpgStream::EventNpc> m_eventList;

	cpVect m_currentKnockback = cpvzero;

	friend class RpgNpc;
};





#endif // RPGNPC_H
