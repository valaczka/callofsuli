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

static const QVector<TiledGame::TextureSpriteMapper> &mapperBase() {
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
		TiledObject::North, TiledObject::SouthEast, TiledObject::South, TiledObject::SouthWest, TiledObject::West,
		TiledObject::NorthWest, TiledObject::NorthEast, TiledObject::East
	};


	static const QVector<BaseMapper> baseMapper = {
		{ QStringLiteral("idle"), 4, 120, 0 },
		{ QStringLiteral("walk"), 8, 90, 0 },
		{ QStringLiteral("run"), 8, 60, 0 },
		{ QStringLiteral("up"), 8, 60, 1 },
		{ QStringLiteral("down"), 8, 60, 1 },
		{ QStringLiteral("hit1"), 8, 80, 1 },
		{ QStringLiteral("hit2"), 8, 80, 1 },
		{ QStringLiteral("hit3"), 8, 80, 1 },
		{ QStringLiteral("death"), 8, 60, 1 },
		{ QStringLiteral("stand"), 8, 60, 0 },
	};

	for (const auto &m : baseMapper) {
		for (const auto &d : directions) {
			TiledGame::TextureSpriteMapper dst;
			dst.name = m.name;
			dst.direction = d;
			dst.width = 256;
			dst.height = 256;
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

RpgWerebear::RpgWerebear(TiledScene *scene)
	: IsometricEnemy(scene)
	, RpgEnemyIface(EnemyWerebear)
	, m_sfxFootStep(this)
	, m_sfxPain(this)
	, m_sfxRoar(this)
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

	m_sfxRoar.setSoundList({
							   QStringLiteral(":/enemy/werebear/monster-1.mp3"),
						   });

	m_sfxRoar.setPlayOneDeadline(600);

	m_moveDisabledSpriteList = QStringList{
							   QStringLiteral("up"),
							   QStringLiteral("down"),
							   QStringLiteral("hit1"),
							   QStringLiteral("hit2"),
							   QStringLiteral("hit3"),
							   QStringLiteral("stand"),
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

	int hp = 9;

	setMaxHp(hp);
	setHp(hp);

	RpgGame::loadTextureSpritesWithHurt(m_spriteHandler,
										RpgWerebearNS::mapperBase(),
										QStringLiteral(":/enemy/werebear/"));

	setWidth(256);
	setHeight(256);
	setBodyOffset(0, 0.40*128);

	connect(m_spriteHandler, &TiledSpriteHandler::currentSpriteChanged, this, &RpgWerebear::onCurrentSpriteChanged);
}



/**
 * @brief RpgWerebear::eventPlayerReached
 */

void RpgWerebear::eventPlayerReached(IsometricPlayer *)
{
	if (!isStanding())
		jumpToSprite("up", m_facingDirection);

	if (m_player)
		m_sfxRoar.playOne();
}


/**
 * @brief RpgWerebear::eventPlayerLeft
 */

void RpgWerebear::eventPlayerLeft(IsometricPlayer *)
{
	if (isStanding())
		jumpToSprite("down", m_facingDirection);
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
		toDeathSprite();
		eventKilledByPlayer(player);
	} else {
		//jumpToSprite("hurt", m_currentDirection);
		if (weaponType == TiledWeapon::WeaponBroadsword ||
				weaponType == TiledWeapon::WeaponAxe)
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
		case TiledWeapon::WeaponMace:
		case TiledWeapon::WeaponHammer:
		case TiledWeapon::WeaponLongsword:
			newHp -= 1;
			break;

		case TiledWeapon::WeaponLightningWeapon:
		case TiledWeapon::WeaponFireFogWeapon:
			newHp = 0;
			break;

		case TiledWeapon::WeaponShortbow:
			newHp -= 1;
			break;

		case TiledWeapon::WeaponLongbow:
		case TiledWeapon::WeaponBroadsword:
			newHp -= 3;
			break;

		case TiledWeapon::WeaponAxe:
			newHp -= 2;
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
		jumpToSprite(QStringLiteral("hit%1").arg(m_nextHit++).toLatin1(), m_facingDirection);
		if (m_nextHit > 3)
			m_nextHit = 1;
	}
}




/**
 * @brief RpgWerebear::playDeadEffect
 */

void RpgWerebear::playDeadEffect()
{
	m_game->playSfx(QStringLiteral(":/enemy/werebear/monster-6.mp3"), scene(), bodyPosition());
}



/**
 * @brief RpgWerebear::playSeeEffect
 */

void RpgWerebear::playSeeEffect()
{
	if (m_player)
		m_sfxRoar.playOne();
}




/**
 * @brief RpgWerebear::getPickablePosition
 * @return
 */

QPointF RpgWerebear::getPickablePosition(const int &num) const
{
	return bodyPosition() - TiledObject::vectorFromAngle(directionToIsometricRadian(m_facingDirection), 50. *num).toPointF();
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

	LOG_CDEBUG("app") << sprite;

	if (sprite == QStringLiteral("run") || sprite == QStringLiteral("walk"))
		m_sfxFootStep.startFromBegin();
	else if (sprite != QStringLiteral("run") && sprite != QStringLiteral("walk"))
		m_sfxFootStep.stop();
}


/**
 * @brief RpgWerebear::isStanding
 * @return
 */

bool RpgWerebear::isStanding() const
{
	return (m_spriteHandler->currentSprite() == QStringLiteral("up") ||
			//m_spriteHandler->currentSprite() == QStringLiteral("down") ||
			m_spriteHandler->currentSprite() == QStringLiteral("hit1") ||
			m_spriteHandler->currentSprite() == QStringLiteral("hit2") ||
			m_spriteHandler->currentSprite() == QStringLiteral("hit3") ||
			m_spriteHandler->currentSprite() == QStringLiteral("stand")
			);
}


/**
 * @brief RpgWerebear::toDeathSprite
 */

void RpgWerebear::toDeathSprite()
{
	if (isStanding()) {
		jumpToSprite("down", m_facingDirection);
		jumpToSpriteLater("death", m_facingDirection);
	} else if (m_spriteHandler->currentSprite() == QStringLiteral("down"))
		jumpToSpriteLater("death", m_facingDirection);
	else
		jumpToSprite("death", m_facingDirection);
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
		return toDeathSprite();
	else if (m_spriteHandler->currentSprite() == QStringLiteral("hit1") ||
			 m_spriteHandler->currentSprite() == QStringLiteral("hit2") ||
			 m_spriteHandler->currentSprite() == QStringLiteral("hit3") ||
			 m_spriteHandler->currentSprite() == QStringLiteral("up"))
		jumpToSpriteLater("stand", m_facingDirection);
	else if (isStanding())
		jumpToSprite("stand", m_facingDirection);
	else if (isRunning() && m_facingDirection != Invalid)
		jumpToSprite("run", m_facingDirection);
	else if (isWalking() && m_facingDirection != Invalid)
		jumpToSprite("walk", m_facingDirection);
	else
		jumpToSprite("idle", m_facingDirection);
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
							wb->scene(), wb->bodyPosition());
}
