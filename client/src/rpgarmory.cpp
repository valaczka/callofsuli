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


/// Static hash

const QHash<TiledWeapon::WeaponType, QString> RpgArmory::m_layerInfoHash = {
	{ TiledWeapon::WeaponLongsword, QStringLiteral("longsword") },
	{ TiledWeapon::WeaponShortbow, QStringLiteral("shortbow") },
	{ TiledWeapon::WeaponLongbow, QStringLiteral("longbow") },
	{ TiledWeapon::WeaponShield, QStringLiteral("shield") }
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
 * @brief RpgArmory::fillAvailableLayers
 * @param dest
 */

void RpgArmory::fillAvailableLayers(IsometricObjectLayeredSprite *dest, const int &spriteNum)
{
	Q_ASSERT(dest);

	for (const QString &s : m_layerInfoHash)
		fillLayer(dest, s, spriteNum);
}


/**
 * @brief RpgArmory::fillLayer
 * @param dest
 * @param layer
 * @param spriteNum
 */

void RpgArmory::fillLayer(IsometricObjectLayeredSprite *dest, const QString &layer, const int &spriteNum)
{
	Q_ASSERT(dest);
	dest->layers.insert(layer, QStringLiteral(":/rpg/%1/_sprite%2.png").arg(layer).arg(spriteNum));
}



/**
 * @brief RpgArmory::fillLayer
 * @param dest
 * @param layer
 * @param subdir
 * @param spriteNum
 */

void RpgArmory::fillLayer(IsometricObjectLayeredSprite *dest, const QString &layer, const QString &subdir, const int &spriteNum)
{
	Q_ASSERT(dest);
	dest->layers.insert(layer, QStringLiteral(":/rpg/%1/_sprite%2.png").arg(subdir).arg(spriteNum));
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

	auto start = m_weaponList->constBegin();

	if (index != -1)
		start += index;

	auto it = (index != -1) ? start+1 : start;

	if (it == m_weaponList->constEnd())
		it = m_weaponList->constBegin();

	do {
		if ((*it)->canAttack())
			wList.append(*it);

		++it;

		if (it == m_weaponList->constEnd())
			it = m_weaponList->constBegin();
	} while (it != start);

	if (wList.isEmpty()) {
		LOG_CWARNING("game") << "No available weapon";
		return nullptr;
	}

	return wList.first();
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

	if (m_currentWeapon) {
		switch (m_currentWeapon->weaponType()) {
			case TiledWeapon::WeaponShortbow:
				layers.append(QStringLiteral("shortbow"));
				break;

			case TiledWeapon::WeaponLongbow:
				layers.append(QStringLiteral("longbow"));
				break;

			case TiledWeapon::WeaponLongsword:
				layers.append(QStringLiteral("longsword"));
				break;

			case TiledWeapon::WeaponShield:
			case TiledWeapon::WeaponHand:
			case TiledWeapon::WeaponGreatHand:
			case TiledWeapon::WeaponInvalid:
				break;
		}
	}

	if (auto it = std::find_if(m_weaponList->cbegin(), m_weaponList->cend(),
							   [](TiledWeapon *w) {
							   return w->weaponType() == TiledWeapon::WeaponShield;
}); it != m_weaponList->cend()) {
		if ((*it)->bulletCount() > 0)
			layers.append(QStringLiteral("shield"));
	}

	handler->setVisibleLayers(layers);
}
