/*
 * ---- Call of Suli ----
 *
 * enginehandler_p.h
 *
 * Created on: 2024. 10. 19.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * %{Cpp:License:ClassName}
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

#ifndef ENGINEHANDLER_P_H
#define ENGINEHANDLER_P_H

#include <QObject>
#include "abstractengine.h"
#include "qbasictimer.h"
#include <QWebSocket>


/**
 * @brief The EngineHandlerPrivate class
 */

class EngineHandlerPrivate : public QObject
{
	Q_OBJECT

public:
	explicit EngineHandlerPrivate(EngineHandler *handler);
	virtual ~EngineHandlerPrivate();

private:
	const QVector<std::shared_ptr<AbstractEngine> > &engines() const { return m_engines; }
	void engineAdd(const std::shared_ptr<AbstractEngine> &engine);
	//void engineRemove(const std::shared_ptr<AbstractEngine> &engine);
	void engineRemove(AbstractEngine *engine);
	void engineRemoveUnused();

	void engineAddStream(WebSocketStream *stream, const std::shared_ptr<AbstractEngine> &engine);
	void engineRemoveStream(WebSocketStream *stream, AbstractEngine *engine);

	void engineTrigger(const AbstractEngine::Type &type);
	void engineTriggerId(const AbstractEngine::Type &type, const int &id);
	void engineTriggerEngine(AbstractEngine *engine);


	void websocketAdd(QWebSocket *socket);
	void websocketRemove(WebSocketStream *stream);
	void websocketCloseAll();
	void websocketDisconnected(WebSocketStream *stream);
	void websocketTrigger(WebSocketStream *stream);
	void websocketObserverAdded(WebSocketStream *stream, const AbstractEngine::Type &type);
	void websocketObserverRemoved(WebSocketStream *stream, const AbstractEngine::Type &type);
	void websocketEngineLink(WebSocketStream *stream, const std::shared_ptr<AbstractEngine> &engine);
	void websocketEngineUnlink(WebSocketStream *stream, AbstractEngine *engine);

	void timerEvent(QTimerEvent *event) override;
	void timerEventRun();
	void timerMinuteEventRun();

	void onBinaryDataReceived(WebSocketStream *stream, const QByteArray &data);

private:
	EngineHandler *q = nullptr;
	QRecursiveMutex m_mutex;

	QDateTime m_timerLastMinute;

	QVector<std::shared_ptr<AbstractEngine>> m_engines;
	std::vector<std::unique_ptr<WebSocketStream>> m_streams;


	friend class EngineHandler;
};


#endif // ENGINEHANDLER_P_H
