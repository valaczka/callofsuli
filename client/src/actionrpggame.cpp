/*
 * ---- Call of Suli ----
 *
 * actionrpggame.cpp
 *
 * Created on: 2024. 03. 24.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ActionRpgGame
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

#include "actionrpggame.h"
#include "Logger.h"
#include "client.h"
#include "gamequestion.h"


/**
 * @brief ActionRpgGame::ActionRpgGame
 * @param missionLevel
 * @param client
 */

ActionRpgGame::ActionRpgGame(GameMapMissionLevel *missionLevel, Client *client)
	: AbstractLevelGame(GameMap::Rpg, missionLevel, client)
{
	LOG_CTRACE("game") << "Action RPG game constructed" << this;

	if (m_missionLevel && m_missionLevel->map()) {
		m_config.mapUuid = m_missionLevel->map()->uuid();
		m_config.missionUuid = m_missionLevel->mission()->uuid();
		m_config.missionLevel = m_missionLevel->level();
	}

	/*connect(this, &AbstractLevelGame::msecLeftChanged, this, &ActionGame::onMsecLeftChanged);
	connect(this, &AbstractLevelGame::gameTimeout, this, &ActionGame::onGameTimeout);
	connect(this, &ActionGame::toolChanged, this, &ActionGame::toolListIconsChanged);*/
}



/**
 * @brief ActionRpgGame::~ActionRpgGame
 */

ActionRpgGame::~ActionRpgGame()
{

}


/**
 * @brief ActionRpgGame::gameAbort
 */

void ActionRpgGame::gameAbort()
{
	setFinishState(Fail);

	LOG_CINFO("game") << "Game aborted:" << this;

	gameFinish();

	m_client->sound()->playSound(QStringLiteral("qrc:/sound/voiceover/game_over.mp3"), Sound::VoiceoverChannel);
	m_client->sound()->playSound(QStringLiteral("qrc:/sound/voiceover/you_lose.mp3"), Sound::VoiceoverChannel);
}




/**
 * @brief ActionRpgGame::playMenuBgMusic
 */

void ActionRpgGame::playMenuBgMusic()
{
m_client->sound()->playSound(QStringLiteral("qrc:/sound/menu/bg.mp3"), Sound::MusicChannel);
}



/**
 * @brief ActionRpgGame::stopMenuBgMusic
 */

void ActionRpgGame::stopMenuBgMusic()
{
	m_client->sound()->stopMusic();
}



/**
 * @brief ActionRpgGame::selectCharacter
 * @param character
 */

void ActionRpgGame::selectCharacter(const QString &character)
{
	LOG_CWARNING("game") << "Character selected:" << character << m_config.gameState;

	auto c = m_config;
	c.gameState = RpgConfig::StatePrepare;
	setConfig(c);
}



/**
 * @brief ActionRpgGame::rpgGameActivated
 */

void ActionRpgGame::rpgGameActivated()
{
	if (!m_rpgGame)
		return;

	m_rpgGame->load();
}




/**
 * @brief ActionRpgGame::loadPage
 * @return
 */

QQuickItem *ActionRpgGame::loadPage()
{

	QQuickItem *item = m_client->stackPushPage(QStringLiteral("PageActionRpgGame.qml"), QVariantMap({
																						   { QStringLiteral("game"), QVariant::fromValue(this) }
																					   }));

	if (!item)
		return nullptr;

	auto c = m_config;
	c.gameState = RpgConfig::StateCharacterSelect;
	setConfig(c);

	return item;
}



/**
 * @brief ActionRpgGame::timerEvent
 */

void ActionRpgGame::timerEvent(QTimerEvent *)
{

}



/**
 * @brief ActionRpgGame::connectGameQuestion
 */

void ActionRpgGame::connectGameQuestion()
{

}


/**
 * @brief ActionRpgGame::gameStartEvent
 * @return
 */

bool ActionRpgGame::gameStartEvent()
{
return true;
}

bool ActionRpgGame::gameFinishEvent()
{
	return true;
}


/**
 * @brief ActionRpgGame::onGamePrepared
 */

void ActionRpgGame::onGamePrepared()
{
	LOG_CINFO("game") << "GAME PREPARED";

	auto c = m_config;
	c.gameState = RpgConfig::StatePlay;
	setConfig(c);
}





/**
 * @brief ActionRpgGame::onGameTimeout
 */

void ActionRpgGame::onGameTimeout()
{
	//m_scene->stopSoundMusic();

	setFinishState(Fail);
	gameFinish();
	m_client->sound()->playSound(QStringLiteral("qrc:/sound/voiceover/game_over.mp3"), Sound::VoiceoverChannel);
	m_client->sound()->playSound(QStringLiteral("qrc:/sound/voiceover/you_lose.mp3"), Sound::VoiceoverChannel);
	//dialogMessageFinish(tr("Lejárt az idő"), "qrc:/Qaterial/Icons/timer-sand.svg", false);
}



/**
 * @brief ActionRpgGame::onGameSuccess
 */

void ActionRpgGame::onGameSuccess()
{
	//m_scene->stopSoundMusic();

	setFinishState(Success);
	gameFinish();

	m_client->sound()->playSound(QStringLiteral("qrc:/sound/sfx/win.mp3"), Sound::SfxChannel);
	m_client->sound()->playSound(QStringLiteral("qrc:/sound/voiceover/game_over.mp3"), Sound::VoiceoverChannel);
	m_client->sound()->playSound(QStringLiteral("qrc:/sound/voiceover/you_win.mp3"), Sound::VoiceoverChannel);

	/*QTimer::singleShot(2000, this, [this](){
		dialogMessageFinish(m_isFlawless ? tr("Mission completed\nHibátlan győzelem!") : tr("Mission completed"), "qrc:/Qaterial/Icons/trophy.svg", true);
	});*/
}



/**
 * @brief ActionRpgGame::onGameFailed
 */

void ActionRpgGame::onGameFailed()
{
	//m_scene->stopSoundMusic();

	setFinishState(Fail);
	gameFinish();
	m_client->sound()->playSound(QStringLiteral("qrc:/sound/voiceover/game_over.mp3"), Sound::VoiceoverChannel);
	m_client->sound()->playSound(QStringLiteral("qrc:/sound/voiceover/you_lose.mp3"), Sound::VoiceoverChannel);
	//dialogMessageFinish(tr("Your man has died"), "qrc:/Qaterial/Icons/skull-crossbones.svg", false);
}




/**
 * @brief ActionRpgGame::onConfigChanged
 */

void ActionRpgGame::onConfigChanged()
{
	//LOG_CDEBUG("game") << "ConquestGame state:" << m_config.gameState << m_config.currentStage << m_config.currentTurn;

	/*if (m_config.currentTurn >= 0 && m_config.currentTurn < m_config.turnList.size())
		setCurrentTurn(m_config.turnList.at(m_config.currentTurn));
	else
		setCurrentTurn({});

	setCurrentStage(m_config.currentStage);*/

	if (m_config.gameState == RpgConfig::StateInvalid)
		return;

	if (m_config.gameState == RpgConfig::StatePrepare && m_oldGameState != RpgConfig::StatePrepare) {
		m_client->sound()->playSound(QStringLiteral("qrc:/sound/voiceover/prepare_yourself.mp3"), Sound::VoiceoverChannel);
		//m_client->sound()->playSound(backgroundMusic(), Sound::MusicChannel);
		stopMenuBgMusic();
	}

	if (m_config.gameState == RpgConfig::StatePlay && m_oldGameState != RpgConfig::StatePlay) {
		m_client->sound()->playSound(QStringLiteral("qrc:/sound/voiceover/begin.mp3"), Sound::VoiceoverChannel);
	}

	/*if (m_config.gameState == ConquestConfig::StateFinished && m_oldGameState != ConquestConfig::StateFinished) {
		m_client->sound()->stopMusic();
		m_client->sound()->playSound(QStringLiteral("qrc:/sound/sfx/win.mp3"), Sound::VoiceoverChannel);
		m_client->sound()->playSound(QStringLiteral("qrc:/sound/voiceover/game_over.mp3"), Sound::VoiceoverChannel);
		if (m_gameSuccess)
			m_client->sound()->playSound(QStringLiteral("qrc:/sound/voiceover/you_win.mp3"), Sound::VoiceoverChannel);
		else
			m_client->sound()->playSound(QStringLiteral("qrc:/sound/voiceover/you_lose.mp3"), Sound::VoiceoverChannel);
	}


	if ((m_config.gameState == ConquestConfig::StatePrepare || m_config.gameState == ConquestConfig::StatePlay) &&
			m_config.world.name != m_loadedWorld) {
		reloadLandList();
		message(tr("Waiting for other players..."));
	}

	for (ConquestLandData *land : *m_landDataList) {
		const int &idx = m_config.world.landFind(land->landId());
		if (idx != -1)
			land->loadFromConfig(m_config.world.landList[idx]);
	}
	*/

	/*if (m_config.gameState == ConquestConfig::StatePrepare && m_oldGameState != ConquestConfig::StatePrepare) {
		sendWebSocketMessage(QJsonObject{
								 { QStringLiteral("cmd"), QStringLiteral("prepare") },
								 { QStringLiteral("engine"), m_engineId },
								 { QStringLiteral("ready"), true }
							 });
	}*/

	m_oldGameState = m_config.gameState;
}



/**
 * @brief ActionRpgGame::playerConfig
 * @return
 */

RpgPlayerConfig ActionRpgGame::playerConfig() const
{
	return m_playerConfig;
}

void ActionRpgGame::setPlayerConfig(const RpgPlayerConfig &newPlayerConfig)
{
	if (m_playerConfig == newPlayerConfig)
		return;
	m_playerConfig = newPlayerConfig;
	emit playerConfigChanged();
}




/**
 * @brief ActionRpgGame::rpgGame
 * @return
 */

RpgGame *ActionRpgGame::rpgGame() const
{
	return m_rpgGame;
}



/**
 * @brief ActionRpgGame::setRpgGame
 * @param newRpgGame
 */

void ActionRpgGame::setRpgGame(RpgGame *newRpgGame)
{
	if (m_rpgGame == newRpgGame)
		return;

	if (m_rpgGame) {
		setGameQuestion(nullptr);
		disconnect(m_rpgGame, &RpgGame::gameLoaded, this, &ActionRpgGame::onGamePrepared);
	}

	m_rpgGame = newRpgGame;
	emit rpgGameChanged();

	if (m_rpgGame) {
		setGameQuestion(m_rpgGame->gameQuestion());
		connect(m_rpgGame, &RpgGame::gameLoaded, this, &ActionRpgGame::onGamePrepared);
	}
}



/**
 * @brief ActionRpgGame::config
 * @return
 */

RpgConfig ActionRpgGame::config() const
{
	return m_config;
}

void ActionRpgGame::setConfig(const RpgConfig &newConfig)
{
	if (m_config == newConfig)
		return;
	m_config = newConfig;
	onConfigChanged();
	emit configChanged();
}
