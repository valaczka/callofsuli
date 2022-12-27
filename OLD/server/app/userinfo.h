/*
 * ---- Call of Suli ----
 *
 * userinfo.h
 *
 * Created on: 2020. 03. 26.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * UserInfo
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

#ifndef USERINFO_H
#define USERINFO_H

#include <QObject>
#include "client.h"
#include "abstracthandler.h"

class Client;

class UserInfo : public AbstractHandler
{
	Q_OBJECT

public:
	explicit UserInfo(Client *client, const CosMessage &message);

	bool classInit() override { return true; }
	void startHttpReply(QNetworkReply *reply, void *data) override;

	struct OAuth2Data {
		QString classCode = "";
		QStringList domainList = {};
		QString token = "";
		QString refreshToken = "";
		QDateTime expiration = QDateTime();
		bool isRefreshQuery = false;
	};

public slots:
	bool getServerInfo(QJsonObject *jsonResponse, QByteArray *);
	bool getUser(QJsonObject *jsonResponse, QByteArray *);
	bool getAllUser(QJsonObject *jsonResponse, QByteArray *);
	bool getUserScore(QJsonObject *jsonResponse, QByteArray *);
	bool registrationRequest(QJsonObject *jsonResponse, QByteArray *);
	bool getResources(QJsonObject *jsonResponse, QByteArray *);
	bool downloadFile(QJsonObject *jsonResponse, QByteArray *binaryResponse);
	bool downloadMap(QJsonObject *jsonResponse, QByteArray *binaryResponse);
	bool getMyGroups(QJsonObject *jsonResponse, QByteArray *);

private:
	void getOAuth2Userinfo(const QString &token, const QString &refreshToken, const QDateTime &expiration, const QStringList &domainList, const QString &classCode);
	void onOAuth2UserinfoReply(QNetworkReply *reply, void *data);

};

#endif // USERINFO_H