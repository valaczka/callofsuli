/*
 * ---- Call of Suli ----
 *
 * litegame.h
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

#ifndef LITEGAME_H
#define LITEGAME_H

#include "abstractlevelgame.h"

#define LITE_GAME_BASE_XP		10
#define SECOND_PER_QUESTION		12


class LiteQuestion;

/**
 * @brief The LiteGame class
 */

class LiteGame : public AbstractLevelGame
{
	Q_OBJECT

	Q_PROPERTY(int hp READ hp WRITE setHp NOTIFY hpChanged)
	Q_PROPERTY(int questions READ questions NOTIFY questionsChanged)

public:
	LiteGame(GameMapMissionLevel *missionLevel, Client *client);
	virtual ~LiteGame();

	int questions() const;

	int hp() const;
	void setHp(int newHp);

public slots:
	void onPageReady();
	void onStarted();
	void dialogMessageFinish(const QString &text, const QString &icon, const bool &success);
	void gameAbort();
	bool nextQuestion();

private slots:
	void onMsecLeftChanged();
	void onGameQuestionSuccess(const QVariantMap &answer);
	void onGameQuestionFailed(const QVariantMap &answer);
	void onGameQuestionFinished();
	void onGameTimeout();
	void onGameSuccess();

signals:
	void timeNotify();
	void questionsChanged();
	void hpChanged();

protected:
	virtual QQuickItem *loadPage() override;
	virtual void connectGameQuestion() override;
	virtual bool gameFinishEvent() override;

private:
	int m_timeNotifySendNext = 60000;
	QVector<LiteQuestion> m_questions;
	QVector<int> m_indices;
	int m_hp = 0;
};


/**
 * @brief The LiteQuestion class
 */

class LiteQuestion
{
public:
	LiteQuestion(const Question &question) : m_question(question) {}

	const Question &question() const { return m_question; }
	const int &used() const { return m_used; }
	const LiteQuestion &increaseUsed() { ++m_used; return *this; }

private:
	Question m_question;
	int m_used = 0;
};

#endif // LITEGAME_H
