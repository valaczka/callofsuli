/*
 * ---- Call of Suli ----
 *
 * rpgpickableobject.cpp
 *
 * Created on: 2024. 03. 19.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgPickableObject
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

#include "rpgpickableobject.h"
#include "rpgplayer.h"
#include "rpggame.h"
#include "utils_.h"



/// Static hash

const QHash<QString, RpgGameData::PickableBaseData::PickableType> RpgPickableObject::m_typeHash = {
	{ QStringLiteral("shield"), RpgGameData::PickableBaseData::PickableShield },
	{ QStringLiteral("hp"), RpgGameData::PickableBaseData::PickableHp },
	{ QStringLiteral("mp"), RpgGameData::PickableBaseData::PickableMp },
	{ QStringLiteral("coin"), RpgGameData::PickableBaseData::PickableCoin },
	{ QStringLiteral("longbow"), RpgGameData::PickableBaseData::PickableLongbow },
	{ QStringLiteral("shortbow"), RpgGameData::PickableBaseData::PickableShortbow },
	{ QStringLiteral("longsword"), RpgGameData::PickableBaseData::PickableLongsword },
	{ QStringLiteral("time"), RpgGameData::PickableBaseData::PickableTime },
	{ QStringLiteral("key"), RpgGameData::PickableBaseData::PickableKey },
};



RpgPickableObject::RpgPickableObject(const RpgGameData::PickableBaseData::PickableType &type, TiledScene *scene)
	: IsometricObject(scene)
	, TiledPickableIface()
	, m_pickableType(type)
{
}



void RpgPickableObject::initialize()
{
	setDefaultZ(1);

	createVisual();

	m_visualItem->setZ(1);

	load();
	onDeactivated();
}


/**
 * @brief RpgPickableObject::pickableName
 * @param type
 * @return
 */

QString RpgPickableObject::pickableName(const RpgGameData::PickableBaseData::PickableType &type)
{
	switch (type) {
		case RpgGameData::PickableBaseData::PickableHp: return QStringLiteral("HP");
		case RpgGameData::PickableBaseData::PickableShortbow: return RpgWeapon::weaponName(RpgGameData::Weapon::WeaponShortbow);
		case RpgGameData::PickableBaseData::PickableLongbow: return RpgWeapon::weaponName(RpgGameData::Weapon::WeaponLongbow);
		case RpgGameData::PickableBaseData::PickableLongsword: return RpgWeapon::weaponName(RpgGameData::Weapon::WeaponLongsword);
		case RpgGameData::PickableBaseData::PickableDagger: return RpgWeapon::weaponName(RpgGameData::Weapon::WeaponDagger);
		case RpgGameData::PickableBaseData::PickableShield: return QStringLiteral("Pajzs");
		case RpgGameData::PickableBaseData::PickableTime: return QStringLiteral("Idő");
		case RpgGameData::PickableBaseData::PickableMp: return QStringLiteral("MP");
		case RpgGameData::PickableBaseData::PickableCoin: return QStringLiteral("Pénz");
		case RpgGameData::PickableBaseData::PickableKey: return QStringLiteral("Kulcs");
		case RpgGameData::PickableBaseData::PickableInvalid: return QStringLiteral("");
	}

	return {};
}


/**
 * @brief RpgPickableObject::pickableNameEn
 * @param type
 * @return
 */

QString RpgPickableObject::pickableNameEn(const RpgGameData::PickableBaseData::PickableType &type)
{
	switch (type) {
		case RpgGameData::PickableBaseData::PickableHp: return QStringLiteral("HP");
		case RpgGameData::PickableBaseData::PickableShortbow: return RpgWeapon::weaponNameEn(RpgGameData::Weapon::WeaponShortbow);
		case RpgGameData::PickableBaseData::PickableLongbow: return RpgWeapon::weaponNameEn(RpgGameData::Weapon::WeaponLongbow);
		case RpgGameData::PickableBaseData::PickableLongsword: return RpgWeapon::weaponNameEn(RpgGameData::Weapon::WeaponLongsword);
		case RpgGameData::PickableBaseData::PickableDagger: return RpgWeapon::weaponNameEn(RpgGameData::Weapon::WeaponDagger);
		case RpgGameData::PickableBaseData::PickableShield: return QStringLiteral("Shield");
		case RpgGameData::PickableBaseData::PickableTime: return QStringLiteral("Time");
		case RpgGameData::PickableBaseData::PickableKey: return QStringLiteral("Key");
		case RpgGameData::PickableBaseData::PickableMp: return QStringLiteral("MP");
		case RpgGameData::PickableBaseData::PickableCoin: return QStringLiteral("Coin");
		case RpgGameData::PickableBaseData::PickableInvalid: return QStringLiteral("");
	}

	return {};
}



/**
 * @brief RpgPickableObject::pickableType
 * @return
 */

RpgGameData::PickableBaseData::PickableType RpgPickableObject::pickableType() const
{
	return m_pickableType;
}



/**
 * @brief RpgPickableObject::onActivated
 */

void RpgPickableObject::onActivated()
{
	if (m_visualItem)
		m_visualItem->setVisible(true);
	setBodyEnabled(true);
	setSubZ(0.3);
	if (m_activateEffect)
		m_activateEffect->play();
}


/**
 * @brief RpgPickableObject::onDeactivated
 */

void RpgPickableObject::onDeactivated()
{
	if (m_visualItem)
		m_visualItem->setVisible(false);
	setBodyEnabled(false);
	setSubZ(0.);
	if (m_deactivateEffect)
		m_deactivateEffect->play();
}



/**
 * @brief RpgPickableObject::loadDefault
 * @param directory
 */

void RpgPickableObject::loadDefault(const QString &directory)
{
	const auto &ptr = Utils::fileToJsonObject(QStringLiteral(":/rpg/%1/pickable.json").arg(directory));

	if (!ptr) {
		LOG_CERROR("game") << "Resource load error:" << directory;
		return;
	}

	TiledObjectSprite json;
	json.fromJson(*ptr);

	appendSprite(QStringLiteral(":/rpg/%1/pickable.png").arg(directory), json);

	if (m_visualItem) {
		m_visualItem->setWidth(json.width);
		m_visualItem->setHeight(json.height);
	}

	jumpToSprite("default");
}


QString RpgPickableObject::name() const
{
	return m_name;
}

void RpgPickableObject::setName(const QString &newName)
{
	if (m_name == newName)
		return;
	m_name = newName;
	emit nameChanged();
}



/**
 * @brief RpgPickableObject::onShapeContactBegin
 * @param self
 * @param other
 */

void RpgPickableObject::onShapeContactBegin(b2::ShapeRef, b2::ShapeRef other)
{
	TiledObjectBody *base = TiledObjectBody::fromBodyRef(other.GetBody());
	RpgGame *g = dynamic_cast<RpgGame*>(m_game);

	if (!base || !g)
		return;

	const FixtureCategories categories = FixtureCategories::fromInt(other.GetFilter().categoryBits);
	RpgPlayer *player = categories.testFlag(FixtureTarget) || categories.testFlag(FixturePlayerBody) ?
							dynamic_cast<RpgPlayer*>(base) :
							nullptr;

	if (player && player == g->controlledPlayer()) {
		setGlowColor(QStringLiteral("#FFF59D"));
		setGlowEnabled(true);
	}
}



/**
 * @brief RpgPickableObject::onShapeContactEnd
 * @param self
 * @param other
 */

void RpgPickableObject::onShapeContactEnd(b2::ShapeRef, b2::ShapeRef other)
{
	TiledObjectBody *base = TiledObjectBody::fromBodyRef(other.GetBody());
	RpgGame *g = dynamic_cast<RpgGame*>(m_game);

	if (!base || !g)
		return;

	const FixtureCategories categories = FixtureCategories::fromInt(other.GetFilter().categoryBits);
	RpgPlayer *player = categories.testFlag(FixtureTarget) || categories.testFlag(FixturePlayerBody) ?
							dynamic_cast<RpgPlayer*>(base) :
							nullptr;

	if (player && player == g->controlledPlayer()) {
		setGlowEnabled(false);
	}
}



/**
 * @brief RpgInventory::RpgInventory
 * @param type
 * @param name
 * @param parent
 */

RpgInventory::RpgInventory(const RpgGameData::PickableBaseData::PickableType &type, const QString &name, QObject *parent)
	: QObject(parent)
	, m_pickableType(type)
	, m_name(name)
{

}

QString RpgInventory::name() const
{
	return m_name;
}

void RpgInventory::setName(const QString &newName)
{
	if (m_name == newName)
		return;
	m_name = newName;
	emit nameChanged();
	emit iconChanged();
	emit iconColorChanged();
}



/**
 * @brief RpgInventory::icon
 * @return
 */

QString RpgInventory::icon() const
{
	switch (m_pickableType) {
		case RpgGameData::PickableBaseData::PickableKey:
			return QStringLiteral("qrc:/Qaterial/Icons/key-chain.svg");

		case RpgGameData::PickableBaseData::PickableHp:
		case RpgGameData::PickableBaseData::PickableMp:
		case RpgGameData::PickableBaseData::PickableCoin:
		case RpgGameData::PickableBaseData::PickableShortbow:
		case RpgGameData::PickableBaseData::PickableLongbow:
		case RpgGameData::PickableBaseData::PickableLongsword:
		case RpgGameData::PickableBaseData::PickableDagger:
		case RpgGameData::PickableBaseData::PickableShield:
		case RpgGameData::PickableBaseData::PickableTime:
		case RpgGameData::PickableBaseData::PickableInvalid:
			break;
	}
	return QStringLiteral("qrc:/Qaterial/Icons/help-box-outline.svg");
}



/**
 * @brief RpgInventory::iconColor
 * @return
 */

QColor RpgInventory::iconColor() const
{
	switch (m_pickableType) {
		case RpgGameData::PickableBaseData::PickableKey:
			return QColor::fromString(QStringLiteral("#FF8F00"));

		case RpgGameData::PickableBaseData::PickableHp:
		case RpgGameData::PickableBaseData::PickableMp:
		case RpgGameData::PickableBaseData::PickableCoin:
		case RpgGameData::PickableBaseData::PickableShortbow:
		case RpgGameData::PickableBaseData::PickableLongbow:
		case RpgGameData::PickableBaseData::PickableLongsword:
		case RpgGameData::PickableBaseData::PickableDagger:
		case RpgGameData::PickableBaseData::PickableShield:
		case RpgGameData::PickableBaseData::PickableTime:
		case RpgGameData::PickableBaseData::PickableInvalid:
			break;
	}

	return QColor::fromRgbF(0.8, 0., 0.);
}
