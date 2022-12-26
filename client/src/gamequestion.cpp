/*
 * ---- Call of Suli ----
 *
 * gamequestion.cpp
 *
 * Created on: 2022. 12. 25.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GameQuestion
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

#include "gamequestion.h"
#include "abstractgame.h"
#include "application.h"


GameQuestion::GameQuestion(QQuickItem *parent)
	: QQuickItem(parent)
{
	qCDebug(lcGame).noquote() << tr("Game question item created");

	setEnabled(false);
	setVisible(false);
}

GameQuestion::~GameQuestion()
{
	if (m_game && m_game->gameQuestion() == this)
		m_game->setGameQuestion(nullptr);

	qCDebug(lcGame).noquote() << tr("Game question item destroyed");
}


/**
 * @brief GameQuestion::loadQuestion
 * @param componentUrl
 * @param data
 */

void GameQuestion::loadQuestion(const QUrl &componentUrl, const QVariantMap &data, const QString &uuid)
{
	if (m_questionComponent) {
		Application::instance()->messageError(tr("Már folyamatban van egy kérdés!"), tr("Belső hiba"));
		emit questionLoadFailed();
		return;
	}

	if (!m_loader) {
		qCCritical(lcGame).noquote() << tr("Missing loader");
		emit questionLoadFailed();
		return;
	}

	setObjectiveUuid(uuid);
	setQuestionData(data);

	m_loader->setProperty("source", componentUrl);

	/*QMetaObject::invokeMethod(this, "loadComponent",
							  Q_ARG(QVariant, componentUrl),
							  Q_ARG(QVariant, data)
							  );*/

}


/**
 * @brief GameQuestion::loadQuestion
 * @param question
 */

void GameQuestion::loadQuestion(const Question &question)
{
	loadQuestion(QStringLiteral("qrc:/")+question.qml(), question.generate(), question.uuid());
}



/**
 * @brief GameQuestion::onShowAnimationFinished
 */

void GameQuestion::onShowAnimationFinished()
{
	qCDebug(lcGame).noquote() << tr("Game question show animation finished");

	setEnabled(true);
	m_questionComponent->forceActiveFocus(Qt::OtherFocusReason);

	m_time = QTime::currentTime();
	m_elapsedMsec = -1;

	emit started();
}


/**
 * @brief GameQuestion::onHideAnimationFinished
 */

void GameQuestion::onHideAnimationFinished()
{
	qCDebug(lcGame).noquote() << tr("Game question hide animation finished");

	if (m_loader)
		m_loader->setProperty("source", QUrl(QLatin1String("")));

	setVisible(false);
	setObjectiveUuid(QLatin1String(""));
	setQuestionComponent(nullptr);

	emit finished();
}


/**
 * @brief GameQuestion::onLoaderLoaded
 * @param item
 */

void GameQuestion::onLoaderLoaded(QQuickItem *item)
{
	qCDebug(lcGame).noquote() << tr("Game question loader loaded:") << item;

	GameQuestionComponent *component = qobject_cast<GameQuestionComponent*>(item);

	if (!component) {
		Application::instance()->messageError(tr("Érvénytelen kérdés!"), tr("Belső hiba"));
		emit questionLoadFailed();
		setObjectiveUuid(QLatin1String(""));
		return;
	}

	component->setQuestion(this);

	setQuestionComponent(component);

	setVisible(true);
	setState(QStringLiteral("started"));
}


/**
 * @brief GameQuestion::answerReveal
 * @param answer
 */

void GameQuestion::answerReveal(const QVariantMap &answer)
{
	if (m_questionComponent)
		emit m_questionComponent->answerReveal(answer);
}


/**
 * @brief GameQuestion::finish
 */

void GameQuestion::finish()
{
	setState(QStringLiteral("finished"));
}


/**
 * @brief GameQuestion::forceDestroy
 */

void GameQuestion::forceDestroy()
{
	qCDebug(lcGame).noquote() << tr("Game question force destroy");

	setEnabled(false);
	setMsecBeforeHide(0);
	finish();
}


/**
 * @brief GameQuestion::postpone
 */

void GameQuestion::onPostpone()
{
	setEnabled(false);
	m_elapsedMsec = m_time.msecsTo(QTime::currentTime());
	qCDebug(lcGame).noquote() << tr("Question postponed after %1 milliseconds:").arg(m_elapsedMsec);
	emit postponed();
}


/**
 * @brief GameQuestion::onSuccess
 * @param answer
 */

void GameQuestion::onSuccess(const QVariantMap &answer)
{
	setEnabled(false);
	m_elapsedMsec = m_time.msecsTo(QTime::currentTime());
	qCDebug(lcGame).noquote() << tr("Question successfully answered in %1 milliseconds:").arg(m_elapsedMsec) << answer;
	emit success(answer);
}


/**
 * @brief GameQuestion::onFailed
 * @param answer
 */

void GameQuestion::onFailed(const QVariantMap &answer)
{
	setEnabled(false);
	m_elapsedMsec = m_time.msecsTo(QTime::currentTime());
	qCDebug(lcGame).noquote() << tr("Question answer failed in %1 milliseconds:").arg(m_elapsedMsec) << answer;
	emit failed(answer);
}



/**
 * @brief GameQuestion::questionData
 * @return
 */

const QVariantMap &GameQuestion::questionData() const
{
	return m_questionData;
}

void GameQuestion::setQuestionData(const QVariantMap &newQuestionData)
{
	if (m_questionData == newQuestionData)
		return;
	m_questionData = newQuestionData;
	emit questionDataChanged();
}

const QString &GameQuestion::objectiveUuid() const
{
	return m_objectiveUuid;
}

void GameQuestion::setObjectiveUuid(const QString &newObjectiveUuid)
{
	if (m_objectiveUuid == newObjectiveUuid)
		return;
	m_objectiveUuid = newObjectiveUuid;
	emit objectiveUuidChanged();
}


/**
 * @brief GameQuestion::elapsedMsec
 * @return
 */

int GameQuestion::elapsedMsec() const
{
	return m_elapsedMsec;
}


/**
 * @brief GameQuestion::loader
 * @return
 */

QQuickItem *GameQuestion::loader() const
{
	return m_loader;
}

void GameQuestion::setLoader(QQuickItem *newLoader)
{
	if (m_loader == newLoader)
		return;
	m_loader = newLoader;
	emit loaderChanged();
}


/**
 * @brief GameQuestion::questionComponent
 * @return
 */

GameQuestionComponent *GameQuestion::questionComponent() const
{
	return m_questionComponent;
}

void GameQuestion::setQuestionComponent(GameQuestionComponent *newQuestionComponent)
{
	if (m_questionComponent == newQuestionComponent)
		return;
	m_questionComponent = newQuestionComponent;
	emit questionComponentChanged();
}




/**
 * @brief GameQuestion::game
 * @return
 */

AbstractGame *GameQuestion::game() const
{
	return m_game;
}


/**
 * @brief GameQuestion::setGame
 * @param newGame
 */

void GameQuestion::setGame(AbstractGame *newGame)
{
	if (m_game == newGame)
		return;
	m_game = newGame;
	emit gameChanged();
	emit gameModeChanged();
	emit toggleModeChanged();

	if (m_game)
		m_game->setGameQuestion(this);
}


/**
 * @brief GameQuestion::msecBeforeHide
 * @return
 */

int GameQuestion::msecBeforeHide() const
{
	return m_msecBeforeHide;
}

void GameQuestion::setMsecBeforeHide(int newMsecBeforeHide)
{
	if (m_msecBeforeHide == newMsecBeforeHide)
		return;
	m_msecBeforeHide = newMsecBeforeHide;
	emit msecBeforeHideChanged();
}


/**
 * @brief GameQuestion::postponeEnabled
 * @return
 */

bool GameQuestion::postponeEnabled() const
{
	return m_postponeEnabled;
}

void GameQuestion::setPostponeEnabled(bool newPostponeEnabled)
{
	if (m_postponeEnabled == newPostponeEnabled)
		return;
	m_postponeEnabled = newPostponeEnabled;
	emit postponeEnabledChanged();
}


/**
 * @brief GameQuestion::gameMode
 * @return
 */

AbstractGame::Mode GameQuestion::gameMode() const
{
	if (m_game)
		return m_game->mode();
	else
		return AbstractGame::Invalid;
}



/**
 * @brief GameQuestion::toggleMode
 * @return
 */

bool GameQuestion::toggleMode() const
{
	return (gameMode() == AbstractGame::Lite) ? true : false;
}
