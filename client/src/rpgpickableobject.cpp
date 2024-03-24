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
	{ QStringLiteral("arrow"), PickableArrow },
	{ QStringLiteral("fireball"), PickableFireball },
	{ QStringLiteral("hp"), PickableHp },
	{ QStringLiteral("longbow"), PickableLongbow },
	{ QStringLiteral("shortbow"), PickableShortbow },
	{ QStringLiteral("longsword"), PickableLongsword },
};



RpgPickableObject::RpgPickableObject(const PickableType &type, QQuickItem *parent)
	: IsometricObjectCircle(parent)
	, TiledPickableIface()
	, m_pickableType(type)
{
	connect(m_fixture.get(), &Box2DCircle::beginContact, this, &RpgPickableObject::fixtureBeginContact);
	connect(m_fixture.get(), &Box2DCircle::endContact, this, &RpgPickableObject::fixtureEndContact);
}



void RpgPickableObject::initialize()
{
	m_body->setBodyType(Box2DBody::Static);

	setZ(1);
	setDefaultZ(1);

	m_fixture->setDensity(0);
	m_fixture->setFriction(0);
	m_fixture->setRestitution(0);
	m_fixture->setSensor(true);

	createVisual();

	load();
	onDeactivated();
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
	m_body->setActive(true);
	m_fixture->setCategories(TiledObjectBody::fixtureCategory(TiledObjectBody::FixturePickable));
	m_fixture->setCollidesWith(Box2DFixture::All);
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
	m_body->setActive(false);
	m_fixture->setCategories(Box2DFixture::None);
	m_fixture->setCollidesWith(Box2DFixture::None);
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



/**
 * @brief RpgPickableObject::fixtureBeginContact
 * @param other
 */

void RpgPickableObject::fixtureBeginContact(Box2DFixture *other)
{
	TiledObjectBase *base = TiledObjectBase::getFromFixture(other);
	RpgGame *g = qobject_cast<RpgGame*>(m_game);

	if (!base || !g)
		return;

	if (other->categories().testFlag(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureVirtualCircle))) {
		if (RpgPlayer *player = dynamic_cast<RpgPlayer*>(base)) {
			if (player == g->controlledPlayer()) {
				setGlowColor(QStringLiteral("#FFF59D"));
				setGlowEnabled(true);
			}
		}
	}
}


/**
 * @brief RpgPickableObject::fixtureEndContact
 * @param other
 */

void RpgPickableObject::fixtureEndContact(Box2DFixture *other)
{
	TiledObjectBase *base = TiledObjectBase::getFromFixture(other);
	RpgGame *g = qobject_cast<RpgGame*>(m_game);

	if (!base || !g)
		return;

	if (other->categories().testFlag(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureVirtualCircle))) {
		if (RpgPlayer *player = dynamic_cast<RpgPlayer*>(base)) {
			if (player == g->controlledPlayer()) {
				setGlowEnabled(false);
			}
		}
	}
}
