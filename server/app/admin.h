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

#ifndef ADMIN_H
#define ADMIN_H

#include <QObject>
#include "client.h"
#include "abstracthandler.h"

class Client;

class Admin : public AbstractHandler
{
	Q_OBJECT

public:
	explicit Admin(Client *client, const CosMessage &message);

	bool classInit() override;

	QString userCreateReal(const QString &username, const QString &firstname, const QString &lastname, const bool &isTeacher = false,
						   const QString &classCode = "", const QString &oauthToken = "",
						   const QString &refreshToken = "", const QDateTime &expiration = QDateTime(),
						   const QString &picture = "", const QString &character = "default");

	bool userPasswordChangeReal(const QString &username, const QString &password);

public slots:
	bool getAllUser(QJsonObject *jsonResponse, QByteArray *);
	bool userListGet(QJsonObject *jsonResponse, QByteArray *);
	bool userGet(QJsonObject *jsonResponse, QByteArray *);
	bool userCreate(QJsonObject *jsonResponse, QByteArray *);
	bool userModify(QJsonObject *jsonResponse, QByteArray *);
	bool userBatchUpdate(QJsonObject *jsonResponse, QByteArray *);
	bool userBatchRemove(QJsonObject *jsonResponse, QByteArray *);
	bool userPasswordChange(QJsonObject *jsonResponse, QByteArray *);

	bool getAllClass(QJsonObject *jsonResponse, QByteArray *);
	bool classCreate(QJsonObject *jsonResponse, QByteArray *);
	bool classUpdate(QJsonObject *jsonResponse, QByteArray *);
	bool classRemove(QJsonObject *jsonResponse, QByteArray *);

	bool getSettings(QJsonObject *jsonResponse, QByteArray *);
	bool setSettings(QJsonObject *jsonResponse, QByteArray *);

	bool classRegistration(QJsonObject *jsonResponse, QByteArray *);

	bool getAllClients(QJsonObject *jsonResponse, QByteArray *);

};

#endif // USER_H
