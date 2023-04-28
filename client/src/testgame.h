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

class TestGame : public AbstractLevelGame
{
	Q_OBJECT

	Q_PROPERTY(int questions READ questions NOTIFY questionsChanged)
	Q_PROPERTY(int currentQuestion READ currentQuestion WRITE setCurrentQuestion NOTIFY currentQuestionChanged)

public:
	TestGame(GameMapMissionLevel *missionLevel, Client *client);
	virtual ~TestGame();

	int questions() const;

	int currentQuestion() const;
	void setCurrentQuestion(int newCurrentQuestion);

	void loadCurrentQuestion() const;

	Q_INVOKABLE void nextQuestion();
	Q_INVOKABLE void previousQuestion();

	/**
	 * @brief The QuestionData class
	 */

	struct QuestionData {
		QUrl url;
		QVariantMap data;
		QString uuid;
		QVariantMap answer;
		bool success = false;
	};

public slots:
	void onPageReady();
	void onStarted();
	void dialogMessageFinish(const QString &text, const QString &icon, const bool &success);
	void gameAbort();

private slots:
	void onMsecLeftChanged();
	void onGameQuestionFinished();
	void onGameQuestionSuccess(const QVariantMap &answer);
	void onGameQuestionFailed(const QVariantMap &answer);
	void onGameTimeout();
	void onGameSuccess();

signals:
	void timeNotify();
	void questionsChanged();
	void currentQuestionChanged();

protected:
	virtual QQuickItem *loadPage() override;
	virtual void connectGameQuestion() override;
	virtual bool gameFinishEvent() override;

private:
	int m_timeNotifySendNext = 60000;
	QVector<QuestionData> m_questionList;
	int m_currentQuestion = -1;
};

#endif // TESTGAME_H
