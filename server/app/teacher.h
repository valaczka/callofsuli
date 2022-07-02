/*
 * ---- Call of Suli ----
 *
 * teachermap.h
 *
 * Created on: 2020. 12. 26.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * TeacherMap
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

#ifndef TEACHER_H
#define TEACHER_H

#include "abstracthandler.h"
#include <QObject>

class Client;

class Teacher : public AbstractHandler
{
	Q_OBJECT

public:
	struct Grading {
		enum Type {
			TypeInvalid,
			TypeGrade,
			TypeXP
		};

		enum Mode {
			ModeInvalid,
			ModeDefault,
			ModeRequired
		};

		Type type;
		int value;
		int ref;
		Mode mode;
		QJsonObject criteria;
		bool success;
		int id;

		Grading() :
			type(TypeInvalid), value(-1), ref(-1), mode(ModeInvalid), criteria(), success(false), id(-1)
		{}

		Grading(const int &i, const Type &t, const int &v, const QJsonObject &c, const int &r = -1, const bool &s = false);

		bool isValid() const { return type != TypeInvalid; }

		static QJsonArray toArray(const QVector<Grading> &list);
		static QJsonObject toNestedArray(const QVector<Grading> &list);
		static QMap<int, QVector<Grading>> toMap(const QVector<Grading> &list, const Type &type);
	};


	explicit Teacher(Client *client, const CosMessage &message);

	bool classInit() override;


	// Grading

	static QVector<Grading> gradingFromVariantList(const QVariantList &list);
	static Grading gradingResult(const QVector<Grading> &list, const Grading::Type &type);
	static int startAndFinishCampaigns(CosDb *db);
	static bool finishCampaign(CosDb *db, const int &campaignId);
	static bool startCampaign(CosDb *db, const int &campaignId);
	static QVector<Grading>& evaluate(CosDb *db, QVector<Grading> &list, const int &campaignId, const QString &username);
	static Grading& evaluate(CosDb *db, Grading &grading, const int &campaignId, const QString &username);

	QVector<Grading> gradingGet(const int &assignmentId, const int &campaignId, const QString &username, const bool &isFinished);

public slots:

	bool userGet(QJsonObject *jsonResponse, QByteArray *);
	bool userModify(QJsonObject *jsonResponse, QByteArray *);
	bool userPasswordChange(QJsonObject *jsonResponse, QByteArray *);

	bool groupListGet(QJsonObject *jsonResponse, QByteArray *);
	bool groupGet(QJsonObject *jsonResponse, QByteArray *);
	bool groupCreate(QJsonObject *jsonResponse, QByteArray *);
	bool groupRemove(QJsonObject *jsonResponse, QByteArray *);
	bool groupModify(QJsonObject *jsonResponse, QByteArray *);

	bool groupUserGet(QJsonObject *jsonResponse, QByteArray *);
	bool groupUserAdd(QJsonObject *jsonResponse, QByteArray *);
	bool groupUserRemove(QJsonObject *jsonResponse, QByteArray *);
	bool groupExcludedUserListGet(QJsonObject *jsonResponse, QByteArray *);

	bool groupClassAdd(QJsonObject *jsonResponse, QByteArray *);
	bool groupClassRemove(QJsonObject *jsonResponse, QByteArray *);
	bool groupExcludedClassListGet(QJsonObject *jsonResponse, QByteArray *);

	bool groupMapAdd(QJsonObject *jsonResponse, QByteArray *);
	bool groupMapActivate(QJsonObject *jsonResponse, QByteArray *);
	bool groupMapRemove(QJsonObject *jsonResponse, QByteArray *);
	bool groupExcludedMapListGet(QJsonObject *jsonResponse, QByteArray *);

	bool groupTrophyGet(QJsonObject *jsonResponse, QByteArray *);

	bool mapListGet(QJsonObject *jsonResponse, QByteArray *);
	bool mapAdd(QJsonObject *jsonResponse, QByteArray *);
	bool mapRemove(QJsonObject *jsonResponse, QByteArray *);
	bool mapModify(QJsonObject *jsonResponse, QByteArray *);

	bool gameListUserGet(QJsonObject *jsonResponse, QByteArray *);
	bool gameListGroupGet(QJsonObject *jsonResponse, QByteArray *);
	bool gameListCampaignGet(QJsonObject *jsonResponse, QByteArray *);

};










#endif // TEACHER_H
