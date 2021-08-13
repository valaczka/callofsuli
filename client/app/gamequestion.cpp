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
	, m_questionData()
{
}


/**
 * @brief GameQuestion::~GameQuestion
 */

GameQuestion::~GameQuestion()
{
}


/**
 * @brief GameQuestion::run
 */

void GameQuestion::run()
{
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

	if (enemyData->targetId() == -1 || enemyData->targetDestroyed()) {
		qDebug() << "Empty question";
		m_enemy->killByPlayer(m_player);
		emit finished();
		return;
	}

	if (!m_game->gameScene()) {
		qWarning() << "Missing scene";
		emit finished();
		return;
	}

	QQuickItem *scene = m_game->gameScene();
	m_question = nullptr;

	m_questionData = enemyData->generateQuestion();

	if (!m_questionData.isValid()) {
		qWarning() << "Invalid question";
		emit finished();
		return;
	}


	if (!m_questionData.generate()) {
		qWarning() << "Generate() error";
		emit finished();
		return;
	}


	QMetaObject::invokeMethod(scene, "createQuestion", Qt::DirectConnection,
							  Q_RETURN_ARG(QQuickItem*, m_question),
							  Q_ARG(GameQuestion*, this)
							  );


	if (m_question) {
		m_game->setRunning(false);

		connect(m_question, &QQuickItem::destroyed, this, &GameQuestion::onDestroyed);
		connect(m_question, SIGNAL(succeed(qreal)), this, SLOT(onSuccess(qreal)));
		connect(m_question, SIGNAL(failed()), this, SLOT(onFailed()));

		scene->setFocus(false, Qt::OtherFocusReason);
		m_question->setFocus(true, Qt::OtherFocusReason);
	} else {
		qWarning() << "Can't create question";
		emit finished();
		return;
	}

}



/**
 * @brief GameQuestion::questionQml
 * @return
 */

QString GameQuestion::questionQml() const
{
	return m_questionData.qml();
}


/**
 * @brief GameQuestion::questionData
 * @return
 */


QVariantMap GameQuestion::questionData() const
{
	return m_questionData.question();
}


/**
 * @brief GameQuestion::answerData
 * @return
 */

QVariantMap GameQuestion::answerData() const
{
	return m_questionData.answer();
}


/**
 * @brief GameQuestion::forceDestroy
 */

void GameQuestion::forceDestroy()
{
	qDebug() << "FORCE DESTROY" << this;

	if (m_question) {
		m_question->setFocus(false, Qt::OtherFocusReason);
		m_question->deleteLater();
	}

	m_game->gameScene()->setFocus(true, Qt::OtherFocusReason);
	m_game->setRunning(true);

	emit finished();
}


/**
 * @brief GameQuestion::onSuccess
 */

void GameQuestion::onSuccess(const qreal &xpFactor)
{
	emit xpGained(xpFactor);

	m_enemy->setXpGained(true);
	m_enemy->enemyData()->setTargetDestroyed(true);

	m_question->setFocus(false, Qt::OtherFocusReason);
	m_game->gameScene()->setFocus(true, Qt::OtherFocusReason);

	m_enemy->killByPlayer(m_player);
}


/**
 * @brief GameQuestion::onFailed
 */

void GameQuestion::onFailed()
{
	m_question->setFocus(false, Qt::OtherFocusReason);
	m_game->gameScene()->setFocus(true, Qt::OtherFocusReason);

	m_enemy->missedByPlayer(m_player);
}


/**
 * @brief GameQuestion::onDestroyed
 */

void GameQuestion::onDestroyed()
{
	m_question = nullptr;

	m_game->setRunning(true);
	emit finished();
}
