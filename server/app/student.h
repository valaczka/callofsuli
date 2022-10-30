/*
 * ---- Call of Suli ----
 *
 * student.h
 *
 * Created on: 2020. 12. 28.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * Student
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

#ifndef STUDENT_H
#define STUDENT_H

#include "abstracthandler.h"
#include "gamemap.h"

class Client;

class Student : public AbstractHandler
{
	Q_OBJECT

public:
	explicit Student(Client *client, const CosMessage &message);

	bool classInit() override;

public slots:
	bool groupListGet(QJsonObject *jsonResponse, QByteArray *);
	bool mapListGet(QJsonObject *jsonResponse, QByteArray *);
	bool userListGet(QJsonObject *jsonResponse, QByteArray *);
	bool missionListGet(QJsonObject *jsonResponse, QByteArray *);
	bool campaignGet(QJsonObject *jsonResponse, QByteArray *);
	bool campaignListGet(QJsonObject *jsonResponse, QByteArray *);

	bool gameCreate(QJsonObject *jsonResponse, QByteArray *);
	bool gameUpdate(QJsonObject *jsonResponse, QByteArray *);
	bool gameFinish(QJsonObject *jsonResponse, QByteArray *);
	bool gameListUserGet(QJsonObject *jsonResponse, QByteArray *);
	bool gameListCampaignGet(QJsonObject *jsonResponse, QByteArray *);
	bool gameListUserMissionGet(QJsonObject *jsonResponse, QByteArray *);

	bool userGet(QJsonObject *jsonResponse, QByteArray *);
	bool userModify(QJsonObject *jsonResponse, QByteArray *);
	bool userPasswordChange(QJsonObject *jsonResponse, QByteArray *);

	bool examEngineConnect(QJsonObject *jsonResponse, QByteArray *);
	bool examEngineMapGet(QJsonObject *jsonResponse, QByteArray *);

private:
	GameMap::SolverInfo missionSolverInfo(const QString &mapid, const QString &missionid) const;
	void updateStatistics(const QVariantList &list);
};

#endif // STUDENT_H
