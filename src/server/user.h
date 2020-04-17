/*
 * ---- Call of Suli ----
 *
 * user.h
 *
 * Created on: 2020. 04. 11.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * User
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

#ifndef USER_H
#define USER_H

#include <QObject>
#include "client.h"
#include "abstracthandler.h"

class Client;

class User : public AbstractHandler
{
	Q_OBJECT

public:
	explicit User(Client *client, const QJsonObject &object, const QByteArray &binaryData);

	bool classInit() override;

public slots:
	void getAllUser(QJsonObject *jsonResponse, QByteArray *);
	void userGet(QJsonObject *jsonResponse, QByteArray *);
	void userCreate(QJsonObject *jsonResponse, QByteArray *);
	void userUpdate(QJsonObject *jsonResponse, QByteArray *);

	void getAllClass(QJsonObject *jsonResponse, QByteArray *);
	void classCreate(QJsonObject *jsonResponse, QByteArray *);
	void classUpdate(QJsonObject *jsonResponse, QByteArray *);

};

#endif // USER_H