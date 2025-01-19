/*
 * ---- Call of Suli ----
 *
 * litegame.cpp
 *
 * Created on: 2023. 04. 28.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * LiteGame
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

#include "litegame.h"
#include "Logger.h"
#include "application.h"
#include "client.h"
#include "server.h"
#include "gamequestion.h"

#ifndef Q_OS_WASM
#include "standaloneclient.h"
#endif

#define EXTRA_TIME_REQUIRED_FACTOR 0.75

/**
 * @brief LiteGame::LiteGame
 * @param missionLevel
 * @param client
 */

LiteGame::LiteGame(GameMapMissionLevel *missionLevel, Client *client, const bool &isPractice)
	: AbstractLevelGame(isPractice ? GameMap::Practice : GameMap::Lite, missionLevel, client)
{
	Q_ASSERT(missionLevel);

	if (isPractice)
		LOG_CTRACE("game") << "Practice game constructed" << this;
	else
		LOG_CTRACE("game") << "Lite game constructed" << this;

	connect(this, &AbstractLevelGame::msecLeftChanged, this, &LiteGame::onMsecLeftChanged);
	connect(this, &AbstractLevelGame::gameTimeout, this, &LiteGame::onGameTimeout);
}

/**
 * @brief LiteGame::~LiteGame
 */

LiteGame::~LiteGame()
{
	LOG_CTRACE("game") << "Lite/Practice game destroyed" << this;
}


/**
 * @brief LiteGame::onPageReady
 */

void LiteGame::onPageReady()
{
	QVector<Question> list = createQuestions();

	if (list.empty()) {
		m_client->messageError(tr("Nem lehet előkészíteni a kérdéseket!"), tr("Nem lehet elindítani a játékot"));
		pageItem()->setProperty("closeDisabled", QStringLiteral(""));
		pageItem()->setProperty("onPageClose", QVariant(QMetaType::fromType<QJSValue>()));
		pageItem()->setProperty("closeQuestion", QStringLiteral(""));

		unloadPageItem();

		emit gameFinished(AbstractGame::Fail);
		return;
	}

	m_questions.reserve(list.size());
	m_indices.reserve(list.size());

	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(list.begin(), list.end(), g);

	for (const Question &q : list) {
		m_questions.append(LiteQuestion(q));
		m_indices.append(m_questions.size()-1);
	}

	m_questions.squeeze();
	m_indices.squeeze();

	emit questionsChanged();

	pageItem()->setProperty("closeDisabled", QStringLiteral(""));

	pageItem()->setState(QStringLiteral("run"));
}



/**
 * @brief LiteGame::onStarted
 */

void LiteGame::onStarted()
{
	if (m_mode == GameMap::Practice) {
		gameStart();
		nextQuestion();
		return;
	}

	qint64 msec = 0;

	foreach (const LiteQuestion &q, m_questions) {
		ModuleInterface *iface = Application::instance()->objectiveModules().value(q.question().module());

		if (!iface)
			continue;

		msec += SECOND_PER_QUESTION*1000;

		qreal factor = (iface->xpFactor()-1.0) * 2.0;

		// Exponenciálisan növeljük

		msec += factor * SECOND_PER_QUESTION * 1000;
	}

	const qint64 sni = msec*0.5;

	if (m_addExtraTime > 0.0) {
		LOG_CDEBUG("game") << "Add extra time" << m_addExtraTime;
		msec *= (1.0+m_addExtraTime);
	}

	if (m_client->server() && m_client->server()->user() &&
			m_client->server()->user()->roles().testFlag(Credential::SNI))
		msec += sni;

	startWithRemainingTime(msec);

	nextQuestion();
}






/**
 * @brief LiteGame::dialogMessageFinish
 * @param text
 * @param icon
 * @param success
 */

void LiteGame::dialogMessageFinish(const QString &text, const QString &icon, const bool &success)
{
	if (!m_pageItem) {
		LOG_CINFO("game") << text;
		return;
	}

	LOG_CDEBUG("game") << text;

	QMetaObject::invokeMethod(m_pageItem, "messageFinish", Qt::DirectConnection,
							  Q_ARG(QString, text),
							  Q_ARG(QString, icon),
							  Q_ARG(bool, success));
}




/**
 * @brief LiteGame::gameAbort
 */

void LiteGame::gameAbort()
{
	setFinishState(Fail);

	LOG_CINFO("game") << "Game aborted:" << this;

	gameFinish();
}



/**
 * @brief LiteGame::nextQuestion
 * @return
 */

bool LiteGame::nextQuestion() {
	if (m_indices.isEmpty()) {
		onGameSuccess();
		return false;
	}

	if (m_hp <= 0)
		setHp(startHP());

	m_gameQuestion->loadQuestion(m_questions.at(m_indices.at(0)).question());

	return true;
}



/**
 * @brief LiteGame::onMsecLeftChanged
 */

void LiteGame::onMsecLeftChanged()
{
	if (m_mode == GameMap::Practice)
		return;

	const int &msec = msecLeft();

	if (m_timeNotifySendNext > msec) {
		if (msec <= 30000) {
			m_timeNotifySendNext = -1;
			emit timeNotify();
		} else if (msec <= 60000) {
			m_timeNotifySendNext = 30000;
			emit timeNotify();
		}
	}
}


/**
 * @brief LiteGame::onGameQuestionSuccess
 * @param answer
 */

void LiteGame::onGameQuestionSuccess(const QVariantMap &answer)
{
	addStatistics(m_gameQuestion->module(), m_gameQuestion->objectiveUuid(), true, m_gameQuestion->elapsedMsec());

	int xp = m_gameQuestion->questionData().value(QStringLiteral("xpFactor"), 0.0).toReal() * (qreal) LITE_GAME_BASE_XP;
	setXp(m_xp+xp);

	if (!m_indices.isEmpty())
		m_indices.takeFirst();
	emit questionsChanged();

	m_gameQuestion->answerReveal(answer);
	m_gameQuestion->setMsecBeforeHide(0);
	m_gameQuestion->finish();
}



/**i
 * @brief LiteGame::onGameQuestionFailed
 * @param answer
 */

void LiteGame::onGameQuestionFailed(const QVariantMap &answer)
{
#ifndef Q_OS_WASM
	StandaloneClient *client = qobject_cast<StandaloneClient*>(m_client);
	if (client)
		client->performVibrate();
#endif

	addStatistics(m_gameQuestion->module(), m_gameQuestion->objectiveUuid(), false, m_gameQuestion->elapsedMsec());

	setIsFlawless(false);

	if (!m_indices.isEmpty()) {
		const int &idx = m_indices.takeFirst();

		if (m_mode != GameMap::Practice) {
			m_questions[idx].increaseUsed();
			m_indices.append(idx);
		}
		emit questionsChanged();
	}

	if (m_mode != GameMap::Practice) {
		if (m_hp > 1) {
			setHp(m_hp-1);
		} else {
			const int &hp = startHP();
			int used = 0;
			int cnt = hp;

			while (cnt>0) {
				QVector<int> idxs;
				for (int i=0; i<m_questions.size(); ++i) {
					if (m_questions.at(i).used() == used)
						idxs.append(i);
				}

				std::random_device rd;
				std::mt19937 g(rd());
				std::shuffle(idxs.begin(), idxs.end(), g);

				for (const int idx : idxs) {
					if (cnt<=0)
						break;

					m_questions[idx].increaseUsed();
					m_indices.append(idx);
					--cnt;
				}

				++used;

				if (used > 100)
					break;
			}

			emit questionsChanged();

			setHp(0);
		}
	}

	m_gameQuestion->answerReveal(answer);
	m_gameQuestion->setMsecBeforeHide(m_mode == GameMap::Practice ? 3000 : 1250);

	m_gameQuestion->finish();
}




/**
 * @brief LiteGame::onGameQuestionFinished
 */

void LiteGame::onGameQuestionFinished()
{
	nextQuestion();
}




/**
 * @brief LiteGame::onGameTimeout
 */

void LiteGame::onGameTimeout()
{
	if (m_mode == GameMap::Practice)
		return;

	if (m_isFlawless && m_questions.size()) {
		int answeredQuestions = m_questions.size() - m_indices.size();

		qreal factor = (qreal) answeredQuestions / (qreal) m_questions.size();

		if (factor >= EXTRA_TIME_REQUIRED_FACTOR)
			m_shortTimeHelper = true;
	}

	setFinishState(Fail);
	gameFinish();

	if (m_shortTimeHelper)
		dialogMessageFinish(tr("Lejárt az idő\nA játék beleszámít az extra időkérés lehetőségébe"), "qrc:/Qaterial/Icons/timer-sand.svg", false);
	else
		dialogMessageFinish(tr("Lejárt az idő"), "qrc:/Qaterial/Icons/timer-sand.svg", false);
}



/**
 * @brief LiteGame::onGameSuccess
 */

void LiteGame::onGameSuccess()
{
	if (m_mode == GameMap::Practice)
		setFinishState(Fail);
	else
		setFinishState(Success);

	gameFinish();

	dialogMessageFinish(m_isFlawless ? tr("Mission completed\nHibátlan győzelem!") : tr("Mission completed"), "qrc:/Qaterial/Icons/trophy.svg", true);
}



/**
 * @brief LiteGame::loadPage
 * @return
 */

QQuickItem *LiteGame::loadPage()
{
	return m_client->stackPushPage(QStringLiteral("PageLiteGame.qml"), QVariantMap({
																					   { QStringLiteral("game"), QVariant::fromValue(this) }
																				   }));

}


/**
 * @brief LiteGame::connectGameQuestion
 */

void LiteGame::connectGameQuestion()
{
	connect(m_gameQuestion, &GameQuestion::success, this, &LiteGame::onGameQuestionSuccess);
	connect(m_gameQuestion, &GameQuestion::failed, this, &LiteGame::onGameQuestionFailed);
	connect(m_gameQuestion, &GameQuestion::finished, this, &LiteGame::onGameQuestionFinished);
}


/**
 * @brief LiteGame::gameFinishEvent
 * @return
 */

bool LiteGame::gameFinishEvent()
{
	if (m_closedSuccesfully)
		return false;

	m_closedSuccesfully = true;
	return true;
}


/**
 * @brief LiteGame::addExtraTime
 * @return
 */

qreal LiteGame::addExtraTime() const
{
	return m_addExtraTime;
}

void LiteGame::setAddExtraTime(qreal newAddExtraTime)
{
	m_addExtraTime = newAddExtraTime;
}


/**
 * @brief LiteGame::shortTimeHelper
 * @return
 */

bool LiteGame::shortTimeHelper() const
{
	return m_shortTimeHelper;
}




int LiteGame::hp() const
{
	return m_hp;
}

void LiteGame::setHp(int newHp)
{
	if (m_hp == newHp)
		return;
	m_hp = newHp;
	emit hpChanged();
}



/**
 * @brief LiteGame::questions
 * @return
 */

int LiteGame::questions() const
{
	return m_indices.size();
}
