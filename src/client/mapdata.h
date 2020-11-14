/*
 * ---- Call of Suli ----
 *
 * map.h
 *
 * Created on: 2020. 03. 27.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * Map
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

#ifndef MAPDATA_H
#define MAPDATA_H

#include <QObject>
#include "abstractactivity.h"
#include "../common/cosdb.h"


class MapData : public AbstractActivity
{
	Q_OBJECT

public:

	enum IntroType { IntroUndefined, IntroCampaign, IntroMission, IntroSummary };
	Q_ENUM(IntroType)

	Q_PROPERTY(QVariantList storageModules READ storageModules)
	Q_PROPERTY(QVariantList objectiveModules READ objectiveModules)

	explicit MapData(const QString &connectionName = "mapDB", QQuickItem *parent = nullptr);
	virtual ~MapData();

	static QVariantList storageModules();
	static QVariantList objectiveModules();

public slots:
	QJsonObject loadFromJson(const QByteArray &data, const bool &binaryFormat = true, double *steps = nullptr, double *currentStep = nullptr);

	static QVariantMap storageModule(const QString &type);
	static QVariantList storageObjectiveModules(const QString &type);
	static QVariantMap objectiveModule(const QString &type);

	virtual QVariantMap infoGet();

	QVariantMap campaignGet(const int &id);
	QVariantList campaignListGet();

	QVariantMap missionGet(const int &id, const bool &isSummary = false, const bool &fullStorage = false, const int &filterLevel = -1);
	QVariantList missionListGet(const int &campaignId = -1);


	QVariantMap summaryGet(const int &id) { return missionGet(id, true); }
	QVariantList summaryStorageListGet(const int &id, const int &filterLevel = -1);

	QVariantMap storageGet(const int &id);
	QVariantList storageListGet(const int &missionId = -1, const int &summaryId = -1, const int &filterLevel = -1);
	QVariantMap storageInfo(const QString &type) const;

	QVariantMap introGet(const int &id);
	QVariantList introListGet(const int &parentId = -1, const IntroType &type = IntroUndefined);

	QVariantMap objectiveGet(const int &id);
	QVariantList objectiveListGet(const int &storageId = -1, const int filterLevel = -1);
	QVariantMap objectiveInfo(const QString &type) const;


private slots:
	//bool databaseInit() override;

protected:
	QJsonArray tableToJson(const QString &table, const bool &convertData = false);
	bool JsonToTable(const QJsonArray &array, const QString &table, const bool &convertData = false);

	void generateMissionUuids();

	QStringList m_tableNames;
	QStringList m_databaseInitSql;


signals:
	void mapLoadingProgress(const double &progress);

private:
	static void setStorageModules();
	static void setObjectiveModules();

	static QVariantList m_storageModules;
	static QVariantList m_objectiveModules;
};

#define DATETIME_JSON_FORMAT QString("yyyy-MM-dd hh:mm:ss")

#endif // MAP_H
