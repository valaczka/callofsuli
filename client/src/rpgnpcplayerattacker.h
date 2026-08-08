/*
 * ---- Call of Suli ----
 *
 * rpgnpcplayerattacker.h
 *
 * Created on: 2026. 08. 08.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgNpcPlayerAttacker
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

#ifndef RPGNPCPLAYERATTACKER_H
#define RPGNPCPLAYERATTACKER_H

#include "rpgnpc.h"
#include <QQmlEngine>




/**
 * @brief The RpgNpcMpLeecherDefinition class
 */

class RpgNpcPlayerAttackerDefinition : public QSerializer
{
	Q_GADGET

public:
	RpgNpcPlayerAttackerDefinition() : QSerializer()
	  , attackDelay(2000)					// Delay between attacks (msec)
	{}


	QS_SERIALIZABLE

	QS_FIELD(int, attackDelay)
};




/**
 * @brief The RpgNpcMpLeecher class
 */

class RpgNpcPlayerAttacker : public RpgNpc
{
	Q_OBJECT
	QML_ELEMENT

public:
	RpgNpcPlayerAttacker(RpgGameItem *gameItem, const cpVect &center = cpvzero);
	virtual ~RpgNpcPlayerAttacker();

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
 * @brief The RpgMotorNpcMpLeecher class
 */

class RpgMotorNpcPlayerAttacker : public RpgMotorNpcControlled
{
public:
	RpgMotorNpcPlayerAttacker(RpgNpc *npc);

protected:
	virtual void updateTarget() override;
	virtual void updateMotor() override;
	virtual void saveState(RpgStream::NpcState &dest) override;
	virtual int getMovementSpeed() override;

	virtual void onShapeContactBegin(cpShape *self, cpShape *other) override;
	virtual void onShapeContactEnd(cpShape *self, cpShape *other) override;

	//virtual void processEventAt(const qint64 &tick) override;

private:
	void attackTarget();

	QList<RpgEntity*> m_targetList;
	int m_head = 0;
	bool m_pursuit = false;
	qint64 m_lastAttack = 0;

	RpgNpcPlayerAttackerDefinition m_config;
};


#endif // RPGNPCPLAYERATTACKER_H
