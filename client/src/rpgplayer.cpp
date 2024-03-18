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
#include "tiledspritehandler.h"
#include "tiledrpggame.h"
#include "utils_.h"
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
	, m_weaponList(new TiledWeaponList)

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
	m_sfxFootStep.setInterval(250);

	connect(this, &RpgPlayer::hurt, this, &RpgPlayer::playHurtEffect);
	connect(this, &RpgPlayer::healed, this, &RpgPlayer::playHealedEffect);
	connect(this, &RpgPlayer::becameAlive, this, &RpgPlayer::playAliveEffect);
	connect(this, &RpgPlayer::becameDead, this, &RpgPlayer::playDeadEffect);

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

RpgPlayer *RpgPlayer::createPlayer(TiledRpgGame *game, TiledScene *scene, const QString &character)
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

	QDirIterator it(QStringLiteral(":/character"), {QStringLiteral("rpgdata.json")}, QDir::Files, QDirIterator::Subdirectories);

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
	if (!weapon)
		return;

	if (!hasAbility())
		return;

	if (weapon->canShot()) {
		weapon->shot(IsometricBullet::TargetEnemy, m_body->bodyPosition(), currentAngle());
		playAttackEffect(weapon);
	} else if (weapon->canHit()) {
		if (!hasAbility())
			return;

		weapon->hit(enemy());

		playAttackEffect(weapon);
	}
}



/**
 * @brief RpgPlayer::nextWeapon
 */

void RpgPlayer::nextWeapon()
{
	if (m_weaponList->empty())
		return;

	int index = m_weaponList->indexOf(m_currentWeapon);

	/// TODO: only attack(!!!)

	if (index < 0 || index >= m_weaponList->size())
		index = 0;
	else
		++index;

	setCurrentWeapon(m_weaponList->at(index));
}









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

	const QString directory = QStringLiteral(":/character/").append(m_character).append(QStringLiteral("/"));

	const auto &ptr = Utils::fileToJsonObject(directory+QStringLiteral("rpgData.json"));

	if (!ptr) {
		LOG_CERROR("game") << "Resource load error";
		return;
	}

	IsometricObjectLayeredSprite json;
	json.fromJson(*ptr);

	appendSprite(json, directory);

	setWidth(128);
	setHeight(128);
	setBodyOffset(0, 0.45*64);

	loadDefaultWeapons();

	connect(m_spriteHandler, &TiledSpriteHandler::currentSpriteChanged, this, &RpgPlayer::onCurrentSpriteChanged);
}





/**
 * @brief RpgPlayer::updateSprite
 */

void RpgPlayer::updateSprite()
{
	if (m_hp <= 0) {
		jumpToSprite("die", m_currentDirection);
		return;
	}

	if (m_spriteHandler->currentSprite() == "block" || m_spriteHandler->currentSprite() == "swing" ||
			m_spriteHandler->currentSprite() == "shoot")
		jumpToSpriteLater("idle", m_currentDirection);
	else if (m_movingDirection != Invalid)
		jumpToSprite("run", m_movingDirection);
	else
		jumpToSprite("idle", m_currentDirection);
}





/**
 * @brief RpgPlayer::attackedByEnemy
 */

void RpgPlayer::attackedByEnemy(IsometricEnemy *, const TiledWeapon::WeaponType &weaponType)
{
	if (!isAlive())
		return;

	setHp(m_hp-1);

	/*if (m_hp < 3)
		m_currentAlteration = "none";
	else if (m_hp < 6)
		m_currentAlteration = "sword";*/

	if (m_hp <= 0) {
		jumpToSprite("die", m_currentDirection);
	} else if (m_spriteHandler->currentSprite() != "swing" && m_spriteHandler->currentSprite() != "shoot") {
		emit hurt();
		jumpToSprite("block", m_currentDirection);
		startInabililty();
	}


	/// updateLayers
}



/**
 * @brief RpgPlayer::loadDefaultWeapons
 */

void RpgPlayer::loadDefaultWeapons()
{
	setCurrentWeapon(weaponAdd(new TiledWeaponHand));
	weaponAdd(new RpgLongsword);
	weaponAdd(new RpgShortbow)->setBulletCount(12);
}



/**
 * @brief RpgPlayer::updateLayers
 */

void RpgPlayer::updateLayers()
{
	QStringList layers;
	layers << QStringLiteral("base");


	if (m_currentWeapon) {
		switch (m_currentWeapon->weaponType()) {
			case TiledWeapon::WeaponShortbow:
				layers.append(QStringLiteral("shortbow"));
				break;

			case TiledWeapon::WeaponSword:
				layers.append(QStringLiteral("longsword"));
				break;

			case TiledWeapon::WeaponShield:
			case TiledWeapon::WeaponHand:
			case TiledWeapon::WeaponInvalid:
				break;
		}
	}

	if (auto it = std::find_if(m_weaponList->cbegin(), m_weaponList->cend(),
							   [](TiledWeapon *w) {
							   return w->weaponType() == TiledWeapon::WeaponShield;
}); it != m_weaponList->cend()) {
		/// add shield;
	}

	m_spriteHandler->setVisibleLayers(layers);
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
	QString test = R"({
			"name": "base",
			"x": 0,
			"y": 0,
			"count": 6,
			"width": 64,
			"height": 64,
			"duration": 60,
			"loops": 1
	})";


	TiledObjectSprite json;
	json.fromJson(QJsonDocument::fromJson(test.toUtf8()).object());

	playAuxSprite(QStringLiteral(":/heal.png"), json);
}


/**
 * @brief RpgPlayer::playDeadEffect
 */

void RpgPlayer::playDeadEffect()
{
	m_game->playSfx(QStringLiteral("qrc:/sound/sfx/dead.mp3"), m_scene, m_body->bodyPosition());
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
		case TiledWeapon::WeaponSword:
			jumpToSprite("swing", m_currentDirection);
			break;

		case TiledWeapon::WeaponShortbow:
			jumpToSprite("shoot", m_currentDirection);
			break;


		case TiledWeapon::WeaponShield:
		case TiledWeapon::WeaponInvalid:
			break;
	}
}



/**
 * @brief RpgPlayer::currentWeapon
 * @return
 */

TiledWeapon *RpgPlayer::currentWeapon() const
{
	return m_currentWeapon;
}

void RpgPlayer::setCurrentWeapon(TiledWeapon *newCurrentWeapon)
{
	if (m_currentWeapon == newCurrentWeapon)
		return;
	m_currentWeapon = newCurrentWeapon;
	emit currentWeaponChanged();
	updateLayers();
}


/**
 * @brief RpgPlayer::weaponList
 * @return
 */

TiledWeaponList *RpgPlayer::weaponList() const
{
	return m_weaponList.get();
}



/**
 * @brief RpgPlayer::weaponAdd
 * @param weapon
 */

TiledWeapon* RpgPlayer::weaponAdd(TiledWeapon *weapon)
{
	if (!weapon)
		return nullptr;

	m_weaponList->append(weapon);
	weapon->setParentObject(this);
	return weapon;
}



/**
 * @brief RpgPlayer::weaponRemove
 * @param weapon
 */

void RpgPlayer::weaponRemove(TiledWeapon *weapon)
{
	if (!weapon)
		return;

	weapon->setParentObject(nullptr);
	m_weaponList->remove(weapon);
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
