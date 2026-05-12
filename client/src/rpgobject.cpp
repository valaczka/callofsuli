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

RpgObject::RpgObject(RpgGameItem *gameItem, const QPointF &center, const qreal &radius, const cpBodyType &type)
	: IsometricObject(center, radius, gameItem, type)
{
	Q_ASSERT(gameItem);
	Q_ASSERT(gameItem->game());

	m_rpgGame = gameItem->game();
}


/**
 * @brief RpgObject::worldStep
 */

void RpgObject::worldStep()
{
	if (AbstractRpgMotor *m = currentMotor())
		m->updateBody(this);
	else
		LOG_CERROR("game") << "Missing RpgMotor" << this;

	IsometricObject::worldStep();
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


