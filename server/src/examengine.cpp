/*
 * ---- Call of Suli ----
 *
 * examengine.cpp
 *
 * Created on: 2023. 12. 22.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ExamEngine
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

#include "examengine.h"
#include "Logger.h"
#include "enginehandler.h"
#include "qjsonobject.h"


ExamEngine::ExamEngine(EngineHandler *handler, QObject *parent)
	: AbstractEngine{EngineMultiPlayer, handler, parent}
{

}

ExamEngine::~ExamEngine()
{

}


/**
 * @brief ExamEngine::handleWebSocketMessage
 * @param stream
 * @param message
 * @param handler
 */

void ExamEngine::handleWebSocketMessage(WebSocketStream *stream, const QJsonValue &message, EngineHandler *handler)
{
	if (!handler || !stream)
		return;
/*
	const QJsonObject &obj = message.toObject();
	const QString &cmd = obj.value(QStringLiteral("cmd")).toString();

	const auto &id = obj.value(QStringLiteral("engine")).toInt(-1);
	ExamEngine *engine = stream->engineGet<ExamEngine>(AbstractEngine::EngineExam, id);

	LOG_CINFO("engine") << "HANDLE" << cmd << message;

	if (cmd == QStringLiteral("connect")) {
		const auto &eList = handler->engines();

		int eid = -1;

		for (const auto &ptr : eList) {
			if (ptr->type() == EngineMultiPlayer) {
				eid = ptr->id();
				break;
			}
		}

		if (eid == -1) {
			const auto &ptr = createEngine(stream, handler);
			if (!ptr.expired())
				eid = ptr.lock()->id();
		} else {
			auto engine = connectToEngine(eid, stream, handler);
		}

		LOG_CINFO("engine") << "CONNECTED" << eid;

		sendStreamJson(stream, QJsonObject{
								   { QStringLiteral("cmd"), cmd },
								   { QStringLiteral("engine"), eid }
							   });

	} else if (cmd == QStringLiteral("state")) {
		handler->engineTrigger(EngineMultiPlayer);
	} else {
		if (!engine) {
			sendStreamJson(stream, QJsonObject{
									   { QStringLiteral("cmd"), cmd },
									   { QStringLiteral("error"), QStringLiteral("invalid engine") }
								   });
			return;
		}

		if (cmd == QStringLiteral("start")) {
			engine->startGame(stream);
		} else if (cmd == QStringLiteral("create")) {
			engine->createGame(stream, obj);
		} else if (cmd == QStringLiteral("enroll")) {
			int pid = engine->enrollPlayer(stream);
			if (pid == -1)
				sendStreamJson(stream, QJsonObject{
										   { QStringLiteral("cmd"), cmd },
										   { QStringLiteral("error"), QStringLiteral("enroll failed") }
									   });
			else
				sendStreamJson(stream, QJsonObject{
										   { QStringLiteral("cmd"), cmd },
										   { QStringLiteral("engine"), engine->id() },
										   { QStringLiteral("playerId"), pid }
									   });
		} else if (cmd == QStringLiteral("prepare")) {
			engine->prepareGame(stream, obj);
		} else {
			sendStreamJson(stream, QJsonObject{
									   { QStringLiteral("cmd"), cmd },
									   { QStringLiteral("error"), QStringLiteral("invalid command") }
								   });
		}
	}
	*/
}



/**
 * @brief ExamEngine::createEngine
 * @param stream
 * @param handler
 * @return
 */

std::weak_ptr<ExamEngine> ExamEngine::createEngine(WebSocketStream *stream, EngineHandler *handler)
{
	if (!handler)
		return std::weak_ptr<ExamEngine>();

	LOG_CDEBUG("engine") << "Create ExamEngine" << stream;

	static int nextId = 0;

	auto ptr = std::make_shared<ExamEngine>(handler);

	++nextId;

	ptr->setId(nextId);
/*
	if (stream) {
		ptr->setHostStream(stream);
		LOG_CTRACE("engine") << "SET HOST" << ptr.get() << stream << ptr->m_id;
		ptr->setGameState(StateConnecting);
		handler->websocketEngineLink(stream, ptr);
		ptr->enrollPlayer(stream);
	}
*/
	handler->engineAdd(ptr);

	return ptr;
}


/**
 * @brief ExamEngine::connectToEngine
 * @param id
 * @param stream
 * @param handler
 * @return
 */

std::weak_ptr<AbstractEngine> ExamEngine::connectToEngine(const int &id, WebSocketStream *stream, EngineHandler *handler)
{
	return {};
}



/**
 * @brief ExamEngine::sendStreamJson
 * @param stream
 * @param value
 */

void ExamEngine::sendStreamJson(WebSocketStream *stream, const QJsonValue &value)
{
	if (!stream)
		return;

	stream->sendJson("exam", value);
}
