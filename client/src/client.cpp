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

#include "client.h"

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

void Client::addPage() const
{
	if (m_mainStack) {
		bool ret = false;
		QMetaObject::invokeMethod(m_mainStack, "createPage",
								  Q_RETURN_ARG(bool, ret),
								  Q_ARG(QString, "PageStart.qml"),
								  Q_ARG(QVariant, QVariantMap({}))
								  );
	}
}


/**
 * @brief Client::resetPixelSize
 */

void Client::resetPixelSize()
{
	setPixelSize(m_defaultPixelSize);
}
