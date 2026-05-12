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
#include <QQmlEngine>


/**
 * @brief The RpgPlayer class
 */

class RpgPlayer : public RpgEntity
{
	Q_OBJECT
	QML_ELEMENT

public:
	RpgPlayer(RpgGameItem *gameItem, const QPointF &center = {});

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

class RpgMotorPlayerControlled : public AbstractRpgMotor
{
public:
	RpgMotorPlayerControlled(RpgPlayer *player);

	virtual void updateBody(TiledObject *) override;
	virtual bool afterWorldStep(const qint64 &tick, entt::entity &entity) override;

	TiledGame::JoystickState currentJoystickState() const;
	void setCurrentJoystickState(const TiledGame::JoystickState &newCurrentJoystickState);

protected:
	RpgPlayer *const m_player;
	Rpg::RpgPlayerStatePull m_statePull;

	TiledGame::JoystickState m_currentJoystickState;

};




#endif // RPGPLAYER_H
