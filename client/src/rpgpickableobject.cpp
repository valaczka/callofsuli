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

const QHash<QString, RpgPickableObject::PickableType> RpgPickableObject::m_typeHash = {
	{ QStringLiteral("shield"), PickableShield },
	{ QStringLiteral("hp"), PickableHp },
	{ QStringLiteral("mp"), PickableMp },
	{ QStringLiteral("coin"), PickableCoin },
	{ QStringLiteral("longbow"), PickableLongbow },
	{ QStringLiteral("shortbow"), PickableShortbow },
	{ QStringLiteral("longsword"), PickableLongsword },
	{ QStringLiteral("time"), PickableTime },
	{ QStringLiteral("key"), PickableKey },
};



RpgPickableObject::RpgPickableObject(const PickableType &type, TiledScene *scene)
	: IsometricObject(scene)
	, TiledPickableIface()
	, m_pickableType(type)
{
}



void RpgPickableObject::initialize()
{
	setZ(1);
	setDefaultZ(1);

	createVisual();

	load();
	onDeactivated();
}


/**
 * @brief RpgPickableObject::pickableName
 * @param type
 * @return
 */

QString RpgPickableObject::pickableName(const PickableType &type)
{
	switch (type) {
		case PickableHp: return QStringLiteral("HP");
		case PickableShortbow: return TiledWeapon::weaponName(TiledWeapon::WeaponShortbow);
		case PickableLongbow: return TiledWeapon::weaponName(TiledWeapon::WeaponLongbow);
		case PickableLongsword: return TiledWeapon::weaponName(TiledWeapon::WeaponLongsword);
		case PickableDagger: return TiledWeapon::weaponName(TiledWeapon::WeaponDagger);
		case PickableShield: return QStringLiteral("Pajzs");
		case PickableTime: return QStringLiteral("Idő");
		case PickableMp: return QStringLiteral("MP");
		case PickableCoin: return QStringLiteral("Pénz");
		case PickableKey: return QStringLiteral("Kulcs");
		case PickableInvalid: return QStringLiteral("");
	}

	return {};
}


/**
 * @brief RpgPickableObject::pickableNameEn
 * @param type
 * @return
 */

QString RpgPickableObject::pickableNameEn(const PickableType &type)
{
	switch (type) {
		case PickableHp: return QStringLiteral("HP");
		case PickableShortbow: return TiledWeapon::weaponNameEn(TiledWeapon::WeaponShortbow);
		case PickableLongbow: return TiledWeapon::weaponNameEn(TiledWeapon::WeaponLongbow);
		case PickableLongsword: return TiledWeapon::weaponNameEn(TiledWeapon::WeaponLongsword);
		case PickableDagger: return TiledWeapon::weaponNameEn(TiledWeapon::WeaponDagger);
		case PickableShield: return QStringLiteral("Shield");
		case PickableTime: return QStringLiteral("Time");
		case PickableKey: return QStringLiteral("Key");
		case PickableMp: return QStringLiteral("MP");
		case PickableCoin: return QStringLiteral("Coin");
		case PickableInvalid: return QStringLiteral("");
	}

	return {};
}



/**
 * @brief RpgPickableObject::pickableType
 * @return
 */

RpgPickableObject::PickableType RpgPickableObject::pickableType() const
{
	return m_pickableType;
}



/**
 * @brief RpgPickableObject::onActivated
 */

void RpgPickableObject::onActivated()
{
	setVisible(true);
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
	setVisible(false);
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

	setWidth(json.width);
	setHeight(json.height);

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

RpgInventory::RpgInventory(const RpgPickableObject::PickableType &type, const QString &name, QObject *parent)
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
		case RpgPickableObject::PickableKey:
			return QStringLiteral("qrc:/Qaterial/Icons/key-chain.svg");

		case RpgPickableObject::PickableHp:
		case RpgPickableObject::PickableMp:
		case RpgPickableObject::PickableCoin:
		case RpgPickableObject::PickableShortbow:
		case RpgPickableObject::PickableLongbow:
		case RpgPickableObject::PickableLongsword:
		case RpgPickableObject::PickableDagger:
		case RpgPickableObject::PickableShield:
		case RpgPickableObject::PickableTime:
		case RpgPickableObject::PickableInvalid:
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
		case RpgPickableObject::PickableKey:
			return QColor::fromString(QStringLiteral("#FF8F00"));

		case RpgPickableObject::PickableHp:
		case RpgPickableObject::PickableMp:
		case RpgPickableObject::PickableCoin:
		case RpgPickableObject::PickableShortbow:
		case RpgPickableObject::PickableLongbow:
		case RpgPickableObject::PickableLongsword:
		case RpgPickableObject::PickableDagger:
		case RpgPickableObject::PickableShield:
		case RpgPickableObject::PickableTime:
		case RpgPickableObject::PickableInvalid:
			break;
	}

	return QColor::fromRgbF(0.8, 0., 0.);
}
