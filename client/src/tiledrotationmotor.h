/*
 * ---- Call of Suli ----
 *
 * tiledrotationmotor.h
 *
 * Created on: 2025. 07. 10.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * TiledRotationMotor
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

#ifndef TILEDROTATIONMOTOR_H
#define TILEDROTATIONMOTOR_H

#include "abstracttiledmotor.h"

class TiledRotationMotorPrivate;


/**
 * @brief The TiledRotationMotor class
 */

class TiledRotationMotor : public AbstractTiledMotor
{
public:
	TiledRotationMotor();
	virtual ~TiledRotationMotor();

	enum Direction {
		DirectionCCW,
		DirectionCW
	};

	void updateBody(TiledObject *object, const float &distance, AbstractGame::TickTimer *timer = nullptr) override;
	cpVect basePoint() override { return m_point; }

	float from() const;
	void setFrom(float newFrom);

	float to() const;
	void setTo(float newTo);

	int steps() const;
	void setSteps(int newSteps);

	qint64 waitMs() const;
	void setWaitMs(qint64 newWaitMs);

	Direction direction() const;
	void setDirection(Direction newDirection);

	cpVect point() const;
	void setPoint(const cpVect &newPoint);

private:
	cpVect m_point = cpvzero;

	float m_from = 0.;			// angle
	float m_to = 0.;			// angle
	int m_steps = 4;
	qint64 m_waitMs = 1250;
	Direction m_direction = DirectionCCW;

	TiledRotationMotorPrivate *d = nullptr;
	friend class TiledRotationMotorPrivate;
};

#endif // TILEDROTATIONMOTOR_H
