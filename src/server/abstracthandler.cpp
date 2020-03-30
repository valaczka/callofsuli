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

AbstractHandler::AbstractHandler(Client *client, const QJsonObject &object, const QByteArray &binaryData)
	: QObject(client)
	, m_client(client)
	, m_jsonData(object)
	, m_binaryData(binaryData)
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

void AbstractHandler::start(const QString &func, QJsonObject *jsonData, QByteArray *binaryData)
{
	if (!classInit()) {
		addPermissionDenied(jsonData);
		return;
	}

	QMetaObject::invokeMethod(this, func.toStdString().data(), Qt::DirectConnection,
							  Q_ARG(QJsonObject*, jsonData),
							  Q_ARG(QByteArray*, binaryData));

}


/**
 * @brief AbstractHandler::addPermissionDenied
 * @param object
 */

void AbstractHandler::addPermissionDenied(QJsonObject *object)
{
	Q_ASSERT (object);

	object->insert("error", "permission denied");
}
