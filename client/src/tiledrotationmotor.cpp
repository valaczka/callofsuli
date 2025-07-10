/*
 * ---- Call of Suli ----
 *
 * tiledrotationmotor.cpp
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

#include "tiledrotationmotor.h"


static const float pi2 = 2*M_PI;

class TiledRotationMotorPrivate {
private:
	TiledRotationMotorPrivate(TiledRotationMotor *motor)
		: q(motor)
	{}


	void update(TiledObject *object);
	void recalculateDelta();


	TiledRotationMotor *const q;

	qint64 m_waitTimerEnd = 0;

	float m_delta = 0;
	TiledRotationMotor::Direction m_currentDirection = TiledRotationMotor::DirectionCCW;
	float m_fromNR = 0;
	float m_toNR = 0;

	friend class TiledRotationMotor;
};




/**
 * @brief TiledRotationMotor::TiledRotationMotor
 */

TiledRotationMotor::TiledRotationMotor()
	: AbstractTiledMotor(RotationMotor)
	, d(new TiledRotationMotorPrivate(this))
{

}


/**
 * @brief TiledRotationMotor::~TiledRotationMotor
 */

TiledRotationMotor::~TiledRotationMotor()
{
	delete d;
	d = nullptr;
}



/**
 * @brief TiledRotationMotor::updateBody
 * @param object
 * @param distance
 * @param timer
 */

void TiledRotationMotor::updateBody(TiledObject *object, const float &, AbstractGame::TickTimer *timer)
{
	Q_ASSERT(object);
	Q_ASSERT(timer);

	object->stop();


	if (d->m_waitTimerEnd <= 0) {
		d->m_currentDirection = m_direction;
		object->setCurrentAngle(TiledObject::toRadian(m_from));
	} else if (timer->currentTick() < d->m_waitTimerEnd) {
		return;
	} else {
		d->update(object);
	}

	d->m_waitTimerEnd = timer->tickAddMsec(m_waitMs);
}



/**
 * @brief TiledRotationMotor::from
 * @return
 */

float TiledRotationMotor::from() const
{
	return m_from;
}

void TiledRotationMotor::setFrom(float newFrom)
{
	if (newFrom < 0)
		newFrom = 360+newFrom;
	else if (newFrom > 360)
		newFrom -= 360;

	m_from = newFrom;
	d->m_fromNR = TiledObject::normalizeFromRadian(TiledObject::toRadian(m_from));

	d->recalculateDelta();
}

float TiledRotationMotor::to() const
{
	return m_to;
}

void TiledRotationMotor::setTo(float newTo)
{
	if (newTo < 0)
		newTo = 360+newTo;
	else if (newTo > 360)
		newTo -= 360;

	m_to = newTo;
	d->m_toNR = TiledObject::normalizeFromRadian(TiledObject::toRadian(m_to));

	d->recalculateDelta();
}

int TiledRotationMotor::steps() const
{
	return m_steps;
}

void TiledRotationMotor::setSteps(int newSteps)
{
	m_steps = newSteps;
	d->recalculateDelta();
}

qint64 TiledRotationMotor::waitMs() const
{
	return m_waitMs;
}

void TiledRotationMotor::setWaitMs(qint64 newWaitMs)
{
	m_waitMs = newWaitMs;
}

TiledRotationMotor::Direction TiledRotationMotor::direction() const
{
	return m_direction;
}

void TiledRotationMotor::setDirection(Direction newDirection)
{
	m_direction = newDirection;
	d->recalculateDelta();
}

cpVect TiledRotationMotor::point() const
{
	return m_point;
}

void TiledRotationMotor::setPoint(const cpVect &newPoint)
{
	m_point = newPoint;
}




/**
 * @brief TiledRotationMotorPrivate::update
 * @param object
 */

void TiledRotationMotorPrivate::update(TiledObject *object)
{
	Q_ASSERT(object);

	float currentNormal = TiledObjectBody::normalizeFromRadian(object->desiredBodyRotation());

	if (q->m_from == q->m_to) {
		if (q->m_direction == TiledRotationMotor::DirectionCCW)
			currentNormal -= m_delta;
		else
			currentNormal += m_delta;
	} else {
		const float start = (m_currentDirection == q->m_direction ? m_fromNR : m_toNR);
		const float end = (m_currentDirection == q->m_direction ? m_toNR : m_fromNR);

		float next;

		if (m_currentDirection == TiledRotationMotor::DirectionCCW) {
			next = currentNormal - m_delta;

			const float final = (currentNormal < end && start < end ? end-pi2 : end);

			if (next < final || qFuzzyCompare(next, final)) {
				next = end;
				m_currentDirection = TiledRotationMotor::DirectionCW;
			}
		} else {
			next = currentNormal + m_delta;
			const float final = (currentNormal > end && start > end ? end+pi2 : end);

			if (next > final || qFuzzyCompare(next, final)) {
				next = end;
				m_currentDirection = TiledRotationMotor::DirectionCCW;
			}
		}

		currentNormal = next;
	}

	if (currentNormal > pi2)
		currentNormal -= pi2;
	else if (currentNormal < 0)
		currentNormal += pi2;

	object->setCurrentAngle(TiledObject::normalizeToRadian(currentNormal));
}



/**
 * @brief TiledRotationMotorPrivate::recalculateDelta
 */

void TiledRotationMotorPrivate::recalculateDelta()
{
	float diff = std::abs(q->m_to - q->m_from);

	if (q->m_from == q->m_to)
		diff = 360;
	else if (q->m_direction == TiledRotationMotor::DirectionCCW && q->m_from > q->m_to)
		diff = 360-diff;
	else if (q->m_direction == TiledRotationMotor::DirectionCW && q->m_from < q->m_to)
		diff = 360-diff;

	if (q->m_steps > 1)
		diff /= (float) q->m_steps;

	m_delta = qDegreesToRadians(diff);
}
