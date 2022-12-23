/*
 * ---- Call of Suli ----
 *
 * client.cpp
 *
 * Created on: 2022. 12. 09.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * Client
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

#include <QDebug>

#include "application.h"
#include "client.h"
#include "actiongame.h"
#include "qquickwindow.h"

Q_LOGGING_CATEGORY(lcClient, "app.client")

Client::Client(Application *app, QObject *parent)
	: QObject{parent}
	, m_application(app)
	, m_networkManager(new QNetworkAccessManager(this))
	, m_utils(new Utils(this))
{
	Q_ASSERT(app);
}



/**
 * @brief Client::~Client
 */

Client::~Client()
{
	if (m_currentGame)
		delete m_currentGame;

	delete m_networkManager;
	delete m_utils;
}

/**
 * @brief Client::mainStack
 * @return
 */

QQuickItem *Client::mainStack() const
{
	return m_mainStack;
}

/**
 * @brief Client::setMainStack
 * @param newMainStack
 */

void Client::setMainStack(QQuickItem *newMainStack)
{
	if (m_mainStack == newMainStack)
		return;
	m_mainStack = newMainStack;
	emit mainStackChanged();
}




/**
 * @brief Client::addPage
 */

QQuickItem* Client::stackPushPage(const QString &qml, const QVariantMap &parameters) const
{
	if (!m_mainStack) {
		qCCritical(lcClient).noquote() << tr("mainStack nincsen beállítva!");
		return nullptr;
	}

	if (qml.isEmpty()) {
		qCWarning(lcClient).noquote() << tr("Nincs megadva lap");
		return nullptr;
	}

	QQuickItem *o = nullptr;
	QMetaObject::invokeMethod(m_mainStack, "createPage",
							  Q_RETURN_ARG(QQuickItem*, o),
							  Q_ARG(QString, qml),
							  Q_ARG(QVariant, parameters)
							  );

	if (!o) {
		qCCritical(lcClient).noquote() << tr("Nem lehet a lapot betölteni: %1").arg(qml);
		return nullptr;
	}

	qCDebug(lcClient).noquote() << tr("Lap betöltve:") << qml << o;

	return o;
}






/**
 * @brief Client::stackPop
 * @param depth
 */

bool Client::stackPop(const int &index, const bool &forced) const
{
	if (!m_mainStack) {
		qCCritical(lcClient).noquote() << tr("mainStack nincsen beállítva!");
		return false;
	}

	const int &depth = m_mainStack->property("depth").toInt();

	if (index >= depth) {
		qCWarning(lcClient).noquote() << tr("Nem lehet a #%1 lapra visszalépni (mélység: %2)").arg(index).arg(depth);
		return false;
	}

	if (depth <= 2) {
#if !defined(Q_OS_IOS) && !defined(Q_OS_ANDROID)
		qCDebug(lcClient).noquote() << tr("Nem lehet visszalépni (mélység: %1)").arg(depth);
		//Application::instance()->messageInfo("Nem lehet visszalépni!");
		return false;
#endif

		m_mainWindow->close();
		return true;
	}



	QQuickItem *currentItem = qvariant_cast<QQuickItem*>(m_mainStack->property("currentItem"));

	if (!currentItem) {
		qCCritical(lcClient).noquote() << tr("mainStack currentItem unavailable");
		return false;
	}


	QString closeDisabled = currentItem->property("closeDisabled").toString();
	QString question = currentItem->property("closeQuestion").toString();

	if (!closeDisabled.isEmpty()) {
		messageWarning(closeDisabled);
		return false;
	}


	if (forced || question.isEmpty()) {
		QMetaObject::invokeMethod(m_mainStack, "popPage",
								  Q_ARG(int, index)
								  );

		return true;
	}

	qCDebug(lcClient).noquote() << tr("Kérdés a visszalépés előtt") << currentItem;

	QMetaObject::invokeMethod(m_mainWindow, "closeQuestion",
							  Q_ARG(QString, question),
							  Q_ARG(bool, true),						// _pop
							  Q_ARG(int, index)
							  );

	return false;
}






/**
 * @brief Client::closeWindow
 * @return
 */

bool Client::closeWindow(const bool &forced)
{
	if (m_mainWindowClosable)
		return true;


	QQuickItem *currentItem = qvariant_cast<QQuickItem*>(m_mainStack->property("currentItem"));

	if (!currentItem) {
		qCCritical(lcClient).noquote() << tr("mainStack currentItem unavailable");
		return false;
	}

	QString closeDisabled = currentItem->property("closeDisabled").toString();
	QString question = currentItem->property("closeQuestion").toString();

	if (!closeDisabled.isEmpty()) {
		messageWarning(closeDisabled);
		return false;
	}


	if (forced || question.isEmpty()) {
		qCDebug(lcClient).noquote() << tr("Ablak bezárása");
		m_mainWindowClosable = true;
		m_mainWindow->close();
		return true;
	}

	qCDebug(lcClient).noquote() << tr("Kérdés a bezárás előtt") << currentItem;

	QMetaObject::invokeMethod(m_mainWindow, "closeQuestion",
							  Q_ARG(QString, question),
							  Q_ARG(bool, false),					// _pop
							  Q_ARG(int, -1)						// _index
							  );

	return false;
}





/**
 * @brief Client::onApplicationStarted
 */

void Client::onApplicationStarted()
{
	qCInfo(lcClient).noquote() << tr("A kliens alkalmazás sikeresen elindult.");

	QCoreApplication::processEvents();

	stackPushPage("PageStart.qml");
}




/**
 * @brief Client::_message
 * @param text
 * @param title
 * @param icon
 */

void Client::_message(const QString &text, const QString &title, const QString &icon) const
{
	if (m_mainWindow) {
		QMetaObject::invokeMethod(m_mainWindow, "messageDialog",
								  Q_ARG(QString, text),
								  Q_ARG(QString, title),
								  Q_ARG(QString, icon)
								  );
	}
}

qreal Client::safeMarginBottom() const
{
	return m_safeMarginBottom;
}

void Client::setSafeMarginBottom(qreal newSafeMarginBottom)
{
	if (qFuzzyCompare(m_safeMarginBottom, newSafeMarginBottom))
		return;
	m_safeMarginBottom = newSafeMarginBottom;
	emit safeMarginBottomChanged();
}


/**
 * @brief Client::setSafeMargins
 * @param margins
 */

void Client::setSafeMargins(const QMarginsF &margins)
{
	setSafeMarginLeft(margins.left());
	setSafeMarginRight(margins.right());
	setSafeMarginTop(margins.top());
	setSafeMarginBottom(margins.bottom());
}



qreal Client::safeMarginTop() const
{
	return m_safeMarginTop;
}

void Client::setSafeMarginTop(qreal newSafeMarginTop)
{
	if (qFuzzyCompare(m_safeMarginTop, newSafeMarginTop))
		return;
	m_safeMarginTop = newSafeMarginTop;
	emit safeMarginTopChanged();
}

qreal Client::safeMarginRight() const
{
	return m_safeMarginRight;
}

void Client::setSafeMarginRight(qreal newSafeMarginRight)
{
	if (qFuzzyCompare(m_safeMarginRight, newSafeMarginRight))
		return;
	m_safeMarginRight = newSafeMarginRight;
	emit safeMarginRightChanged();
}

qreal Client::safeMarginLeft() const
{
	return m_safeMarginLeft;
}

void Client::setSafeMarginLeft(qreal newSafeMarginLeft)
{
	if (qFuzzyCompare(m_safeMarginLeft, newSafeMarginLeft))
		return;
	m_safeMarginLeft = newSafeMarginLeft;
	emit safeMarginLeftChanged();
}



/**
 * @brief Client::safeMarginsGet
 */

void Client::safeMarginsGet()
{
	m_utils->safeMarginsGet(this);
}



/**
 * @brief Client::application
 * @return
 */

Application *Client::application() const
{
	return m_application;
}


/**
 * @brief Client::utils
 * @return
 */

Utils *Client::utils() const
{
	return m_utils;
}


/**
 * @brief Client::netoworkManager
 * @return
 */

QNetworkAccessManager *Client::networkManager() const
{
	return m_networkManager;
}






/**
 * @brief Client::mainWindow
 * @return
 */

QQuickWindow *Client::mainWindow() const
{
	return m_mainWindow;
}


/**
 * @brief Client::setMainWindow
 * @param newMainWindow
 */

void Client::setMainWindow(QQuickWindow *newMainWindow)
{
	if (m_mainWindow == newMainWindow)
		return;

	m_mainWindow = newMainWindow;
	emit mainWindowChanged();

	if (!m_mainWindow)
		return;

	m_mainWindow->setIcon(QIcon(":/internal/img/cos.png"));

	safeMarginsGet();
}


AbstractGame *Client::currentGame() const
{
	return m_currentGame;
}


void Client::setCurrentGame(AbstractGame *newCurrentGame)
{
	if (m_currentGame == newCurrentGame)
		return;
	m_currentGame = newCurrentGame;
	emit currentGameChanged();
}


/**
 * @brief Client::messageInfo
 * @param text
 * @param title
 */

void Client::messageInfo(const QString &text, QString title) const
{
	if (title.isEmpty())
		title = m_application->application()->applicationDisplayName();

	qCInfo(lcClient).noquote() << QString("%1 (%2)").arg(text, title);
	_message(text, title, "qrc:/Qaterial/Icons/alert-circle-outline.svg");
}


/**
 * @brief Client::messageWarning
 * @param text
 * @param title
 */

void Client::messageWarning(const QString &text, QString title) const
{
	if (title.isEmpty())
		title = m_application->application()->applicationDisplayName();

	qCWarning(lcClient).noquote() << QString("%1 (%2)").arg(text, title);
	_message(text, title, "qrc:/Qaterial/Icons/alert.svg");
}


/**
 * @brief Client::messageError
 * @param text
 * @param title
 */

void Client::messageError(const QString &text, QString title) const
{
	if (title.isEmpty())
		title = m_application->application()->applicationDisplayName();

	qCCritical(lcClient).noquote() << QString("%1 (%2)").arg(text, title);
	_message(text, title, "qrc:/Qaterial/Icons/alert-octagon.svg");
}



/**
 * @brief Client::loadGame
 */

void Client::loadGame()
{
	if (m_currentGame) {
		qCCritical(lcClient).noquote() << tr("Game already exists");
		return;
	}

	GameMap *map = GameMap::fromBinaryData(Utils::fileContent("/home/valaczka/demo_test.map"));

	if (!map)
		return;

	GameMapMissionLevel *ml = map->missionLevel("{90cb2799-f189-4255-9667-1f19b582284b}", 1);

	ActionGame *game = new ActionGame(ml, this);
	setCurrentGame(game);

	game->load();
}


/**
 * @brief Client::debug
 * @return
 */

bool Client::debug() const
{
	return m_application->debug();
}



