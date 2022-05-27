/*
 * ---- Call of Suli ----
 *
 * gameactivity.h
 *
 * Created on: 2020. 12. 23.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GameActivity
 *
 *  This file is part of Call of Suli.
 *
 *  Call of Suli is free software: you can redistribute it and/or modify
 *  it under the terms of the MIT license.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef GAMEACTIVITY_H
#define GAMEACTIVITY_H

#include "abstractactivity.h"
#include "gameenemy.h"
#include "gameenemydata.h"
#include "gamequestion.h"
#include "question.h"
#include <QObject>

class CosGame;

class GameActivity : public AbstractActivity
{
	Q_OBJECT

	Q_PROPERTY(bool prepared READ prepared WRITE setPrepared NOTIFY preparedChanged)
	Q_PROPERTY(CosGame * game READ game WRITE setGame NOTIFY gameChanged)
	Q_PROPERTY(int currentQuestion READ currentQuestion WRITE setCurrentQuestion NOTIFY currentQuestionChanged)
	Q_PROPERTY(int postponedQuestions READ postponedQuestions WRITE setPostponedQuestions NOTIFY postponedQuestionsChanged)


public:
	struct GameActivityQuestion;

	GameActivity(QQuickItem *parent = nullptr);
	virtual ~GameActivity();

	Q_INVOKABLE void prepare();
	Q_INVOKABLE void createPickable(GameEnemy *enemy);

	void createTarget(GameEnemy *enemy);
	bool createTargets(QVector<GameEnemy *> enemies);

	bool prepared() const { return m_prepared; }
	CosGame * game() const { return m_game; }

	const QVector<GameActivityQuestion> &questionList() const;

	int currentQuestion() const;
	void setCurrentQuestion(int newCurrentQuestion);
	int setNextQuestion();
	int repeatCurrentQuestion();
	int postponeCurrentQuestion();
	void setCurrentQuestionAnswer(const QVariantMap &answer);

	bool generateQuestion(GameQuestion *gameQuestion);
	int activeQuestions() const;

	int postponedQuestions() const;
	void setPostponedQuestions(int newPostponedQuestions);

public slots:
	void onEnemyKilled(GameEnemy *enemy);
	void onEnemyKillMissed(GameEnemy *enemy);
	void setPrepared(bool prepared);
	void setGame(CosGame * game);


signals:
	void prepareFailed();
	void prepareSucceed();

	void questionFailed();

	void preparedChanged(bool prepared);
	void gameChanged(CosGame * game);
	void currentQuestionChanged();

	void postponedQuestionsChanged();

protected slots:
	//void clientSetup() override;
	//void onMessageReceived(const CosMessage &message) override;
	//void onMessageFrameReceived(const CosMessage &message) override;

private slots:
	void prepareDb(QVariantMap = QVariantMap());
	void prepareLite();

private:
	bool m_prepared;
	CosGame * m_game;
	QVector<GameActivityQuestion> m_questionList;
	int m_currentQuestion;
	int m_postponedQuestions;
};






/**
 * @brief The GameActivity::GameActivityQuestion struct
 */

struct GameActivity::GameActivityQuestion
{
	Question question;
	QVariantMap answer;

	GameActivityQuestion(Question q, QVariantMap a = QVariantMap())
		: question(q), answer(a)
	{}
};

#endif // GAMEACTIVITY_H
