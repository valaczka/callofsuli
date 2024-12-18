/*
 * ---- Call of Suli ----
 *
 * rpgarmory.cpp
 *
 * Created on: 2024. 03. 22.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgArmory
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

#include "rpgarmory.h"
#include "tiledspritehandler.h"
#include "rpgmagestaff.h"
#include "rpggame.h"


/// Static hash

const QHash<TiledWeapon::WeaponType, QString> RpgArmory::m_layerInfoHash = {
	{ TiledWeapon::WeaponLongsword, QStringLiteral("longsword") },
	{ TiledWeapon::WeaponShortbow, QStringLiteral("shortbow") },
	{ TiledWeapon::WeaponLongbow, QStringLiteral("longbow") },
	{ TiledWeapon::WeaponDagger, QStringLiteral("dagger") },
	{ TiledWeapon::WeaponBroadsword, QStringLiteral("broadsword") },
	{ TiledWeapon::WeaponMageStaff, QStringLiteral("magestaff") },
	{ TiledWeapon::WeaponAxe, QStringLiteral("axe") },
	{ TiledWeapon::WeaponMace, QStringLiteral("mace") },
	{ TiledWeapon::WeaponHammer, QStringLiteral("hammer") },
	/*{ TiledWeapon::WeaponShield, QStringLiteral("shield") }*/
};


/**
 * @brief RpgArmory::RpgArmory
 * @param parentObject
 * @param parent
 */

RpgArmory::RpgArmory(TiledObject *parentObject, QObject *parent)
	: QObject{parent}
	, m_parentObject(parentObject)
	, m_weaponList(new TiledWeaponList)
{

}



/**
 * @brief RpgArmory::~RpgArmory
 */

RpgArmory::~RpgArmory()
{

}


/**
 * @brief RpgArmory::changeToNextWeapon
 * @return
 */

bool RpgArmory::changeToNextWeapon()
{
	if (!m_nextWeapon)
		return false;

	setCurrentWeapon(m_nextWeapon);
	return true;
}





/**
 * @brief RpgArmory::weaponList
 * @return
 */

TiledWeaponList *RpgArmory::weaponList() const
{
	return m_weaponList.get();
}


/**
 * @brief RpgArmory::weaponFind
 * @param type
 * @return
 */

TiledWeapon *RpgArmory::weaponFind(const TiledWeapon::WeaponType &type) const
{
	for (TiledWeapon *w : std::as_const(*m_weaponList)) {
		if (w->weaponType() == type)
			return w;
	}

	return nullptr;
}


/**
 * @brief RpgArmory::weaponAdd
 * @param weapon
 * @return
 */

TiledWeapon *RpgArmory::weaponAdd(TiledWeapon *weapon)
{
	if (!weapon)
		return nullptr;

	m_weaponList->append(weapon);
	weapon->setParentObject(m_parentObject);
	updateLayers();

	if (weapon->weaponType() == TiledWeapon::WeaponMageStaff)
		emit mageStaffChanged();

	return weapon;
}


/**
 * @brief RpgArmory::weaponRemove
 * @param weapon
 */

void RpgArmory::weaponRemove(TiledWeapon *weapon)
{
	if (!weapon)
		return;

	weapon->setParentObject(nullptr);
	m_weaponList->remove(weapon);

	if (weapon->weaponType() == TiledWeapon::WeaponMageStaff)
		emit mageStaffChanged();

	if (m_currentWeapon == weapon) {
		if (m_nextWeapon == weapon) {
			setNextWeapon(nullptr);
			if (m_weaponList->empty()) {
				setCurrentWeapon(nullptr);
			} else {
				setCurrentWeapon(m_weaponList->first());
			}
		} else {
			if (m_nextWeapon)
				setCurrentWeapon(m_nextWeapon);
			else if (m_weaponList->empty())
				setCurrentWeapon(nullptr);
			else
				setCurrentWeapon(m_weaponList->first());
		}
	} else {
		setNextWeapon(getNextWeapon());
	}
}


/**
 * @brief RpgArmory::currentWeapon
 * @return
 */

TiledWeapon *RpgArmory::currentWeapon() const
{
	return m_currentWeapon;
}

void RpgArmory::setCurrentWeapon(TiledWeapon *newCurrentWeapon)
{
	if (m_currentWeapon == newCurrentWeapon)
		return;
	m_currentWeapon = newCurrentWeapon;
	emit currentWeaponChanged();
	updateLayers();

	setNextWeapon(getNextWeapon());
}


/**
 * @brief RpgArmory::baseLayers
 * @return
 */

QStringList RpgArmory::baseLayers() const
{
	return m_baseLayers;
}

void RpgArmory::setBaseLayers(const QStringList &newBaseLayers)
{
	if (m_baseLayers == newBaseLayers)
		return;
	m_baseLayers = newBaseLayers;
	emit baseLayersChanged();
	updateLayers();
}


/**
 * @brief RpgArmory::updateNextWeapon
 */

void RpgArmory::updateNextWeapon()
{
	setNextWeapon(getNextWeapon());
}


/**
 * @brief RpgArmory::setCurrentWeaponIf
 * @param newCurrentWeapon
 * @param currentType
 */

void RpgArmory::setCurrentWeaponIf(TiledWeapon *newCurrentWeapon, const TiledWeapon::WeaponType &currentType)
{
	if (!m_currentWeapon || m_currentWeapon->weaponType() == currentType)
		setCurrentWeapon(newCurrentWeapon);
	else
		updateNextWeapon();
}



TiledWeapon *RpgArmory::nextWeapon() const
{
	return m_nextWeapon;
}

void RpgArmory::setNextWeapon(TiledWeapon *newNextWeapon)
{
	if (m_nextWeapon == newNextWeapon)
		return;
	m_nextWeapon = newNextWeapon;
	emit nextWeaponChanged();
}


/**
 * @brief RpgArmory::getNextWeapon
 * @return
 */

TiledWeapon *RpgArmory::getNextWeapon() const
{
	if (m_weaponList->empty())
		return nullptr;

	const int index = m_weaponList->indexOf(m_currentWeapon);

	QVector<TiledWeapon *> wList;
	TiledWeapon *weaponHand = nullptr;

	auto start = m_weaponList->constBegin();

	if (index != -1)
		start += index;

	auto it = (index != -1) ? start+1 : start;

	if (it == m_weaponList->constEnd())
		it = m_weaponList->constBegin();

	do {
		if ((*it)->canAttack()) {
			if ((*it)->weaponType() == TiledWeapon::WeaponHand)
				weaponHand = *it;
			else if ((*it)->canHit() || (*it)->canShot()) {
				if (RpgPlayer *p = qobject_cast<RpgPlayer*>(m_parentObject); p && (*it)->weaponType() == TiledWeapon::WeaponMageStaff) {
					if (p->mp() > 0)
						wList.append(*it);
				} else {
					wList.append(*it);
				}
			}
		}

		++it;

		if (it == m_weaponList->constEnd())
			it = m_weaponList->constBegin();
	} while (it != start);

	if (wList.isEmpty()) {
		if (weaponHand)
			return weaponHand;

		LOG_CWARNING("game") << "No available weapon";
		return nullptr;
	}

	return wList.first();
}


/**
 * @brief RpgArmory::mageStaff
 * @return
 */

RpgMageStaff *RpgArmory::mageStaff() const
{
	return qobject_cast<RpgMageStaff*>(weaponFind(TiledWeapon::WeaponMageStaff));
}




/**
 * @brief RpgArmory::updateLayers
 */

void RpgArmory::updateLayers()
{
	if (!m_parentObject || !m_parentObject->spriteHandler())
		return;

	TiledSpriteHandler *handler = m_parentObject->spriteHandler();

	QStringList layers = m_baseLayers;
	QStringList loadableLayers;

	bool addMageStaff = false;

	if (m_currentWeapon && !m_currentWeapon->excludeFromLayers()) {
		switch (m_currentWeapon->weaponType()) {
			case TiledWeapon::WeaponShortbow:
			case TiledWeapon::WeaponLongbow:
			case TiledWeapon::WeaponLongsword:
			case TiledWeapon::WeaponDagger:
			case TiledWeapon::WeaponBroadsword:
			case TiledWeapon::WeaponAxe:
			case TiledWeapon::WeaponMace:
			case TiledWeapon::WeaponHammer:
				layers.append(m_layerInfoHash.value(m_currentWeapon->weaponType()));
				loadableLayers.append(m_layerInfoHash.value(m_currentWeapon->weaponType()));
				break;

			case TiledWeapon::WeaponMageStaff:
			case TiledWeapon::WeaponShield:
			case TiledWeapon::WeaponHand:
			case TiledWeapon::WeaponGreatHand:
			case TiledWeapon::WeaponLightningWeapon:
			case TiledWeapon::WeaponFireFogWeapon:
			case TiledWeapon::WeaponInvalid:
				addMageStaff = true;
				break;
		}
	}

	if (addMageStaff) {
		if (auto it = std::find_if(m_weaponList->cbegin(), m_weaponList->cend(),
								   [](TiledWeapon *w) {
								   return w->weaponType() == TiledWeapon::WeaponMageStaff && !w->excludeFromLayers();
	}); it != m_weaponList->cend()) {
			layers.append(m_layerInfoHash.value(TiledWeapon::WeaponMageStaff));
			loadableLayers.append(m_layerInfoHash.value(TiledWeapon::WeaponMageStaff));
		}
	}

	if (auto it = std::find_if(m_weaponList->cbegin(), m_weaponList->cend(),
							   [](TiledWeapon *w) {
							   return w->weaponType() == TiledWeapon::WeaponShield && !w->excludeFromLayers();
}); it != m_weaponList->cend()) {
		if ((*it)->bulletCount() > 0)
			layers.append(QStringLiteral("shield"));
	}


	for (const QString &s : loadableLayers) {
		if (auto *h = m_parentObject->spriteHandler(); !h->layers().contains(s)) {
			RpgGame::loadBaseTextureSprites(h, QStringLiteral(":/rpg/")+s+QStringLiteral("/"), s);
		}
	}

	handler->setVisibleLayers(layers);
}



/**
 * @brief RpgArmory::getShieldCount
 * @return
 */

int RpgArmory::getShieldCount() const
{
	int n = 0;

	for (TiledWeapon *w : *m_weaponList) {
		if (w->weaponType() == TiledWeapon::WeaponShield) {
			if (w->bulletCount() > 0)
				n += w->bulletCount();
		}
	}

	return n;
}
