/*
 * ---- Call of Suli ----
 *
 * rpgdefender.h
 *
 * Created on: 2026. 05. 22.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgDefender
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

#ifndef RPGDEFENDER_H
#define RPGDEFENDER_H

#include "rpgentity.h"
#include "rpgtower.h"
#include <QQmlEngine>




/**
 * @brief The RpgDefender class
 */

class RpgDefender : public RpgEntity
{
	Q_OBJECT
	QML_ELEMENT

	Q_PROPERTY(RpgTower *tower READ tower CONSTANT FINAL)

public:
	RpgDefender(RpgGameItem *gameItem, const cpVect &pos);

	virtual void initialize() override;

	RpgDefenderPoint *defenderPoint() const;
	void setDefenderPoint(RpgDefenderPoint *newDefenderPoint);

	RpgTower *tower() const;
	void setTower(RpgTower *newTower);

	RpgStream::Team team() const;
	void setTeam(RpgStream::Team newTeam);

protected:
	RpgDefenderPoint* m_defenderPoint = nullptr;
	QPointer<RpgTower> m_tower;
	RpgStream::Team m_team = RpgStream::TeamNone;
};








/**
 * @brief The RpgDefenderMotor class
 */

class RpgDefenderMotor : public AbstractRpgMotor
{
public:
	RpgDefenderMotor(RpgDefender *object)
		: AbstractRpgMotor(object)
		, m_defender(object)
	{}

	virtual void updateBody(TiledObject *) override {};
	virtual bool beforeWorldStep(const qint64 &tick, entt::entity &entity) override;

protected:
	QPointer<RpgDefender> m_defender;
};



#endif // RPGDEFENDER_H
