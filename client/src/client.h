/*
 * ---- Call of Suli ----
 *
 * client.h
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

#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QQuickItem>

class Client : public QObject
{
	Q_OBJECT

	Q_PROPERTY(qreal pixelSize READ pixelSize WRITE setPixelSize NOTIFY pixelSizeChanged)
	Q_PROPERTY(qreal pixelSizeRatio READ pixelSizeRatio WRITE setPixelSizeRatio NOTIFY pixelSizeRatioChanged RESET resetPixelSize)

	Q_PROPERTY(QQuickItem* mainStack READ mainStack WRITE setMainStack NOTIFY mainStackChanged)


public:
	explicit Client(QObject *parent = nullptr);
	virtual ~Client();

	qreal pixelSize() const;
	void setPixelSize(qreal newPixelSize);

	qreal pixelSizeRatio() const;
	void setPixelSizeRatio(qreal newPixelSizeRatio);
	Q_INVOKABLE void resetPixelSize();

	QQuickItem *mainStack() const;
	void setMainStack(QQuickItem *newMainStack);

	Q_INVOKABLE void addPage() const;


signals:
	void pixelSizeChanged();
	void pixelSizeRatioChanged();

	void mainStackChanged();

private:
	const qreal m_defaultPixelSize = 16.0;
	qreal m_pixelSize = m_defaultPixelSize;

	QQuickItem *m_mainStack = nullptr;
	qreal m_pixelSizeRatio;
};

#endif // CLIENT_H
