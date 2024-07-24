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
#include "isometricplayer.h"
#include "isometricenemy.h"

TiledWeapon::TiledWeapon(const WeaponType &type, QObject *parent)
	: QObject(parent)
	, m_weaponType(type)
{
	m_timerRepeater.setRemainingTime(-1);
}


/**
 * @brief TiledWeapon::~TiledWeapon
 */

TiledWeapon::~TiledWeapon()
{

}



/**
 * @brief TiledWeapon::weaponType
 * @return
 */

TiledWeapon::WeaponType TiledWeapon::weaponType() const
{
	return m_weaponType;
}


/**
 * @brief TiledWeapon::weaponName
 * @param type
 * @return
 */

QString TiledWeapon::weaponName(const WeaponType &type)
{
	switch (type) {
		case WeaponShortbow: return tr("íj");
		case WeaponLongbow: return tr("longbow");
		case WeaponGreatHand: return tr("kéz");
		case WeaponHand: return tr("kéz");
		case WeaponShield: return tr("pajzs");
		case WeaponLongsword: return tr("kard");
		case WeaponDagger: return tr("tőr");
		case WeaponBroadsword: return tr("pallos");
		case WeaponMageStaff: return QStringLiteral("varázsbot");
		case WeaponLightningWeapon: return QStringLiteral("villám");
		case WeaponFireFogWeapon: return QStringLiteral("tűz");
		case WeaponInvalid: return tr("érvénytelen");
	}

	return {};
}


/**
 * @brief TiledWeapon::weaponNameEn
 * @param type
 * @return
 */

QString TiledWeapon::weaponNameEn(const WeaponType &type)
{
	switch (type) {
		case WeaponShortbow: return QStringLiteral("Shortbow");
		case WeaponLongbow: return QStringLiteral("Longbow");
		case WeaponGreatHand: return QStringLiteral("Hand");
		case WeaponHand: return QStringLiteral("Hand");
		case WeaponShield: return QStringLiteral("Shield");
		case WeaponLongsword: return QStringLiteral("Sword");
		case WeaponBroadsword: return QStringLiteral("Broadsword");
		case WeaponDagger: return QStringLiteral("Dagger");
		case WeaponMageStaff: return QStringLiteral("Mage staff");
		case WeaponLightningWeapon: return QStringLiteral("Lightning");
		case WeaponFireFogWeapon: return QStringLiteral("Fire fog");
		case WeaponInvalid: return QStringLiteral("Invalid");
	}

	return {};
}



/**
 * @brief TiledWeapon::shot
 * @param targets
 * @param from
 * @param direction
 */

bool TiledWeapon::shot(const IsometricBullet::Targets &targets, const QPointF &from, const TiledObject::Direction &direction, const qreal &distance)
{
	if (m_bulletCount == 0)
		return false;

	if (!m_disableTimerRepeater && !m_timerRepeater.isForever() && !m_timerRepeater.hasExpired())
		return false;

	IsometricBullet *bullet = createBullet(distance);

	if (!bullet) {
		LOG_CWARNING("game") << "Can't create bullet";
		return false;
	}

	setBulletCount(m_bulletCount-1);

	bullet->setFromWeapon(this);
	bullet->setTargets(targets);
	bullet->shot(from, direction);

	if (m_repeaterIdle > 0)
		m_timerRepeater.setRemainingTime(m_repeaterIdle);

	eventAttack(nullptr);

	return true;
}


/**
 * @brief TiledWeapon::shot
 * @param targets
 * @param from
 * @param angle
 */


bool TiledWeapon::shot(const IsometricBullet::Targets &targets, const QPointF &from, const qreal &angle, const qreal &distance)
{
	if (m_bulletCount == 0)
		return false;

	if (!m_disableTimerRepeater && !m_timerRepeater.isForever() && !m_timerRepeater.hasExpired())
		return false;

	IsometricBullet *bullet = createBullet(distance);

	if (!bullet) {
		LOG_CWARNING("game") << "Can't create bullet";
		return false;
	}

	setBulletCount(m_bulletCount-1);

	bullet->setFromWeapon(this);
	bullet->setTargets(targets);
	bullet->shot(from, angle);

	if (m_repeaterIdle > 0)
		m_timerRepeater.setRemainingTime(m_repeaterIdle);

	eventAttack(nullptr);

	return true;
}



/**
 * @brief TiledWeapon::hit
 * @param target
 */

bool TiledWeapon::hit(TiledObject *target)
{
	if (!m_disableTimerRepeater && !m_timerRepeater.isForever() && !m_timerRepeater.hasExpired())
		return false;

	if (target && m_parentObject) {
		IsometricEnemy *enemy = qobject_cast<IsometricEnemy*>(m_parentObject);
		IsometricPlayer *player = qobject_cast<IsometricPlayer*>(m_parentObject);

		if (player)
			m_parentObject->game()->playerAttackEnemy(m_parentObject, target, m_weaponType);

		if (enemy)
			m_parentObject->game()->enemyAttackPlayer(m_parentObject, target, m_weaponType);

		/// TODO: player attacks player ?
	}

	if (m_repeaterIdle > 0)
		m_timerRepeater.setRemainingTime(m_repeaterIdle);

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
 * @brief TiledWeapon::canHit
 * @return
 */

bool TiledWeapon::canHit() const
{
	return m_canHit;
}

void TiledWeapon::setCanHit(bool newCanHit)
{
	if (m_canHit == newCanHit)
		return;
	m_canHit = newCanHit;
	emit canHitChanged();
}






/**
 * @brief TiledWeaponHand::TiledWeaponHand
 * @param parent
 */

TiledWeaponHand::TiledWeaponHand(QObject *parent)
	: TiledWeapon(WeaponHand, parent)
{
	m_canHit = true;
	m_icon = QStringLiteral("qrc:/internal/medal/Icon.3_31.png");
}



/**
 * @brief TiledWeaponHand::eventAttack
 * @param target
 */

void TiledWeaponHand::eventAttack(TiledObject *target)
{
	if (!m_parentObject) {
		LOG_CERROR("game") << "Missing parent object" << this;
		return;
	}

	TiledObject *p = m_parentObject.data();

	if (TiledGame *g = p->game(); g && target) {
		g->playSfx(QStringLiteral(":/rpg/common/hit.mp3"), p->scene(), p->body()->bodyPosition());
	}
}
