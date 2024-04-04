/*
 * ---- Call of Suli ----
 *
 * rpgquestion.cpp
 *
 * Created on: 2024. 03. 25.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgQuestion
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

#include "rpgquestion.h"
#include "actionrpggame.h"
#include <random>

/**
 * @brief RpgQuestion::RpgQuestion
 * @param game
 */

RpgQuestion::RpgQuestion(ActionRpgGame *game)
	: m_game(game)
{
	Q_ASSERT(m_game);

	m_questionIterator = m_questionList.constBegin();
}


/**
 * @brief RpgQuestion::reloadQuestions
 */

void RpgQuestion::reloadQuestions()
{
	m_questionList = m_game->createQuestions();

	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(m_questionList.begin(), m_questionList.end(), g);

	m_questionIterator = m_questionList.constBegin();
}


/**
 * @brief RpgQuestion::nextQuestion
 * @return
 */

bool RpgQuestion::nextQuestion(RpgPlayer *player, IsometricEnemy *enemy, const TiledWeapon::WeaponType &weaponType)
{
	GameQuestion *gq = m_game->gameQuestion();

	if (!gq) {
		LOG_CERROR("game") << "Missing GameQuestion";
		return false;
	}

	if (gq->questionComponent()) {
		LOG_CERROR("game") << "GameQuestionComponent already loaded";
		return false;
	}

	if (!player) {
		LOG_CERROR("game") << "Missing RpgPlayer";
		return false;
	}

	if (player->isLocked()) {
		LOG_CWARNING("game") << "RpgPlayer already locked";
		return false;
	}

	if (!enemy) {
		LOG_CERROR("game") << "Missing IsometricEnemy";
		return false;
	}

	if (m_questionIterator == m_questionList.constEnd()) {
		LOG_CDEBUG("game") << "Reload questions";
		reloadQuestions();
	}

	if (m_questionIterator == m_questionList.constEnd()) {
		LOG_CERROR("game") << "Reload questions error";
		return false;
	}

	m_player = player;
	m_enemy = enemy;
	m_weaponType = weaponType;

	gq->loadQuestion(*m_questionIterator);
	++m_questionIterator;

	player->onJoystickStateChanged({});
	player->setIsLocked(true);

	return true;
}



/**
 * @brief RpgQuestion::questionSuccess
 * @param answer
 */

void RpgQuestion::questionSuccess(const QVariantMap &answer)
{
	GameQuestion *gq = m_game->gameQuestion();

	if (!gq) {
		LOG_CERROR("game") << "Missing GameQuestion";
		return;
	}

	m_game->addStatistics(gq->module(), gq->objectiveUuid(), true, gq->elapsedMsec());

	//int xp = gq->questionData().value(QStringLiteral("xpFactor"), 0.0).toReal() * (qreal) ACTION_GAME_BASE_XP;
	//setXp(m_xp+xp);

	gq->answerReveal(answer);
	gq->setMsecBeforeHide(0);

	if (m_enemy)
		m_enemy->setHp(0);
		//m_enemy->attackedByPlayer(m_player, m_weaponType);

	gq->finish();
}




/**
 * @brief RpgQuestion::questionFailed
 * @param answer
 */

void RpgQuestion::questionFailed(const QVariantMap &answer)
{
	GameQuestion *gq = m_game->gameQuestion();

	if (!gq) {
		LOG_CERROR("game") << "Missing GameQuestion";
		return;
	}

	m_game->addStatistics(gq->module(), gq->objectiveUuid(), false, gq->elapsedMsec());

	gq->answerReveal(answer);
	gq->setMsecBeforeHide(1250);

	if (m_player)
		m_player->setHp(std::max(0, m_player->hp()-1));

	gq->finish();
}



/**
 * @brief RpgQuestion::questionFinished
 */

void RpgQuestion::questionFinished()
{
	if (m_player)
		m_player->setIsLocked(false);

	m_player = nullptr;
	m_enemy = nullptr;
	m_weaponType = TiledWeapon::WeaponInvalid;
}