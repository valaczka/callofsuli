/*
 * ---- Call of Suli ----
 *
 * rpgobject.cpp
 *
 * Created on: 2026. 05. 10.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgObject
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

#include "rpgobject.h"


/**
 * @brief RpgObject::RpgObject
 * @param center
 * @param radius
 * @param game
 * @param type
 */

RpgObject::RpgObject(RpgGameItem *gameItem, const cpVect &center, const qreal &radius, const cpBodyType &type)
	: IsometricObject(toPointF(center), radius, gameItem, type)
{
	Q_ASSERT(gameItem);
	Q_ASSERT(gameItem->game());

	m_rpgGame = gameItem->game();
}

RpgObject::~RpgObject()
{
	if (m_rpgGame)
		m_rpgGame->rpgLogicClient()->removeFromMapper(this);
}


/**
 * @brief RpgObject::worldStep
 */

void RpgObject::worldStep()
{
	IsometricObject::worldStep();

	if (AbstractRpgMotor *m = currentMotor()) {
		m->updateBody(this);
		onMotorStepped();
	} else
		LOG_CERROR("game") << "Missing RpgMotor" << this;

}


/**
 * @brief RpgObject::onShapeContactBegin
 * @param self
 * @param other
 */

void RpgObject::onShapeContactBegin(cpShape *self, cpShape *other)
{
	if (AbstractRpgMotor *m = currentMotor())
		m->onShapeContactBegin(self, other);
}


/**
 * @brief RpgObject::onShapeContactEnd
 * @param self
 * @param other
 */

void RpgObject::onShapeContactEnd(cpShape *self, cpShape *other)
{
	if (AbstractRpgMotor *m = currentMotor())
		m->onShapeContactEnd(self, other);
}


/**
 * @brief RpgObject::secondaryMotor
 * @return
 */

AbstractRpgMotor*RpgObject::secondaryMotor() const
{
	return m_secondaryMotor.get();
}

void RpgObject::setSecondaryMotor(std::unique_ptr<AbstractRpgMotor> newSecondaryMotor)
{
	m_secondaryMotor = std::move(newSecondaryMotor);
}


/**
 * @brief RpgObject::synchronize
 */

void RpgObject::synchronize()
{
	updateSprite();
	IsometricObject::synchronize();
}


/**
 * @brief RpgObject::defaultMotor
 * @return
 */

AbstractRpgMotor*RpgObject::defaultMotor() const
{
	return m_defaultMotor.get();
}

void RpgObject::setDefaultMotor(std::unique_ptr<AbstractRpgMotor> newDefaultMotor)
{
	m_defaultMotor = std::move(newDefaultMotor);
}


/**
 * @brief AbstractRpgMotor::AbstractRpgMotor
 * @param rpgObject
 */

AbstractRpgMotor::AbstractRpgMotor(RpgObject *rpgObject)
	: AbstractTiledMotor()
	, m_object(rpgObject)
{
	Q_ASSERT(m_object);

	m_game = m_object->m_rpgGame;
	m_gameItem = m_game ? m_game->gameItem() : nullptr;
}




/**
 * @brief RpgEasingMotor::updateBody
 */

RpgEasingMotor::RpgEasingMotor(RpgObject *rpgObject, const cpVect &endPos, const quint64 &endTick,
							   const cpVect &startPos, const quint64 &startTick,
							   const QEasingCurve::Type &type)
	: AbstractRpgMotor(rpgObject)
	, m_startTick(startTick)
	, m_endTick(endTick)
	, m_startPos(startPos)
	, m_endPos(endPos)
	, m_curve(type)
{
	if (m_gameItem)
		m_timer = m_gameItem->tickTimer();
}

/**
 * @brief RpgEasingMotor::RpgEasingMotor
 * @param rpgObject
 * @param endPos
 * @param endTick
 * @param startTick
 * @param type
 */

RpgEasingMotor::RpgEasingMotor(RpgObject *rpgObject, const cpVect &endPos, const quint64 &endTick,
							   const quint64 &startTick, const QEasingCurve::Type &type)
	: AbstractRpgMotor(rpgObject)
	, m_startTick(startTick)
	, m_endTick(endTick)
	, m_endPos(endPos)
	, m_curve(type)
{
	if (rpgObject)
		m_startPos = rpgObject->bodyPosition();

	if (m_gameItem)
		m_timer = m_gameItem->tickTimer();
}


/**
 * @brief RpgEasingMotor::RpgEasingMotor
 * @param rpgObject
 * @param endPos
 * @param endTick
 * @param type
 */

RpgEasingMotor::RpgEasingMotor(RpgObject *rpgObject, const cpVect &endPos, const quint64 &endTick, const QEasingCurve::Type &type)
	: RpgEasingMotor(rpgObject, endPos, endTick, 0, type)
{
	if (m_timer)
		m_startTick = m_timer->currentTick();
}






/**
 * @brief RpgEasingMotor::updateBody
 */

void RpgEasingMotor::updateBody(TiledObject *)
{
	if (!m_timer)
		return;

	if (m_finished)
		return;

	const qint64 tick = m_timer->currentTick();

	if (tick >= m_endTick || m_startTick >= m_endTick) {
		m_object->emplace(m_endPos);
		m_finished = true;
		return;
	}

	if (tick < m_startTick)
		return;

	const qreal progress = m_curve.valueForProgress((float) (tick-m_startTick) / (float) (m_endTick-m_startTick));

	const cpVect dest = cpvadd(m_startPos, cpvmult(cpvsub(m_endPos, m_startPos), progress));

	m_object->moveToPoint(dest);
}


/**
 * @brief RpgEasingMotor::startTick
 * @return
 */

qint64 RpgEasingMotor::startTick() const
{
	return m_startTick;
}

void RpgEasingMotor::setStartTick(qint64 newStartTick)
{
	m_startTick = newStartTick;
}

qint64 RpgEasingMotor::endTick() const
{
	return m_endTick;
}

void RpgEasingMotor::setEndTick(qint64 newEndTick)
{
	m_endTick = newEndTick;
}

cpVect RpgEasingMotor::startPos() const
{
	return m_startPos;
}

void RpgEasingMotor::setStartPos(const cpVect &newStartPos)
{
	m_startPos = newStartPos;
}

cpVect RpgEasingMotor::endPos() const
{
	return m_endPos;
}

void RpgEasingMotor::setEndPos(const cpVect &newEndPos)
{
	m_endPos = newEndPos;
}

bool RpgEasingMotor::finished() const
{
	return m_finished;
}



void RpgEasingMotor::setCurve(const QEasingCurve &newCurve)
{
	m_curve = newCurve;
}

