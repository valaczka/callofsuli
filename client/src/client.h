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

#include "qnetworkaccessmanager.h"
#include <QObject>
#include <QQuickItem>
#include <QLoggingCategory>
#include "style.h"
#include "utils.h"

class Application;
class AbstractGame;


/**
 * @brief The Client class
 */

class Client : public QObject
{
	Q_OBJECT

	Q_PROPERTY(qreal pixelSize READ pixelSize WRITE setPixelSize NOTIFY pixelSizeChanged)
	Q_PROPERTY(qreal pixelSizeRatio READ pixelSizeRatio WRITE setPixelSizeRatio NOTIFY pixelSizeRatioChanged RESET resetPixelSize)

	Q_PROPERTY(QQuickItem* mainStack READ mainStack WRITE setMainStack NOTIFY mainStackChanged)
	Q_PROPERTY(QQuickWindow* mainWindow READ mainWindow WRITE setMainWindow NOTIFY mainWindowChanged)

	Q_PROPERTY(Utils* Utils READ utils CONSTANT)
	Q_PROPERTY(Style* Style READ style CONSTANT)
	Q_PROPERTY(bool debug READ debug CONSTANT)

	Q_PROPERTY(AbstractGame* currentGame READ currentGame NOTIFY currentGameChanged)

public:
	explicit Client(Application *app, QObject *parent = nullptr);
	virtual ~Client();

	qreal pixelSize() const;
	void setPixelSize(qreal newPixelSize);

	qreal pixelSizeRatio() const;
	void setPixelSizeRatio(qreal newPixelSizeRatio);
	Q_INVOKABLE void resetPixelSize();

	QQuickItem *mainStack() const;
	void setMainStack(QQuickItem *newMainStack);

	Q_INVOKABLE QQuickItem *stackPushPage(const QString &qml, const QVariantMap &parameters = {}) const;
	Q_INVOKABLE bool stackPop(const int &index = -1, const bool &forced = false) const;

	QQuickWindow *mainWindow() const;
	void setMainWindow(QQuickWindow *newMainWindow);

	Q_INVOKABLE bool closeWindow(const bool &forced = false);

	QNetworkAccessManager *networkManager() const;
	Utils *utils() const;
	Style *style() const;
	Application *application() const;

	bool debug() const;

	AbstractGame *currentGame() const;
	void setCurrentGame(AbstractGame *newCurrentGame);

	Q_INVOKABLE void loadGame();

	Q_INVOKABLE void messageInfo(const QString &text, QString title = "") const;
	Q_INVOKABLE void messageWarning(const QString &text, QString title = "") const;
	Q_INVOKABLE void messageError(const QString &text, QString title = "") const;



protected slots:
	virtual void onApplicationStarted();
	friend class Application;

protected:
	void _message(const QString &text, const QString &title, const QString &icon) const;

signals:
	void pixelSizeChanged();
	void pixelSizeRatioChanged();
	void mainStackChanged();
	void mainWindowChanged();
	void currentGameChanged();

protected:
	Application *const m_application = nullptr;

	const qreal m_defaultPixelSize = 16.0;
	qreal m_pixelSize = m_defaultPixelSize;

	QQuickItem *m_mainStack = nullptr;
	qreal m_pixelSizeRatio;

	QQuickWindow *m_mainWindow = nullptr;
	bool m_mainWindowClosable = false;

	QNetworkAccessManager *const m_networkManager = nullptr;
	Utils *const m_utils = nullptr;
	Style *const m_style = nullptr;
	AbstractGame *m_currentGame = nullptr;

};

Q_DECLARE_LOGGING_CATEGORY(lcClient)

#endif // CLIENT_H
