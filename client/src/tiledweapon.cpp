/*
 * ---- Call of Suli ----
 *
 * tiledweapon.cpp
 *
 * Created on: 2024. 03. 18.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * TiledWeapon
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

#include "tiledweapon.h"
#include "tiledgame.h"

TiledWeapon::TiledWeapon(QObject *parent)
	: QObject(parent)
{

}


/**
 * @brief TiledWeapon::~TiledWeapon
 */

TiledWeapon::~TiledWeapon()
{

}




/**
 * @brief TiledWeapon::shot
 * @param targets
 * @param from
 * @param angle
 */


bool TiledWeapon::shot(const bool &forced)
{
	if (m_bulletCount == 0)
		return false;

	if (!forced) {
		if (!m_parentObject->game() || !m_parentObject->game()->tickTimer())
			return false;

		const auto tick = m_parentObject->game()->tickTimer()->currentTick();

		if (!m_disableTimerRepeater && m_timerRepeater >= 0 && m_timerRepeater > tick)
			return false;

		if (m_repeaterIdle > 0)
			m_timerRepeater = tick + m_repeaterIdle;
	}

	eventAttack(nullptr);

	return true;
}







/**
 * @brief TiledWeapon::hit
 * @param target
 */

bool TiledWeapon::hit(TiledObject *target, const bool &forced)
{
	if (m_bulletCount == 0)
		return false;

	if (!forced) {
		if (!m_parentObject->game() || !m_parentObject->game()->tickTimer())
			return false;

		const auto tick = m_parentObject->game()->tickTimer()->currentTick();

		if (!m_disableTimerRepeater && m_timerRepeater >= 0 && m_timerRepeater > tick)
			return false;

		if (m_repeaterIdle > 0)
			m_timerRepeater = tick + m_repeaterIdle;
	}

	eventAttack(target);

	return true;
}




/**
 * @brief TiledWeapon::parentObject
 * @return
 */

TiledObject *TiledWeapon::parentObject() const
{
	return m_parentObject;
}

void TiledWeapon::setParentObject(TiledObject *newParentObject)
{
	if (m_parentObject == newParentObject)
		return;
	m_parentObject = newParentObject;
	emit parentObjectChanged();
}

int TiledWeapon::bulletCount() const
{
	return m_bulletCount;
}

void TiledWeapon::setBulletCount(int newBulletCount)
{
	if (m_bulletCount == newBulletCount)
		return;
	m_bulletCount = newBulletCount;
	emit bulletCountChanged();
	emit canShotChanged();

	setMaxBulletCount(std::max(m_bulletCount, m_maxBulletCount));
}


/**
 * @brief TiledWeapon::icon
 * @return
 */

QString TiledWeapon::icon() const
{
	return m_icon;
}

void TiledWeapon::setIcon(const QString &newIcon)
{
	if (m_icon == newIcon)
		return;
	m_icon = newIcon;
	emit iconChanged();
}



/**
 * @brief TiledWeapon::repeaterIdle
 * @return
 */

qint64 TiledWeapon::repeaterIdle() const
{
	return m_repeaterIdle;
}

void TiledWeapon::setRepeaterIdle(qint64 newRepeaterIdle)
{
	if (m_repeaterIdle == newRepeaterIdle)
		return;
	m_repeaterIdle = newRepeaterIdle;
	emit repeaterIdleChanged();
}


/**
 * @brief TiledWeapon::canThrow
 * @return
 */

bool TiledWeapon::canThrow() const
{
	return m_canThrow;
}

void TiledWeapon::setCanThrow(bool newCanThrow)
{
	m_canThrow = newCanThrow;
}


/**
 * @brief TiledWeapon::canThrowBullet
 * @return
 */

bool TiledWeapon::canThrowBullet() const
{
	return m_canThrowBullet;
}

void TiledWeapon::setCanThrowBullet(bool newCanThrowBullet)
{
	m_canThrowBullet = newCanThrowBullet;
}

int TiledWeapon::maxBulletCount() const
{
	return m_maxBulletCount;
}

void TiledWeapon::setMaxBulletCount(int newMaxBulletCount)
{
	if (m_maxBulletCount == newMaxBulletCount)
		return;
	m_maxBulletCount = newMaxBulletCount;
	emit maxBulletCountChanged();
}

bool TiledWeapon::excludeFromLayers() const
{
	return m_excludeFromLayers;
}

void TiledWeapon::setExcludeFromLayers(bool newExcludeFromLayers)
{
	if (m_excludeFromLayers == newExcludeFromLayers)
		return;
	m_excludeFromLayers = newExcludeFromLayers;
	emit excludeFromLayersChanged();
}

int TiledWeapon::pickedBulletCount() const
{
	return m_pickedBulletCount;
}

void TiledWeapon::setPickedBulletCount(int newPickedBulletCount)
{
	m_pickedBulletCount = newPickedBulletCount;
}

bool TiledWeapon::canCast() const
{
	return m_canCast;
}

void TiledWeapon::setCanCast(bool newCanCast)
{
	if (m_canCast == newCanCast)
		return;
	m_canCast = newCanCast;
	emit canCastChanged();
}

bool TiledWeapon::disableTimerRepeater() const
{
	return m_disableTimerRepeater;
}

void TiledWeapon::setDisableTimerRepeater(bool newDisableTimerRepeater)
{
	m_disableTimerRepeater = newDisableTimerRepeater;
}


/**
 * @brief TiledWeapon::bulletDistance
 * @return
 */

qreal TiledWeapon::bulletDistance() const
{
	return m_bulletDistance;
}

void TiledWeapon::setBulletDistance(qreal newBulletDistance)
{
	if (qFuzzyCompare(m_bulletDistance, newBulletDistance))
		return;
	m_bulletDistance = newBulletDistance;
	emit bulletDistanceChanged();
}



/**
 * @brief TiledWeapon::canHit
 * @return
 */



void TiledWeapon::setCanHit(bool newCanHit)
{
	if (m_canHit == newCanHit)
		return;
	m_canHit = newCanHit;
	emit canHitChanged();
}


