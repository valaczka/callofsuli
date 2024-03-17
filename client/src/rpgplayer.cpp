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
{
	connect(this, &RpgPlayer::hurt, this, [this](){
		m_game->playSfx(QStringLiteral("qrc:/sound/sfx/pain%1.mp3").arg(m_pNum++), m_scene, m_body->bodyPosition());

		if (m_pNum>3)
			m_pNum = 1;

		/*if (!m_scene)
			return;

		if (!m_soundEffectHash.contains(effect)) {
			LOG_CWARNING("scene") << "Invalid sound effect:" << effect;
			return;
		}

		if (from == -1)
			from = m_soundEffectNum.value(effect, 0);

		const QStringList &list = m_soundEffectHash.value(effect);

		if (from >= list.size())
			from = 0;

		m_scene->playSoundPlayerVoice(list.at(from));

		m_soundEffectNum[effect] = from+1;*/
	});
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

	m_spriteHandler->setVisibleLayers({ QStringLiteral("base") });
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
 * @brief RpgPlayer::onAlive
 */

void RpgPlayer::onAlive()
{
	IsometricPlayer::onAlive();
	playHealEffect();
}


/**
 * @brief RpgPlayer::onDead
 */

void RpgPlayer::onDead()
{
	IsometricPlayer::onDead();
	m_game->playSfx(QStringLiteral("qrc:/sound/sfx/dead.mp3"), m_scene, m_body->bodyPosition());
}



/**
 * @brief RpgPlayer::attackedByEnemy
 */

void RpgPlayer::attackedByEnemy(IsometricEnemy *)
{
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
	}
}



/**
 * @brief RpgPlayer::playHealEffect
 */

void RpgPlayer::playHealEffect()
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

	playAuxSprite(":/heal.png", json);
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
