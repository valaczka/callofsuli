/*
 * ---- Call of Suli ----
 *
 * rpgnpctowerattacker.h
 *
 * Created on: 2026. 07. 02.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgNpcTowerAttacker
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

#ifndef RPGNPCTOWERATTACKER_H
#define RPGNPCTOWERATTACKER_H

#include "rpgdefender.h"
#include "rpgnpc.h"
#include "rpgtower.h"
#include <QQmlEngine>



/**
 * @brief The RpgNpcTowerAttackerDefinition class
 */

class RpgNpcTowerAttackerDefinition : public QSerializer
{
	Q_GADGET

public:
	RpgNpcTowerAttackerDefinition() : QSerializer()
	  , attackDelay(750)					// Delay between attacks (msec)
	{}


	QS_SERIALIZABLE

	QS_FIELD(int, attackDelay)
};






/**
 * @brief The RpgNpcTowerAttacker class
 */

class RpgNpcTowerAttacker : public RpgNpc
{
	Q_OBJECT
	QML_ELEMENT

public:
	RpgNpcTowerAttacker(RpgGameItem *gameItem, const cpVect &center = cpvzero);
	virtual ~RpgNpcTowerAttacker();



	class Motor : public RpgMotorNpc
	{
	public:
		Motor(RpgNpc *npc) : RpgMotorNpc(npc) {}

	protected:
		virtual void processEventAt(const qint64 &tick) override;
	};



	virtual std::unique_ptr<RpgMotorNpcControlled> getControlledMotor() override;
};







/**
 * @brief The RpgMotorNpcControlled class
 */

class RpgMotorNpcTowerAttacker : public RpgMotorNpcControlled
{
public:
	RpgMotorNpcTowerAttacker(RpgNpc *npc);

protected:
	virtual void updateTarget() override;
	virtual void updateMotor() override;
	virtual void saveState(RpgStream::NpcState &dest) override;
	virtual int getMovementSpeed() override;

	virtual void onShapeContactBegin(cpShape *self, cpShape *other) override;
	virtual void onShapeContactEnd(cpShape *self, cpShape *other) override;

	virtual void processEventAt(const qint64 &tick) override;

private:
	void loadTowers();
	void attackTarget();
	RpgDefender *findNextDefender(RpgTower *tower) const;

	std::vector<quint32> m_towers;
	int m_currentIdx = -1;

	QPointer<RpgTower> m_targetTower;
	QPointer<RpgDefender> m_targetDefender;
	qint64 m_targetTowerReached = 0;
	qint64 m_lastAttack = 0;

	RpgNpcTowerAttackerDefinition m_config;
};


#endif // RPGNPCTOWERATTACKER_H
