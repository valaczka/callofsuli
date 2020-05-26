/*
 * ---- Call of Suli ----
 *
 * teachergroups.h
 *
 * Created on: 2020. 05. 24.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * TeacherGroups
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

#ifndef TEACHERGROUPS_H
#define TEACHERGROUPS_H

#include <QObject>
#include "client.h"
#include "abstracthandler.h"


class TeacherGroups : public AbstractHandler
{
	Q_OBJECT

public:
	explicit TeacherGroups(Client *client, const QJsonObject &object, const QByteArray &binaryData);

	bool classInit() override;

public slots:
	void getAllGroup(QJsonObject *jsonResponse, QByteArray *);
	void getGroup(QJsonObject *jsonResponse, QByteArray *);
	void createGroup(QJsonObject *jsonResponse, QByteArray *);
	void updateGroup(QJsonObject *jsonResponse, QByteArray *);
	void removeGroup(QJsonObject *jsonResponse, QByteArray *);
	void getExcludedUsers(QJsonObject *jsonResponse, QByteArray *);
	void getExcludedClasses(QJsonObject *jsonResponse, QByteArray *);
	void getExcludedMaps(QJsonObject *jsonResponse, QByteArray *);


};

#endif // TEACHERGROUPS_H
