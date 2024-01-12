/*
 * ---- Call of Suli ----
 *
 * testgame.h
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

#ifndef TESTGAME_H
#define TESTGAME_H

#include "abstractlevelgame.h"
#include "qquicktextdocument.h"
#include "qtextdocument.h"

#define TEST_GAME_BASE_XP		10

class TestGame : public AbstractLevelGame
{
	Q_OBJECT

	Q_PROPERTY(int questions READ questions NOTIFY questionsChanged)
	Q_PROPERTY(int currentQuestion READ currentQuestion WRITE setCurrentQuestion NOTIFY currentQuestionChanged)
	Q_PROPERTY(QuestionResult result READ result WRITE setResult NOTIFY resultChanged)
	Q_PROPERTY(bool hasResult READ hasResult WRITE setHasResult NOTIFY hasResultChanged)

public:
	TestGame(GameMapMissionLevel *missionLevel, Client *client);
	virtual ~TestGame();

	int questions() const;

	int currentQuestion() const;
	void setCurrentQuestion(int newCurrentQuestion);

	void loadCurrentQuestion() const;

	Q_INVOKABLE void nextQuestion();
	Q_INVOKABLE void previousQuestion();
	Q_INVOKABLE void finishGame();

	void checkAnswers();


	/**
	 * @brief The QuestionData class
	 */

	struct QuestionData {
		QUrl url;
		QVariantMap data;
		QString uuid;
		QVariantMap answer;
		QString module;
		bool success = false;
		int examPoint = 0;
	};


	/**
	 * @brief The QuestionResult class
	 */

	struct QuestionResult {
		QVector<QuestionData> resultData;
		qreal points = 0;
		qreal maxPoints = 0;
		bool success = false;
		bool isExam = false;
	};

	static QuestionResult questionDataResult(const QVector<QuestionData> &list, const qreal &passed = 1.0);
	QuestionResult questionDataResult(const qreal &passed = 1.0) { return questionDataResult(m_questionList, passed); }

	static QString questionDataResultToHtml(const QString &header, const QuestionResult &result);
	QString questionDataResultToHtml(const QuestionResult &result) const;

	static QString questionDataResultToHtml(const QuestionData &data);

	static const QString CheckOK;
	static const QString CheckFailed;

	const QuestionResult &result() const;
	void setResult(const QuestionResult &newResult);

	Q_INVOKABLE void resultToTextDocument(QTextDocument *document) const;
	Q_INVOKABLE void resultToQuickTextDocument(QQuickTextDocument *document) const;

	bool hasResult() const;
	void setHasResult(bool newHasResult);

public slots:
	void onPageReady();
	void onStarted();
	void dialogMessageFinish(const QString &text, const QString &icon, const bool &success);
	void gameAbort() override;

private slots:
	void onMsecLeftChanged();
	void onGameQuestionFinished();
	void onGameQuestionSuccess(const QVariantMap &answer);
	void onGameQuestionFailed(const QVariantMap &answer);
	void onGameTimeout();
	void onGameSuccess();
	void onGameFailed();

signals:
	void timeNotify();
	void questionsChanged();
	void currentQuestionChanged();
	void resultChanged();
	void hasResultChanged();

protected:
	virtual QQuickItem *loadPage() override;
	virtual void connectGameQuestion() override;
	virtual bool gameFinishEvent() override;

private:
	int m_timeNotifySendNext = 60000;
	QVector<QuestionData> m_questionList;
	int m_currentQuestion = -1;
	QuestionResult m_result;
	bool m_hasResult = false;
};

#endif // TESTGAME_H
