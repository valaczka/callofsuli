/*
 * ---- Call of Suli ----
 *
 * testgame.cpp
 *
 * Created on: 2023. 04. 28.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * TestGame
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

#include "testgame.h"
#include "Logger.h"
#include "client.h"
#include "gamequestion.h"
#include <QRandomGenerator>


TestGame::TestGame(GameMapMissionLevel *missionLevel, Client *client)
	: AbstractLevelGame(GameMap::Test, missionLevel, client)
{
	Q_ASSERT(missionLevel);

	LOG_CTRACE("game") << "Test game constructed" << this;

	connect(this, &AbstractLevelGame::msecLeftChanged, this, &TestGame::onMsecLeftChanged);
	connect(this, &AbstractLevelGame::gameTimeout, this, &TestGame::onGameTimeout);
}


/**
 * @brief TestGame::~TestGame
 */

TestGame::~TestGame()
{
	LOG_CTRACE("game") << "Test game destroyed" << this;
}




/**
 * @brief TestGame::onPageReady
 */

void TestGame::onPageReady()
{
	QVector<Question> list = createQuestions();

	if (list.isEmpty()) {
		m_client->messageError(tr("Nem lehet előkészíteni a kérdéseket!"), tr("Nem lehet elindítani a játékot"));
		pageItem()->setProperty("closeDisabled", QLatin1String(""));
		pageItem()->setProperty("onPageClose", QVariant::Invalid);
		pageItem()->setProperty("closeQuestion", QLatin1String(""));

		unloadPageItem();
		return;
	}

	m_questionList.reserve(list.size());

	while (!list.isEmpty()) {
		const Question &q = list.takeAt(QRandomGenerator::global()->bounded(list.size()));

		QuestionData d;

		d.url = QStringLiteral("qrc:/")+q.qml();
		d.data = q.generate();
		d.uuid = q.uuid();
		d.module = q.module();

		m_questionList.append(d);
	}

	emit questionsChanged();

	pageItem()->setProperty("closeDisabled", QLatin1String(""));

	pageItem()->setState(QStringLiteral("run"));
}



/**
 * @brief TestGame::onStarted
 */

void TestGame::onStarted()
{
	startWithRemainingTime(m_missionLevel->duration()*1000);
	setCurrentQuestion(0);
	loadCurrentQuestion();
}


/**
 * @brief TestGame::dialogMessageFinish
 * @param text
 * @param icon
 * @param success
 */

void TestGame::dialogMessageFinish(const QString &text, const QString &icon, const bool &success)
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
 * @brief TestGame::gameAbort
 */

void TestGame::gameAbort()
{
	setFinishState(Fail);

	LOG_CINFO("game") << "Game aborted:" << this;

	gameFinish();
}


/**
 * @brief TestGame::onMsecLeftChanged
 */

void TestGame::onMsecLeftChanged()
{
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
 * @brief TestGame::onGameQuestionFinished
 */

void TestGame::onGameQuestionFinished()
{
	m_gameQuestion->setStoredAnswer({});

	if (m_currentQuestion >= 0 && m_currentQuestion < m_questionList.size())
		loadCurrentQuestion();
	else if (m_currentQuestion == m_questionList.size())
		m_gameQuestion->loadQuestion(QStringLiteral("qrc:/GameQuestionTestFinishComponent.qml"), QVariantMap());
	else
		checkAnswers();
}


/**
 * @brief TestGame::onGameQuestionSuccess
 * @param answer
 */

void TestGame::onGameQuestionSuccess(const QVariantMap &answer)
{
	if (m_currentQuestion >= 0 && m_currentQuestion < m_questionList.size()) {
		m_questionList[m_currentQuestion].answer = answer;
		m_questionList[m_currentQuestion].success = true;
	}

	setCurrentQuestion(m_currentQuestion+1);

	m_gameQuestion->setMsecBeforeHide(0);
	m_gameQuestion->finish();
}



/**
 * @brief TestGame::onGameQuestionFailed
 * @param answer
 */

void TestGame::onGameQuestionFailed(const QVariantMap &answer)
{
	if (m_currentQuestion >= 0 && m_currentQuestion < m_questionList.size()) {
		m_questionList[m_currentQuestion].answer = answer;
		m_questionList[m_currentQuestion].success = false;
	}

	setCurrentQuestion(m_currentQuestion+1);

	m_gameQuestion->setMsecBeforeHide(0);
	m_gameQuestion->finish();
}




/**
 * @brief TestGame::onGameTimeout
 */

void TestGame::onGameTimeout()
{
	setFinishState(Fail);
	gameFinish();
	dialogMessageFinish(tr("Lejárt az idő"), "qrc:/Qaterial/Icons/timer-sand.svg", false);
}


/**
 * @brief TestGame::onGameSuccess
 */

void TestGame::onGameSuccess()
{
	setFinishState(Success);
	gameFinish();

	dialogMessageFinish(tr("Mission completed"), "qrc:/Qaterial/Icons/trophy.svg", true);
}


void TestGame::onGameFailed()
{
	setFinishState(Fail);
	gameFinish();
	dialogMessageFinish(tr("A teszt megoldása nem sikerült"), "qrc:/Qaterial/Icons/skull-crossbones.svg", false);
}


/**
 * @brief TestGame::loadPage
 * @return
 */

QQuickItem *TestGame::loadPage()
{
	return m_client->stackPushPage(QStringLiteral("PageTestGame.qml"), QVariantMap({
																					   { QStringLiteral("game"), QVariant::fromValue(this) }
																				   }));
}


/**
 * @brief TestGame::connectGameQuestion
 */

void TestGame::connectGameQuestion()
{
	connect(m_gameQuestion, &GameQuestion::finished, this, &TestGame::onGameQuestionFinished);
	connect(m_gameQuestion, &GameQuestion::success, this, &TestGame::onGameQuestionSuccess);
	connect(m_gameQuestion, &GameQuestion::failed, this, &TestGame::onGameQuestionFailed);
}



/**
 * @brief TestGame::gameFinishEvent
 * @return
 */

bool TestGame::gameFinishEvent()
{
	if (m_closedSuccesfully)
		return false;

	m_closedSuccesfully = true;
	return true;
}

const QVariantMap &TestGame::resultData() const
{
	return m_resultData;
}

void TestGame::setResultData(const QVariantMap &newResultData)
{
	if (m_resultData == newResultData)
		return;
	m_resultData = newResultData;
	emit resultDataChanged();
}

int TestGame::currentQuestion() const
{
	return m_currentQuestion;
}

void TestGame::setCurrentQuestion(int newCurrentQuestion)
{
	if (m_currentQuestion == newCurrentQuestion)
		return;
	m_currentQuestion = newCurrentQuestion;
	emit currentQuestionChanged();
}


/**
 * @brief TestGame::loadCurrentQuestion
 */

void TestGame::loadCurrentQuestion() const
{
	if (m_currentQuestion < 0 || m_currentQuestion >= m_questionList.size())
		return;

	const QuestionData &d = m_questionList.at(m_currentQuestion);

	m_gameQuestion->loadQuestion(d.url, d.data, d.uuid, d.answer);
}




/**
 * @brief TestGame::nextQuestion
 */

void TestGame::nextQuestion()
{
	if (m_currentQuestion < m_questionList.size()-1) {
		setCurrentQuestion(m_currentQuestion+1);
		m_gameQuestion->forceDestroy();
	}
}



/**
 * @brief TestGame::previousQuestion
 */

void TestGame::previousQuestion()
{
	if (m_currentQuestion > 0) {
		setCurrentQuestion(m_currentQuestion-1);
		m_gameQuestion->forceDestroy();
	}
}



/**
 * @brief TestGame::checkAnswers
 */

void TestGame::checkAnswers()
{
	const QuestionResult &r = questionDataResult(m_questionList, m_missionLevel->passed());

	setXp(r.points * (qreal) TEST_GAME_BASE_XP);
	setResultData(r.resultData);

	if (r.success)
		onGameSuccess();
	else
		onGameFailed();
}





/**
 * @brief TestGame::questionDataToVariantMap
 * @param list
 * @return
 */

TestGame::QuestionResult TestGame::questionDataResult(const QVector<QuestionData> &list, const qreal &passed)
{
	QuestionResult r;

	QVariantList l;

	foreach (const QuestionData &q, list) {
		const qreal &point = q.data.value(QStringLiteral("xpFactor"), 1.0).toReal();

		r.maxPoints += point;

		if (q.success)
			r.points += point;

		QVariantMap r;
		r.insert(QStringLiteral("module"), q.module);
		r.insert(QStringLiteral("uuid"), q.uuid);
		r.insert(QStringLiteral("question"), q.data);
		r.insert(QStringLiteral("answer"), q.answer);
		r.insert(QStringLiteral("success"), q.success);

		l.append(r);
	}


	r.success = r.maxPoints > 0 ? (r.points/r.maxPoints) >= passed : true;

	r.resultData = QVariantMap({
								  {QStringLiteral("finished"), true},
								  {QStringLiteral("points"), r.points},
								  {QStringLiteral("maxPoints"), r.maxPoints},
								  {QStringLiteral("success"), r.success},
								  {QStringLiteral("list"), l}
							  });

	QJsonDocument doc(QJsonObject::fromVariantMap(r.resultData));
	LOG_CTRACE("game") << doc.toJson(QJsonDocument::Indented).constData();

	return r;
}



/**
 * @brief TestGame::questions
 * @return
 */

int TestGame::questions() const
{
	return m_questionList.size();
}
