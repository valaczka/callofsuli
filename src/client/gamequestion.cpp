/*
 * ---- Call of Suli ----
 *
 * gamequestion.cpp
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

#include "gamequestion.h"
#include "gameenemydata.h"

GameQuestion::GameQuestion(CosGame *game, GamePlayer *player, GameEnemy *enemy, QObject *parent)
	: QObject(parent)
	, m_game(game)
	, m_player(player)
	, m_enemy(enemy)
	, m_question(nullptr)
{
	qDebug() << "CREATE" << this;
}


/**
 * @brief GameQuestion::~GameQuestion
 */

GameQuestion::~GameQuestion()
{
	qDebug() << "DESTROY" << this;
}


/**
 * @brief GameQuestion::run
 */

void GameQuestion::run()
{
	qDebug() << "RUN" << this;

	if (!m_game || !m_player || !m_enemy || m_question) {
		qWarning() << "Invalid game or player or enemy or question already run";
		emit finished();
		return;
	}


	GameEnemyData *enemyData = m_enemy->enemyData();

	if (!enemyData) {
		qWarning() << "Invalid enemy data" << m_enemy;
		emit finished();
		return;
	}

	if (enemyData->targetId() == -1) {
		qDebug() << "Empty question";
		m_enemy->killByPlayer(m_player);
		emit finished();
		return;
	}

	if (!m_game->gameScene())
		return;

	QQuickItem *scene = m_game->gameScene();
	m_question = nullptr;

	QMetaObject::invokeMethod(scene, "createQuestion", Qt::DirectConnection,
							  Q_RETURN_ARG(QQuickItem *, m_question),
							  Q_ARG(QVariant, enemyData->generateQuestion()));


	if (m_question) {
		m_game->setRunning(false);
		/*m_question->setProperty("maximumWidth", 400);
		m_question->setProperty("maximumHeight", 400);*/

		connect(m_question, &QQuickItem::destroyed, this, &GameQuestion::onDestroyed);
		connect(m_question, SIGNAL(succeed()), this, SLOT(onSuccess()));
		connect(m_question, SIGNAL(failed()), this, SLOT(onFailed()));

		scene->setFocus(false, Qt::OtherFocusReason);
		m_question->setFocus(true, Qt::OtherFocusReason);

		qDebug() << "=================== CrEAteED" << m_question;
	} else {
		qDebug() << "??????????????";
		emit finished();
		return;
	}

}


/**
 * @brief GameQuestion::onSuccess
 */

void GameQuestion::onSuccess()
{
	qDebug() << "ON SUCCESS" << this;

	m_question->setFocus(false, Qt::OtherFocusReason);
	m_game->gameScene()->setFocus(true, Qt::OtherFocusReason);
	m_game->setRunning(true);

	m_enemy->killByPlayer(m_player);

	m_question->deleteLater();
}


/**
 * @brief GameQuestion::onFailed
 */

void GameQuestion::onFailed()
{
	qDebug() << "ON FAILED" << this;

	m_question->setFocus(false, Qt::OtherFocusReason);
	m_game->gameScene()->setFocus(true, Qt::OtherFocusReason);
	m_game->setRunning(true);

	m_enemy->missedByPlayer(m_player);

	m_question->deleteLater();
}


/**
 * @brief GameQuestion::onDestroyed
 */

void GameQuestion::onDestroyed()
{
	qDebug() << "ON DESTROYED" << m_question;
	m_question = nullptr;

	emit finished();
}
