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
#include "application.h"
#include <QRandomGenerator>
#include "utils_.h"


const QString TestGame::CheckOK = QStringLiteral("<span class=\"checkOK\">\ue5ca</span>");
const QString TestGame::CheckFailed = QStringLiteral("<span class=\"checkFail\">\ue5cd</span>");


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
		pageItem()->setProperty("closeDisabled", QStringLiteral(""));
		pageItem()->setProperty("onPageClose", QVariant::Invalid);
		pageItem()->setProperty("closeQuestion", QStringLiteral(""));

		unloadPageItem();

		emit gameFinished(AbstractGame::Fail);
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

	pageItem()->setProperty("closeDisabled", QStringLiteral(""));

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
		m_gameQuestion->loadQuestion(QStringLiteral(""), QStringLiteral("qrc:/GameQuestionTestFinishComponent.qml"), QVariantMap());
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

bool TestGame::hasResult() const
{
	return m_hasResult;
}

void TestGame::setHasResult(bool newHasResult)
{
	if (m_hasResult == newHasResult)
		return;
	m_hasResult = newHasResult;
	emit hasResultChanged();
}

const TestGame::QuestionResult &TestGame::result() const
{
	return m_result;
}

void TestGame::setResult(const QuestionResult &newResult)
{
	m_result = newResult;
	emit resultChanged();
}


/**
 * @brief TestGame::resultoToTextDocument
 * @param document
 */

void TestGame::resultoToTextDocument(QTextDocument *document) const
{
	if (!document) {
		LOG_CERROR("game") << "Missing QTextDocument";
		return;
	}

	QFont font(QStringLiteral("Rajdhani"), 14);

	document->setDefaultFont(font);
	document->setDefaultStyleSheet(Utils::fileContent(QStringLiteral(":/gametest.css")).value_or(QByteArrayLiteral("")));


	document->setHtml(questionDataResultToHtml(m_result));
	/*
	QPdfWriter pdf("/tmp/out.pdf");
	pdf.setCreator("Valaczka János Pál");
	//pdf.setPageLayout(QPageLayout(QPageSize(QPageSize::A4), QPageLayout::Portrait, QMarginsF(125, 125, 125, 125), QPageLayout::Millimeter));
	pdf.setTitle("Ez a címe");
	pdf.setCreator("Call of Suli");

	LOG_CTRACE("client") << "RES" << pdf.resolution();

	doc->print(&pdf);*/

}


/**
 * @brief TestGame::resultoToQuickTextDocument
 * @param document
 */

void TestGame::resultoToQuickTextDocument(QQuickTextDocument *document) const
{
	if (!document) {
		LOG_CERROR("game") << "Missing QQuickTextDocument";
		return;
	}

	resultoToTextDocument(document->textDocument());
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

	m_gameQuestion->loadQuestion(d.module, d.url, d.data, d.uuid, d.answer);
}




/**
 * @brief TestGame::nextQuestion
 */

void TestGame::nextQuestion()
{
	if (m_currentQuestion < m_questionList.size()-1) {
		setCurrentQuestion(m_currentQuestion+1);
		m_gameQuestion->forceDestroy();
	} else {
		finishGame();
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
 * @brief TestGame::finishGame
 */

void TestGame::finishGame()
{
	setCurrentQuestion(m_questionList.size());
	m_gameQuestion->forceDestroy();
}



/**
 * @brief TestGame::checkAnswers
 */

void TestGame::checkAnswers()
{
	const QuestionResult &r = questionDataResult(m_questionList, m_missionLevel->passed());

	setXp(r.points * (qreal) TEST_GAME_BASE_XP);
	setResult(r);
	setHasResult(true);

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

	r.resultData = list;

	foreach (const QuestionData &q, list) {
		const qreal &point = q.data.value(QStringLiteral("xpFactor"), 1.0).toReal();

		r.maxPoints += point;

		if (q.success)
			r.points += point;

		/*QJsonDocument doc(QJsonObject::fromVariantMap(q.data));
		LOG_CTRACE("game") << "Q" << q.module << doc.toJson(QJsonDocument::Indented).constData();
		QJsonDocument doc2(QJsonObject::fromVariantMap(q.answer));
		LOG_CTRACE("game") << "A" << doc2.toJson(QJsonDocument::Indented).constData();*/
	}

	r.success = r.maxPoints > 0 ? (r.points/r.maxPoints) >= passed : true;

	return r;
}



/**
 * @brief TestGame::questionDataResutlToHtml
 * @return
 */

QString TestGame::questionDataResultToHtml(const TestGame *game, const QuestionResult &result)
{
	Q_ASSERT(game);

	QString html = QStringLiteral("<html><body>");


	// Title

	html += QStringLiteral("<h1>%1</h1>").arg(game->name());


	// Result

	const qreal &percent = result.maxPoints > 0 ? result.points/result.maxPoints : 0;

	if (result.success)
		html += QStringLiteral("<p class=\"resultSuccess\">%1 (%2%)</p>").arg(tr("Sikeres megoldás")).arg(qFloor(percent*100));
	else
		html += QStringLiteral("<p class=\"resultFail\">%1 (%2%)</p>").arg(tr("Sikertelen megoldás")).arg(qFloor(percent*100));

	// Questions

	html += QStringLiteral("<table class=\"questions\" width=\"100%\">");

	int num = 1;

	foreach (const QuestionData &q, result.resultData) {
		const int &point = qFloor(q.data.value(QStringLiteral("xpFactor"), 1.0).toReal() * TEST_GAME_BASE_XP);
		const QString &question = q.data.value(QStringLiteral("question")).toString();

		html += QStringLiteral("<tr class=\"questionTitle\">");
		html += QStringLiteral("<td class=\"questionNum\" align=right valign=top>%1.</td>").arg(num++);
		html += QStringLiteral("<td class=\"question\" width=\"100%\" align=left valign=top>%1</td>").arg(question);
		if (q.success)
			html += QStringLiteral("<td class=\"questionPoint\" align=right valign=top><span class=\"pointSuccess\">%1</span>/%2</td>").arg(point).arg(point);
		else
			html += QStringLiteral("<td class=\"questionPoint\" align=right valign=top><span class=\"pointFail\">0</span>/%1</td>").arg(point);
		html += QStringLiteral("</tr>");


		// Answer

		html += QStringLiteral("<tr class=\"answer\">");
		html += QStringLiteral("<td></td>");
		html += QStringLiteral("<td class=\"answer\">");

		if (!Application::instance()->objectiveModules().contains(q.module)) {
			html += QStringLiteral("<p>%1</p>").arg(QString::fromUtf8(QJsonDocument(QJsonObject::fromVariantMap(q.data))
																					   .toJson(QJsonDocument::Indented)));
			html += QStringLiteral("<p class=\"answer\">%1</p>").arg(QString::fromUtf8(QJsonDocument(QJsonObject::fromVariantMap(q.answer))
																					   .toJson(QJsonDocument::Indented)));

		} else {

			ModuleInterface *mi = Application::instance()->objectiveModules().value(q.module);

			html += mi->testResult(q.data, q.answer, q.success);

		}

		html += QStringLiteral("</td>");
		html += QStringLiteral("<td class=\"check\" align=right valign=middle>%1</td>").arg(q.success ? CheckOK : CheckFailed);
		html += QStringLiteral("</tr>");

	}

	html += QStringLiteral("</table>");
	html += QStringLiteral("</body></html>");


	return html;
}



/**
 * @brief TestGame::questions
 * @return
 */

int TestGame::questions() const
{
	return m_questionList.size();
}

