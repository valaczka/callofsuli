/*
 * ---- Call of Suli ----
 *
 * rpgplayer.cpp
 *
 * Created on: 2024. 03. 17.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgPlayer
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

#include "rpgplayer.h"
#include "isometricbullet.h"
#include "rpglongsword.h"
#include "rpgshield.h"
#include "tiledspritehandler.h"
#include "rpggame.h"
#include <QDirIterator>


QStringList RpgPlayer::m_availableCharacters;


/**
 * @brief RpgPlayer::RpgPlayer
 * @param parent
 */

RpgPlayer::RpgPlayer(QQuickItem *parent)
	: IsometricPlayer(parent)
	, m_sfxPain(this)
	, m_sfxFootStep(this)
	, m_armory(new RpgArmory(this))
	, m_effectHealed(this)
{
	m_sfxPain.setSoundList({
							   QStringLiteral(":/sound/sfx/pain1.mp3"),
							   QStringLiteral(":/sound/sfx/pain2.mp3"),
							   QStringLiteral(":/sound/sfx/pain3.mp3"),
						   });

	m_sfxPain.setPlayOneDeadline(600);


	m_sfxFootStep.setSoundList({
								   QStringLiteral(":/sound/sfx/run1.mp3"),
								   QStringLiteral(":/sound/sfx/run2.mp3"),
							   });
	m_sfxFootStep.setInterval(350);

	m_moveDisabledSpriteList = {
		QStringLiteral("attack"),
		QStringLiteral("bow"),
		QStringLiteral("cast"),
		QStringLiteral("hurt"),
		QStringLiteral("death")
	};

	connect(this, &RpgPlayer::hurt, this, &RpgPlayer::playHurtEffect);
	connect(this, &RpgPlayer::healed, this, &RpgPlayer::playHealedEffect);
	connect(this, &RpgPlayer::becameAlive, this, &RpgPlayer::playAliveEffect);
	connect(this, &RpgPlayer::becameDead, this, &RpgPlayer::playDeadEffect);
	connect(m_armory.get(), &RpgArmory::currentWeaponChanged, this, &RpgPlayer::playWeaponChangedEffect);

}


/**
 * @brief RpgPlayer::~RpgPlayer
 */

RpgPlayer::~RpgPlayer()
{

}


/**
 * @brief RpgPlayer::createPlayer
 * @param game
 * @param scene
 * @param character
 * @return
 */

RpgPlayer *RpgPlayer::createPlayer(RpgGame *game, TiledScene *scene, const QString &character)
{
	RpgPlayer *player = nullptr;
	TiledObjectBase::createFromCircle<RpgPlayer>(&player, QPointF{}, 50, nullptr, game);

	if (player) {
		player->setParent(game);
		player->setGame(game);
		player->setScene(scene);
		player->setCharacter(character);
		player->initialize();
	}

	return player;
}



/**
 * @brief RpgPlayer::reloadAvailableCharacters
 */

void RpgPlayer::reloadAvailableCharacters()
{
	LOG_CDEBUG("game") << "Reload available RPG characters...";

	m_availableCharacters.clear();

	QDirIterator it(QStringLiteral(":/rpg"), {QStringLiteral("character.json")}, QDir::Files, QDirIterator::Subdirectories);

	while (it.hasNext())
		m_availableCharacters.append(it.next().section('/',-2,-2));

	LOG_CDEBUG("game") << "...loaded " << m_availableCharacters.size() << " characters";
}


/**
 * @brief RpgPlayer::attack
 * @param weapon
 */

void RpgPlayer::attack(TiledWeapon *weapon)
{
	if (!weapon || !isAlive())
		return;

	if (!hasAbility())
		return;

	if (weapon->canShot()) {
		if (weapon->shot(IsometricBullet::TargetEnemy, m_body->bodyPosition(), currentAngle()))
			playAttackEffect(weapon);
	} else if (weapon->canHit()) {
		if (!hasAbility())
			return;

		if (weapon->hit(enemy()))
			playAttackEffect(weapon);
	}
}





/**
 * @brief RpgPlayer::pick
 * @param object
 */

void RpgPlayer::pick(RpgPickableObject *object)
{
	if (!object || !isAlive())
		return;

	m_game->playerPickPickable(this, object);
}


/**
 * @brief RpgPlayer::pickCurrentObject
 */











/**
 * @brief RpgPlayer::load
 */

void RpgPlayer::load()
{
	setMaxHp(25);
	setHp(25);

	setAvailableDirections(Direction_8);

	if (m_character.isEmpty() || !m_availableCharacters.contains(m_character)) {
		LOG_CERROR("game") << "Invalid character:" << m_character;
		return;
	}


	for (int i=0; i<=2; ++i)
	{
		IsometricObjectLayeredSprite json;
		json.fromJson(RpgGame::baseEntitySprite(i));
		json.layers.insert(QStringLiteral("default"), QStringLiteral("_sprite%1.png").arg(i));
		RpgArmory::fillAvailableLayers(&json, i);
		appendSprite(json, QStringLiteral(":/rpg/")+m_character+QStringLiteral("/"));
	}

	setWidth(148);
	setHeight(130);
	setBodyOffset(0, 0.45*64);

	loadDefaultWeapons();

	connect(m_spriteHandler, &TiledSpriteHandler::currentSpriteChanged, this, &RpgPlayer::onCurrentSpriteChanged);

	m_armory->updateLayers();
}





/**
 * @brief RpgPlayer::updateSprite
 */

void RpgPlayer::updateSprite()
{
	if (m_hp <= 0) {
		jumpToSprite("death", m_currentDirection);
		return;
	}

	if (m_spriteHandler->currentSprite() == "attack" ||
			m_spriteHandler->currentSprite() == "bow" ||
			m_spriteHandler->currentSprite() == "hurt" ||
			m_spriteHandler->currentSprite() == "cast")
		jumpToSpriteLater("idle", m_currentDirection);
	else if (m_movingDirection != Invalid) {
		if (m_runSpeed >= 0. && m_movingSpeed >= m_runSpeed)
			jumpToSprite("run", m_movingDirection);
		else
			jumpToSprite("walk", m_movingDirection);
	} else
		jumpToSprite("idle", m_currentDirection);
}



/**
 * @brief RpgPlayer::protectWeapon
 * @param weaponType
 * @return
 */

bool RpgPlayer::protectWeapon(const TiledWeapon::WeaponType &weaponType)
{
	const bool r = IsometricPlayer::protectWeapon(m_armory->weaponList(), weaponType);

	if (r)
		m_armory->updateLayers();

	return r;
}





/**
 * @brief RpgPlayer::attackedByEnemy
 */

void RpgPlayer::attackedByEnemy(IsometricEnemy *, const TiledWeapon::WeaponType &weaponType, const bool &isProtected)
{
	if (!isAlive())
		return;

	if (isProtected) {
		QTimer::singleShot(200, Qt::PreciseTimer, this, [this](){ jumpToSprite("hurt", m_currentDirection); });
		return;
	}


	int hp = m_hp;

	switch (weaponType) {
		case TiledWeapon::WeaponSword:
			hp -= 2;
			break;

		case TiledWeapon::WeaponShortbow:
			hp -= 2;
			break;

		case TiledWeapon::WeaponHand:
		case TiledWeapon::WeaponGreatHand:
			hp -= 1;
			break;

		case TiledWeapon::WeaponShield:
		case TiledWeapon::WeaponInvalid:
			break;
	}



	if (hp <= 0) {
		setHp(hp);
		jumpToSprite("death", m_currentDirection);
	} else if (m_spriteHandler->currentSprite() != "attack" &&
			   m_spriteHandler->currentSprite() != "bow" &&
			   m_spriteHandler->currentSprite() != "cast") {
		QTimer::singleShot(200, Qt::PreciseTimer, this, [this, hp](){
			setHp(hp);
			jumpToSprite("hurt", m_currentDirection);
		});
		startInabililty();
	}

	m_armory->updateLayers();
}



/**
 * @brief RpgPlayer::loadDefaultWeapons
 */

void RpgPlayer::loadDefaultWeapons()
{
	m_armory->setCurrentWeapon(m_armory->weaponAdd(new TiledWeaponHand));
	m_armory->weaponAdd(new RpgLongsword);
	m_armory->weaponAdd(new RpgShortbow)->setBulletCount(12);
	m_armory->weaponAdd(new RpgShield)->setBulletCount(10);
}





/**
 * @brief RpgPlayer::onCurrentSpriteChanged
 */

void RpgPlayer::onCurrentSpriteChanged()
{
	const QString &sprite = m_spriteHandler->currentSprite();

	if (sprite == QStringLiteral("run"))
		m_sfxFootStep.startFromBegin();
	else if (sprite != QStringLiteral("run"))
		m_sfxFootStep.stop();
}



/**
 * @brief RpgPlayer::playAliveEffect
 */

void RpgPlayer::playAliveEffect()
{
	playHealedEffect();
}


/**
 * @brief RpgPlayer::playHurtEffect
 */

void RpgPlayer::playHurtEffect()
{
	m_sfxPain.playOne();
}


/**
 * @brief RpgPlayer::playHealedEffect
 */

void RpgPlayer::playHealedEffect()
{
	m_effectHealed.play();
}


/**
 * @brief RpgPlayer::playDeadEffect
 */

void RpgPlayer::playDeadEffect()
{
	m_game->playSfx(QStringLiteral(":/sound/sfx/dead.mp3"), m_scene, m_body->bodyPosition());
}





/**
 * @brief RpgPlayer::playAttackEffect
 * @param weapon
 */

void RpgPlayer::playAttackEffect(TiledWeapon *weapon)
{
	if (!weapon)
		return;

	switch (weapon->weaponType()) {
		case TiledWeapon::WeaponHand:
		case TiledWeapon::WeaponGreatHand:
		case TiledWeapon::WeaponSword:
			jumpToSprite("attack", m_currentDirection);
			break;

		case TiledWeapon::WeaponShortbow:
			jumpToSprite("bow", m_currentDirection);
			break;


		case TiledWeapon::WeaponShield:
		case TiledWeapon::WeaponInvalid:
			break;
	}
}




/**
 * @brief RpgPlayer::playWeaponChangedEffect
 */

void RpgPlayer::playWeaponChangedEffect()
{
	RpgGame *g = qobject_cast<RpgGame*>(m_game);

	if (!g || g->controlledPlayer() != this)
		return;

	if (!armory()->currentWeapon())
		return m_game->messageColor(tr("Without weapon"), QStringLiteral("EF5350"));


	m_game->message(tr("Active weapon: ").append(armory()->currentWeapon()->weaponNameEn()));
}





/**
 * @brief RpgPlayer::armory
 * @return
 */

RpgArmory *RpgPlayer::armory() const
{
	return m_armory.get();
}








/**
 * @brief RpgPlayer::character
 * @return
 */

QString RpgPlayer::character() const
{
	return m_character;
}

void RpgPlayer::setCharacter(const QString &newCharacter)
{
	if (m_character == newCharacter)
		return;
	m_character = newCharacter;
	emit characterChanged();
}
