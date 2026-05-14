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
#include "tiledpathmotor.h"
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

public:
	RpgPlayer(RpgGameItem *gameItem, const QPointF &center = {});

	QPoint currentChunk() const;
	void setCurrentChunk(QPoint newCurrentChunk);

	float chunkRadius() const;
	void setChunkRadius(float newChunkRadius);

	QPointF currentChunkCenter() const;
	void setCurrentChunkCenter(QPointF newCurrentChunkCenter);

signals:
	void currentChunkChanged();
	void chunkRadiusChanged();

	void currentChunkCenterChanged();

private:
	float m_chunkRadius = 0.;
	QPoint m_currentChunk;
	QPointF m_currentChunkCenter;
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
	virtual bool afterWorldStep(const qint64 &tick, RpgStream::FullState *state) override;

	TiledGame::JoystickState currentJoystickState() const;
	void setCurrentJoystickState(const TiledGame::JoystickState &newCurrentJoystickState);

	void eventTest();

protected:
	RpgPlayer *const m_player;
	Rpg::RpgPlayerStatePull m_statePull;

	TiledGame::JoystickState m_currentJoystickState;
	std::vector<RpgStream::EventPlayer> m_eventList;

};




#endif // RPGPLAYER_H
