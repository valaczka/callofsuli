/*
 * ---- Call of Suli ----
 *
 * rpgnpcmpleecher.h
 *
 * Created on: 2026. 07. 27.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgNpcMpLeecher
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

#ifndef RPGNPCMPLEECHER_H
#define RPGNPCMPLEECHER_H

#include "rpgnpc.h"
#include <QQmlEngine>



/**
 * @brief The RpgNpcMpLeecherDefinition class
 */

class RpgNpcMpLeecherDefinition : public QSerializer
{
	Q_GADGET

public:
	RpgNpcMpLeecherDefinition() : QSerializer()
	  , attackDelay(1000)					// Delay between attacks (msec)
	{}


	QS_SERIALIZABLE

	QS_FIELD(int, attackDelay)
};




/**
 * @brief The RpgNpcMpLeecher class
 */

class RpgNpcMpLeecher : public RpgNpc
{
	Q_OBJECT
	QML_ELEMENT

public:
	RpgNpcMpLeecher(RpgGameItem *gameItem, const cpVect &center = cpvzero);
	virtual ~RpgNpcMpLeecher();

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

class RpgMotorNpcMpLeecher : public RpgMotorNpcControlled
{
public:
	RpgMotorNpcMpLeecher(RpgNpc *npc);

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

	QPointer<RpgPlayer> m_target;
	qint64 m_targetReached = 0;
	qint64 m_lastAttack = 0;

	RpgNpcMpLeecherDefinition m_config;
};


#endif // RPGNPCMPLEECHER_H
