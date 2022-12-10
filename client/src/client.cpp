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

#include "client.h"
#include "qquickwindow.h"

Q_LOGGING_CATEGORY(lcClient, "app.client")

Client::Client(QObject *parent)
	: QObject{parent}
{

}


/**
 * @brief Client::~Client
 */

Client::~Client()
{

}


/**
 * @brief Client::pixelSize
 * @return
 */

qreal Client::pixelSize() const
{
	return m_pixelSize;
}


/**
 * @brief Client::setPixelSize
 * @param newPixelSize
 */

void Client::setPixelSize(qreal newPixelSize)
{
	if (newPixelSize < m_defaultPixelSize/3 || newPixelSize > m_defaultPixelSize*3)
		return;

	if (qFuzzyCompare(m_pixelSize, newPixelSize))
		return;
	m_pixelSize = newPixelSize;
	emit pixelSizeChanged();
	emit pixelSizeRatioChanged();
}


/**
 * @brief Client::pixelSizeRatio
 * @return
 */

qreal Client::pixelSizeRatio() const
{
	return m_pixelSize/m_defaultPixelSize;
}


/**
 * @brief Client::setPixelSizeRatio
 * @param newPixelSizeRatio
 */

void Client::setPixelSizeRatio(qreal newPixelSizeRatio)
{
	if (newPixelSizeRatio < (1.0/3.0) || newPixelSizeRatio > 3.0)
		return;

	m_pixelSize = m_defaultPixelSize*newPixelSizeRatio;
	emit pixelSizeChanged();
	emit pixelSizeRatioChanged();
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

void Client::stackPushPage(const QString &qml, const QVariantMap &parameters) const
{
	if (!m_mainStack) {
		qCCritical(lcClient).noquote() << tr("mainStack nincsen beállítva!");
		return;
	}

	if (qml.isEmpty()) {
		qCWarning(lcClient).noquote() << tr("Nincs megadva lap");
		return;
	}

	int ret = -1;
	QMetaObject::invokeMethod(m_mainStack, "createPage",
							  Q_RETURN_ARG(int, ret),
							  Q_ARG(QString, qml),
							  Q_ARG(QVariant, parameters)
							  );

	if (ret == -1) {
		qCCritical(lcClient).noquote() << tr("Nem lehet a lapot betölteni: %1").arg(qml);
		return;
	}

	qCDebug(lcClient()).noquote() << tr("Lap betöltve: %1 (%2)").arg(qml).arg(ret);
}





/**
 * @brief Client::stackPop
 * @param depth
 */

void Client::stackPop(const int &index) const
{
	if (!m_mainStack) {
		qCCritical(lcClient).noquote() << tr("mainStack nincsen beállítva!");
		return;
	}

	int ret = -1;
	QMetaObject::invokeMethod(m_mainStack, "popPage",
							  Q_RETURN_ARG(int, ret),
							  Q_ARG(int, index)
							  );

	if (ret < 0) {
		qCCritical(lcClient).noquote() << tr("Nem lehet a lapokat eltávolítani!");
		return;
	}

	qCDebug(lcClient()).noquote() << tr("Lapozás vissza eddig: %1").arg(ret);
}





/**
 * @brief Client::closeWindow
 * @return
 */

bool Client::closeWindow(const bool &forced)
{
	if (m_mainWindowClosable)
		return true;

	QString question = "";
	QMetaObject::invokeMethod(m_mainStack, "getCloseWindowQuestion",
							  Q_RETURN_ARG(QString, question)
							  );

	if (forced || question.isEmpty()) {
		qCDebug(lcClient()).noquote() << tr("Ablak bezárása");
		m_mainWindowClosable = true;
		m_mainWindow->close();
		return true;
	}

	qCDebug(lcClient()).noquote() << tr("Ablak bezárás kérése");

	QMetaObject::invokeMethod(m_mainWindow, "closeQuestion",
							  Q_ARG(QString, question)
							  );

	return false;
}





/**
 * @brief Client::resetPixelSize
 */

void Client::resetPixelSize()
{
	setPixelSize(m_defaultPixelSize);
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

	if (m_mainWindow)
		m_mainWindow->setIcon(QIcon(":/internal/img/cos.png"));
}

