/*
 * ---- Call of Suli ----
 *
 * gamequestion.h
 *
 * Created on: 2020. 11. 03.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GameQuestion
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

#ifndef GAMEQUESTION_H
#define GAMEQUESTION_H

#include <QObject>
#include <QQuickItem>

#include "gameenemy.h"
#include "gameplayer.h"
#include "cosgame.h"
#include "question.h"

class GameQuestion : public QObject
{
	Q_OBJECT

	Q_PROPERTY(CosGame* game READ game CONSTANT)
	Q_PROPERTY(bool canPostpone READ canPostpone WRITE setCanPostpone NOTIFY canPostponeChanged)
	Q_PROPERTY(GameMatch::GameMode mode READ mode CONSTANT)
	Q_PROPERTY(QVariantMap answer READ answer WRITE setAnswer NOTIFY answerChanged)

public:

	enum QuestionState {
		StateInvalid,
		StatePresented,
		StateSucceed,
		StateFailed,
		StatePostponed,
		StateAnswered
	};

	Q_ENUM(QuestionState);

	explicit GameQuestion(CosGame *game, GamePlayer *player, GameEnemy *enemy, QObject *parent = nullptr);
	explicit GameQuestion(CosGame *game, QObject *parent = nullptr);
	virtual ~GameQuestion();

	void run();
	bool runLite(const Question &question);

	const QuestionState &state() const;

	CosGame *game() const;

	bool canPostpone() const;
	void setCanPostpone(bool newCanPostpone);

	GameMatch::GameMode mode() const;

	const QVariantMap &answer() const;
	void setAnswer(const QVariantMap &newAnswer);

public slots:
	QString questionQml() const;
	QVariantMap questionData() const;

	bool postpone();

	void forceDestroy();
	void onSuccess(const qreal &xpFactor);
	void onFailed();
	void onDestroyed();

	void onSuccessLite(const qreal &xpFactor);
	void onFailedLite();

signals:
	void finished();
	void xpGained(const qreal &factor);
	void postponed();

	void canPostponeChanged();
	void answerChanged();

private:
	CosGame *m_game;
	GamePlayer *m_player;
	GameEnemy *m_enemy;
	QQuickItem *m_question;
	Question m_questionData;
	QTime m_elapsedTime;
	QuestionState m_state;
	bool m_canPostpone;
	QVariantMap m_answer;
};

#endif // GAMEQUESTION_H
