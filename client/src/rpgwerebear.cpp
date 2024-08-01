/*
 * ---- Call of Suli ----
 *
 * isometricwerebear.cpp
 *
 * Created on: 2024. 03. 17.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgWerebear
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

#include "rpgwerebear.h"
#include "rpggame.h"
#include "tiledspritehandler.h"


namespace RpgWerebearNS {

static const QVector<TiledGame::TextureSpriteMapper> &mapperBase(const bool &isZero = false) {
	static std::unique_ptr<QVector<TiledGame::TextureSpriteMapper>> mapper;

	if (mapper)
		return *(mapper.get());

	mapper.reset(new QVector<TiledGame::TextureSpriteMapper>);

	struct BaseMapper {
		QString name;
		int count = 0;
		int duration = 0;
		int loops = 0;
	};

	static const QVector<TiledObject::Direction> directions = {
		TiledObject::South, TiledObject::SouthEast, TiledObject::East, TiledObject::NorthEast, TiledObject::North,
		TiledObject::NorthWest, TiledObject::West, TiledObject::SouthWest
	};

	static const QVector<TiledObject::Direction> directions0 = {
		TiledObject::West, TiledObject::NorthWest, TiledObject::North, TiledObject::NorthEast, TiledObject::East,
		TiledObject::SouthEast, TiledObject::South, TiledObject::SouthWest
	};


	static const QVector<BaseMapper> baseMapper = {
		{ QStringLiteral("idle"), 4, 120, 0 },
		{ QStringLiteral("run"), 8, 90, 0 },
		{ QStringLiteral("hit"), 4, 60, 1 },
		{ QStringLiteral("death"), 8, 60, 1 },
		{ QStringLiteral("ignored"), 4, 60, 1 },
	};

	const QVector<TiledObject::Direction> &dir = isZero ? directions0 : directions;

	for (const auto &d : dir) {
		for (const auto &m : baseMapper) {
			TiledGame::TextureSpriteMapper dst;
			dst.name = m.name;
			dst.direction = d;
			dst.width = 128;
			dst.height = 128;
			dst.duration = m.duration;
			dst.loops = m.loops;

			for (int i=0; i<m.count; ++i)
				mapper->append(dst);
		}
	}

	return *(mapper.get());
}

};

/**
 * @brief RpgWerebear::RpgWerebear
 * @param parent
 */

RpgWerebear::RpgWerebear(QQuickItem *parent)
	: IsometricEnemy(parent)
	, RpgEnemyIface(EnemyWerebear)
	, m_sfxFootStep(this)
	, m_sfxPain(this)
	, m_effectHealed(this)
	, m_weaponHand(new RpgWerebearWeaponHand)
{
	setMetric(RpgGame::defaultEnemyMetric().werebear.value(QStringLiteral("default")));

	m_sfxPain.setSoundList({
							   QStringLiteral(":/enemy/werebear/monster-5.mp3"),
						   });

	m_sfxPain.setPlayOneDeadline(600);


	m_sfxFootStep.setSoundList({
								   QStringLiteral(":/enemy/werebear/stepdirt_7.mp3"),
								   QStringLiteral(":/enemy/werebear/stepdirt_8.mp3"),
							   });
	m_sfxFootStep.setVolume(0.4);
	m_sfxFootStep.setInterval(450);

	m_moveDisabledSpriteList = QStringList{
							   QStringLiteral("hurt"),
							   QStringLiteral("death")
};

	m_weaponHand->setParentObject(this);

	connect(this, &RpgWerebear::hurt, &m_sfxPain, &TiledGameSfx::playOne);
	//connect(this, &RpgWerebear::healed, this, [this]() { m_effectHealed.play(); });
	connect(this, &RpgWerebear::becameAlive, this, [this]() {
		m_effectHealed.play();
	});
	connect(this, &RpgWerebear::becameDead, this, &RpgWerebear::playDeadEffect);
	connect(this, &RpgWerebear::playerChanged, this, &RpgWerebear::playSeeEffect);
}


/**
 * @brief RpgWerebear::~RpgWerebear
 */

RpgWerebear::~RpgWerebear()
{

}






/**
 * @brief RpgWerebear::load
 */

void RpgWerebear::load()
{
	setAvailableDirections(Direction_8);

	const char *file = nullptr;
	int hp = 9;

	switch (m_werebearType) {
		case WerebearBrownArmor:
			file = "werebear_brown_armor_texture";
			hp = 11;
			break;
		case WerebearBrownBare:
			file = "werebear_brown_bare_texture";
			break;
		case WerebearBrownShirt:
			file = "werebear_brown_shirt_texture";
			break;
		case WerebearWhiteArmor:
			file = "werebear_white_armor_texture";
			hp = 11;
			break;
		case WerebearWhiteBare:
			file = "werebear_white_bare_texture";
			break;
		case WerebearWhiteShirt:
			file = "werebear_white_shirt_texture";
			break;
		case WerebearDefault:
			file = "werebear_0_texture";
			break;
	}

	setMaxHp(hp);
	setHp(hp);


	RpgGame::loadTextureSpritesWithHurt(m_spriteHandler,
										RpgWerebearNS::mapperBase(m_werebearType == WerebearDefault),
										QStringLiteral(":/enemy/werebear/").append(file));

	setWidth(128);
	setHeight(128);
	setBodyOffset(0, 0.45*64);

	connect(m_spriteHandler, &TiledSpriteHandler::currentSpriteChanged, this, &RpgWerebear::onCurrentSpriteChanged);
}





/**
 * @brief RpgWerebear::attackedByPlayer
 * @param player
 * @param weaponType
 */

void RpgWerebear::attackedByPlayer(IsometricPlayer *player, const TiledWeapon::WeaponType &weaponType)
{
	IsometricEnemy::attackedByPlayer(player, weaponType);

	if (!isAlive() || isSleeping())
		return;

	int newHp = getNewHpAfterAttack(m_hp, weaponType, player);

	if (newHp == m_hp)
		return;

	setHp(std::max(0, newHp));

	if (newHp <= 0) {
		jumpToSprite("death", m_currentDirection);
		eventKilledByPlayer(player);
	} else {
		jumpToSprite("hurt", m_currentDirection);
		if (weaponType != TiledWeapon::WeaponHand)
			startInability();
	}
}



/**
 * @brief RpgWerebear::getNewHpAfterAttack
 * @param origHp
 * @param weaponType
 * @param player
 * @return
 */

int RpgWerebear::getNewHpAfterAttack(const int &origHp, const TiledWeapon::WeaponType &weaponType, IsometricPlayer */*player*/) const
{
	int newHp = origHp;

	switch (weaponType) {
		case TiledWeapon::WeaponLongsword:
			newHp -= 1;
			break;

		case TiledWeapon::WeaponLongbow:
		case TiledWeapon::WeaponLightningWeapon:
		case TiledWeapon::WeaponFireFogWeapon:
			newHp = 0;
			break;

		case TiledWeapon::WeaponShortbow:
			newHp -= 2;
			break;

		case TiledWeapon::WeaponBroadsword:
			newHp -= 3;
			break;

		case TiledWeapon::WeaponHand:
		case TiledWeapon::WeaponGreatHand:
		case TiledWeapon::WeaponDagger:
		case TiledWeapon::WeaponShield:
		case TiledWeapon::WeaponMageStaff:
		case TiledWeapon::WeaponInvalid:
			break;
	}

	return newHp;
}



/**
 * @brief RpgWerebear::playAttackEffect
 * @param weapon
 */

void RpgWerebear::playAttackEffect(TiledWeapon *weapon)
{
	if (!weapon)
		return;

	if (weapon->weaponType() == TiledWeapon::WeaponGreatHand || weapon->weaponType() == TiledWeapon::WeaponHand) {
		jumpToSprite("hit", m_currentDirection);
	}
}




/**
 * @brief RpgWerebear::playDeadEffect
 */

void RpgWerebear::playDeadEffect()
{
	m_game->playSfx(QStringLiteral(":/enemy/werebear/monster-6.mp3"), m_scene, m_body->bodyPosition());
}



/**
 * @brief RpgWerebear::playSeeEffect
 */

void RpgWerebear::playSeeEffect()
{
	if (m_player)
		m_game->playSfx(QStringLiteral(":/enemy/werebear/monster-1.mp3"), m_scene, m_body->bodyPosition());
}




/**
 * @brief RpgWerebear::getPickablePosition
 * @return
 */

QPointF RpgWerebear::getPickablePosition(const int &num) const
{
	QLineF line = QLineF::fromPolar(50. * num, toDegree(directionToIsometricRaidan(m_currentDirection)));
	line.translate(m_body->bodyPosition()-line.p2());
	return line.p1();
}



/**
 * @brief RpgWerebear::protectWeapon
 * @param weaponType
 * @return
 */

bool RpgWerebear::protectWeapon(const TiledWeapon::WeaponType &weaponType)
{
	if (m_weaponHand->canProtect(weaponType) && m_weaponHand->protect(weaponType))
		return true;

	return false;
}






/**
 * @brief RpgWerebear::onCurrentSpriteChanged
 */

void RpgWerebear::onCurrentSpriteChanged()
{
	const QString &sprite = m_spriteHandler->currentSprite();

	if (sprite == QStringLiteral("run"))
		m_sfxFootStep.startFromBegin();
	else if (sprite != QStringLiteral("run"))
		m_sfxFootStep.stop();
}





RpgWerebear::WerebearType RpgWerebear::werebearType() const
{
	return m_werebearType;
}

void RpgWerebear::setWerebearType(const WerebearType &newWerebearType)
{
	if (m_werebearType == newWerebearType)
		return;
	m_werebearType = newWerebearType;
	emit werebearTypeChanged();
}


/**
 * @brief RpgWerebear::setWerebearType
 * @param text
 */

void RpgWerebear::setWerebearType(const QString &text)
{
	static const QMap<QString, WerebearType> map = {
		{ QStringLiteral("brownArmor"),	WerebearBrownArmor },
		{ QStringLiteral("brownBare"),	WerebearBrownBare },
		{ QStringLiteral("brownShirt"),	WerebearBrownShirt },
		{ QStringLiteral("whiteArmor"),	WerebearWhiteArmor },
		{ QStringLiteral("whiteBare"),	WerebearWhiteBare },
		{ QStringLiteral("whiteShirt"),	WerebearWhiteShirt },
	};

	setWerebearType(map.value(text, WerebearDefault));
}


/**
 * @brief RpgWerebear::defaultWeapon
 * @return
 */

TiledWeapon *RpgWerebear::defaultWeapon() const
{
	return m_weaponHand.get();
}



/**
 * @brief RpgWerebear::updateSprite
 */

void RpgWerebear::updateSprite()
{
	if (m_hp <= 0)
		jumpToSprite("death", m_currentDirection);
	else if (m_spriteHandler->currentSprite() == QStringLiteral("hit") || m_spriteHandler->currentSprite() == QStringLiteral("hurt"))
		jumpToSpriteLater("idle", m_currentDirection);
	else if (m_movingDirection != Invalid)
		jumpToSprite("run", m_movingDirection);
	else
		jumpToSprite("idle", m_currentDirection);
}



/**
 * @brief RpgWerebearWeaponHand::eventAttack
 */

RpgWerebearWeaponHand::RpgWerebearWeaponHand(QObject *parent)
	: TiledWeapon(TiledWeapon::WeaponGreatHand, parent)
{
	m_canHit = true;
	m_icon = QStringLiteral("qrc:/Qaterial/Icons/hand.svg");
}



/**
 * @brief RpgWerebearWeaponHand::eventAttack
 */

void RpgWerebearWeaponHand::eventAttack(TiledObject *)
{
	RpgWerebear *wb = qobject_cast<RpgWerebear*>(m_parentObject.data());
	if (wb && wb->game())
		wb->game()->playSfx(QStringLiteral(":/enemy/werebear/big_punch.mp3"),
							wb->scene(), wb->body()->bodyPosition());
}
