/*
 * ---- Call of Suli ----
 *
 * serversettings.h
 *
 * Created on: 2020. 11. 20.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ServerSettings
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

#ifndef SERVERSETTINGS_H
#define SERVERSETTINGS_H

#include "abstractactivity.h"
#include "objectlistmodel.h"
#include "userlistobject.h"

class ServerSettings : public AbstractActivity
{
	Q_OBJECT

	Q_PROPERTY(ObjectGenericListModel<UserListObject>* modelUserList READ modelUserList WRITE setModelUserList NOTIFY modelUserListChanged)

public:
	explicit ServerSettings(QQuickItem *parent = nullptr);
	virtual ~ServerSettings();

	ObjectGenericListModel<UserListObject> *modelUserList() const;
	void setModelUserList(ObjectGenericListModel<UserListObject> *newModelUserList);

signals:
	void getSettings(QJsonObject jsonData, QByteArray binaryData);
	void setSettings(QJsonObject jsonData, QByteArray binaryData);
	void classRegistration(QJsonObject jsonData, QByteArray binaryData);

	void getAllClass(QJsonObject jsonData, QByteArray binaryData);
	void classCreate(QJsonObject jsonData, QByteArray binaryData);
	void classUpdate(QJsonObject jsonData, QByteArray binaryData);
	void classRemove(QJsonObject jsonData, QByteArray binaryData);

	void userListGet(QJsonObject jsonData, QByteArray binaryData);

	void modelUserListChanged();

private slots:
	void onUserListGet(QJsonObject jsonData, QByteArray);

private:
	ObjectGenericListModel<UserListObject> *m_modelUserList;
};


#endif // SERVERSETTINGS_H
