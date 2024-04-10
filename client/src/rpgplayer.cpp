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
#include "rpggame.h"
#include "utils_.h"
#include <QDirIterator>
#include "application.h"

#ifndef Q_OS_WASM
#include "standaloneclient.h"
#endif


QVector<RpgPlayer::CharacterData> RpgPlayer::m_availableCharacters;


/**
 * @brief RpgPlayer::RpgPlayer
 * @param parent
 */

RpgPlayer::RpgPlayer(QQuickItem *parent)
	: IsometricPlayer(parent)
	, m_sfxPain(this)
	, m_sfxFootStep(this)
	, m_sfxAccept(this)
	, m_sfxDecline(this)
	, m_armory(new RpgArmory(this))
	, m_effectHealed(this)
	, m_effectShield(this)
{
	m_sfxPain.setFollowPosition(false);
	m_sfxAccept.setFollowPosition(false);
	m_sfxDecline.setFollowPosition(false);

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
	connect(this, &RpgPlayer::isLockedChanged, this, &RpgPlayer::playShieldEffect);
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
	TiledObjectBase::createFromCircle<RpgPlayer>(&player, QPointF{}, 30, nullptr, game);

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

	while (it.hasNext()) {
		const QString &f = it.next();

		CharacterData data;
		data.id = f.section('/',-2,-2);

		const auto &ptr = Utils::fileToJsonObject(f);

		if (!ptr) {
			LOG_CERROR("game") << "Invalid config json:" << f;
			continue;
		}

		QString prefixPath = QStringLiteral(":/")+f.section('/', 1, -2).append('/');

		data.config.fromJson(ptr.value());

		if (data.config.name.isEmpty())
			data.config.name = f;


		if (!data.config.image.isEmpty()) {
			data.config.image.prepend(QStringLiteral("qrc")+prefixPath);
		}

		data.config.updateSfxPath(prefixPath);

		m_availableCharacters.append(data);
	}

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

	clearDestinationPoint();

	if (!hasAbility())
		return;

	if (weapon->canShot()) {
		if (weapon->shot(IsometricBullet::TargetEnemy, m_body->bodyPosition(), currentAngle()))
			playAttackEffect(weapon);

		if (weapon->bulletCount() == 0)
			messageEmptyBullet(weapon->weaponType());

	} else if (weapon->canHit()) {
		if (!hasAbility())
			return;

		if (weapon->hit(enemy()))
			playAttackEffect(weapon);
	} else {
#ifndef Q_OS_WASM
		StandaloneClient *client = qobject_cast<StandaloneClient*>(Application::instance()->client());
		if (client)
			client->performVibrate();
#endif
		if (!m_sfxDecline.soundList().isEmpty()) m_sfxDecline.playOne();
		//m_game->messageColor(tr("Empty weapon"), QColor::fromRgbF(0.8, 0., 0.));
	}
}



/**
 * @brief RpgPlayer::attackToPoint
 * @param x
 * @param y
 */

void RpgPlayer::attackToPoint(const qreal &x, const qreal &y)
{
	if (!isAlive())
		return;

	clearDestinationPoint();
	m_pickAtDestination = false;

	if (m_isLocked) {
		m_body->stop();
		return;
	}

	QLineF l(m_body->bodyPosition(), QPointF{x,y});
	setCurrentAngle(toRadian(l.angle()));

	attackCurrentWeapon();
}





/**
 * @brief RpgPlayer::pick
 * @param object
 */

void RpgPlayer::pick(RpgPickableObject *object)
{
	if (!object || !isAlive())
		return;

	clearDestinationPoint();

	if (!m_game->playerPickPickable(this, object)) {
		if (!m_sfxDecline.soundList().isEmpty()) m_sfxDecline.playOne();
	}
}







/**
 * @brief RpgPlayer::load
 */

void RpgPlayer::load()
{
	setAvailableDirections(Direction_8);

	auto ptr = std::find_if(m_availableCharacters.cbegin(), m_availableCharacters.cend(),
							[this](const CharacterData &d) {
		return d.id == m_character;
	});

	if (ptr == m_availableCharacters.cend()) {
		LOG_CERROR("game") << "Invalid character:" << m_character;
		return;
	}

	m_config = ptr->config;


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

	loadSfx();
	loadDefaultWeapons();

	connect(m_spriteHandler, &TiledSpriteHandler::currentSpriteChanged, this, &RpgPlayer::onCurrentSpriteChanged);

	m_armory->updateLayers();

	setShieldCount(m_armory->getShieldCount());
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

	setShieldCount(m_armory->getShieldCount());

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


	int hp = m_hp-1;

	if (hp <= 0) {
		setHp(0);
		jumpToSprite("death", m_currentDirection);
	} else {
		setHp(hp);
		if (m_spriteHandler->currentSprite() != "attack" &&
				m_spriteHandler->currentSprite() != "bow" &&
				m_spriteHandler->currentSprite() != "cast") {
			QTimer::singleShot(200, Qt::PreciseTimer, this, [this](){
				jumpToSprite("hurt", m_currentDirection);
			});
		}
		startInability();
	}

	m_armory->updateLayers();
}



/**
 * @brief RpgPlayer::atDestinationPointEvent
 */

void RpgPlayer::atDestinationPointEvent()
{
	if (m_pickAtDestination)
		pickCurrentObject();

	m_pickAtDestination = false;
}



/**
 * @brief RpgPlayer::loadDefaultWeapons
 */

void RpgPlayer::loadDefaultWeapons()
{
	m_armory->setCurrentWeapon(m_armory->weaponAdd(new TiledWeaponHand));
}




/**
 * @brief RpgPlayer::loadSfx
 */

void RpgPlayer::loadSfx()
{
	if (m_config.sfxPain.isEmpty()) {
		m_sfxPain.setSoundList({
								   QStringLiteral(":/sound/sfx/pain1.mp3"),
								   QStringLiteral(":/sound/sfx/pain2.mp3"),
								   QStringLiteral(":/sound/sfx/pain3.mp3"),
							   });
	} else {
		m_sfxPain.setSoundList(m_config.sfxPain);
	}

	m_sfxPain.setPlayOneDeadline(600);


	if (m_config.sfxFootStep.isEmpty()) {
		m_sfxFootStep.setSoundList({
									   QStringLiteral(":/sound/sfx/run1.mp3"),
									   QStringLiteral(":/sound/sfx/run2.mp3"),
								   });
	} else {
		m_sfxFootStep.setSoundList(m_config.sfxFootStep);
	}

	m_sfxAccept.setSoundList(m_config.sfxAccept);
	m_sfxAccept.setPlayOneDeadline(1500);
	m_sfxDecline.setSoundList(m_config.sfxDecline);
	m_sfxDecline.setPlayOneDeadline(1500);

	m_sfxFootStep.setInterval(350);
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
	m_game->playSfx(m_config.sfxDead.isEmpty() ? QStringLiteral(":/sound/sfx/dead.mp3")
											   : m_config.sfxDead
												 , m_scene /*, m_body->bodyPosition()*/);
	playShieldEffect();
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
		case TiledWeapon::WeaponLongsword:
			jumpToSprite("attack", m_currentDirection);
			break;

		case TiledWeapon::WeaponLongbow:
		case TiledWeapon::WeaponShortbow:
			jumpToSprite("bow", m_currentDirection);
			break;


		case TiledWeapon::WeaponShield:
		case TiledWeapon::WeaponInvalid:
			break;
	}

	emit attackDone();
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
		return m_game->messageColor(tr("Without weapon"), QStringLiteral("#EF5350"));


	m_game->message(armory()->currentWeapon()->weaponNameEn().append(tr(" activated")));

	if (!m_sfxAccept.soundList().isEmpty())
		m_sfxAccept.playOne();
}



/**
 * @brief RpgPlayer::playShieldEffect
 */

void RpgPlayer::playShieldEffect()
{
	if (m_isLocked && m_hp > 0)
		m_effectShield.play();
	else
		m_effectShield.clear();
}



/**
 * @brief RpgPlayer::messageEmptyBullet
 * @param weaponType
 */

void RpgPlayer::messageEmptyBullet(const TiledWeapon::WeaponType &weaponType)
{
	QString msg;

	switch (weaponType) {
		case TiledWeapon::WeaponLongbow:
			msg = tr("All fireballs lost");
			break;
		case TiledWeapon::WeaponShortbow:
			msg = tr("All arrows lost");
			break;

		case TiledWeapon::WeaponShield:
			msg = tr("All shields lost");
			break;

		case TiledWeapon::WeaponHand:
		case TiledWeapon::WeaponGreatHand:
		case TiledWeapon::WeaponLongsword:
		case TiledWeapon::WeaponInvalid:
			break;
	}

	if (msg.isEmpty())
		return;

	m_game->messageColor(msg, QColor::fromRgbF(0.8, 0., 0.));
}

const RpgPlayerCharacterConfig &RpgPlayer::config() const
{
	return m_config;
}


/**
 * @brief RpgPlayer::shieldCount
 * @return
 */

int RpgPlayer::shieldCount() const
{
	return m_shieldCount;
}

void RpgPlayer::setShieldCount(int newShieldCount)
{
	if (m_shieldCount == newShieldCount)
		return;
	m_shieldCount = newShieldCount;
	emit shieldCountChanged();

	if (m_shieldCount == 0)
		messageEmptyBullet(TiledWeapon::WeaponShield);
}


/**
 * @brief RpgPlayer::currentSceneStartPosition
 * @return
 */

QPointF RpgPlayer::currentSceneStartPosition() const
{
	return m_currentSceneStartPosition;
}

void RpgPlayer::setCurrentSceneStartPosition(QPointF newCurrentSceneStartPosition)
{
	m_currentSceneStartPosition = newCurrentSceneStartPosition;
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



/**
 * @brief RpgPlayerCharacterConfig::updateSfxPath
 * @param prefix
 */

void RpgPlayerCharacterConfig::updateSfxPath(const QString &prefix)
{
	for (QStringList *ptr : std::vector<QStringList*>{
		 &sfxAccept,
		 &sfxDecline,
		 &sfxFootStep,
		 &sfxPain,
}) {
		for (QString &s : *ptr) {
			if (!s.isEmpty() && !s.startsWith(QStringLiteral(":/")))
				s.prepend(prefix);
		}
	}

	if (!sfxDead.isEmpty() && !sfxDead.startsWith(QStringLiteral(":/")))
		sfxDead.prepend(prefix);
}
