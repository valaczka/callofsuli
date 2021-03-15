/*
 * ---- Call of Suli ----
 *
 * abstracthandler.cpp
 *
 * Created on: 2020. 03. 26.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * AbstractHandler
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

#include "abstracthandler.h"

AbstractHandler::AbstractHandler(Client *client, const CosMessage &message, const CosMessage::CosClass &cosClass)
	: QObject(client)
	, m_class(cosClass)
	, m_serverError(CosMessage::ServerNoError)
	, m_client(client)
	, m_message(message)
{

}


/**
 * @brief AbstractHandler::~AbstractHandler
 */

AbstractHandler::~AbstractHandler()
{

}


/**
 * @brief AbstractHandler::start
 * @param func
 */

void AbstractHandler::start()
{
	if (!classInit()) {
		CosMessage r(CosMessage::ClassPermissionDenied, m_message);
		r.send(m_client->socket());
		return;
	}

	QJsonObject jsonObject;
	QByteArray binaryData;
	bool returnArg = true;


	QString func = m_message.cosFunc();

	if (!QMetaObject::invokeMethod(this, func.toStdString().data(), Qt::DirectConnection,
								   Q_RETURN_ARG(bool, returnArg),
								   Q_ARG(QJsonObject *, &jsonObject),
								   Q_ARG(QByteArray *, &binaryData))) {
		CosMessage r(CosMessage::InvalidFunction, m_message);
		r.setCosClass(m_class);
		r.send(m_client->socket());
		return;
	}

	CosMessage r(jsonObject, m_class, func, m_message);


	if (!returnArg || m_serverError != CosMessage::ServerNoError)
		r.setMessageError(CosMessage::OtherError);

	r.setServerError(m_serverError);

	if (!binaryData.isEmpty())
		r.setBinaryData(binaryData);
	r.send(m_client->socket());
}

