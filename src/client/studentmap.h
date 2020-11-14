/*
 * ---- Call of Suli ----
 *
 * studentmap.h
 *
 * Created on: 2020. 06. 01.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * StudentMap
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

#ifndef STUDENTMAP_H
#define STUDENTMAP_H

#include <QObject>
#include "mapdata.h"
#include "student.h"


class StudentMap : public MapData
{
	Q_OBJECT

	Q_PROPERTY(Student * student READ student WRITE setStudent NOTIFY studentChanged)
	Q_PROPERTY(QVariantList campaignList READ campaignList NOTIFY campaignListChanged)

public:
	StudentMap(QQuickItem *parent = nullptr);

	Student* student() const { return m_student; }
	QVariantList campaignList() const { return m_campaignList; }

public slots:
	bool loadFromRepository(const QString &uuid, const QString &md5 = "");
	void onMapDataReceived(const QJsonObject &jsonData, const QByteArray &mapData);
	void onMapResultListLoaded(const QJsonArray &list);
	void campaignListUpdate();
	QVariantList missionListGet(const int &campaignId);

	void setStudent(Student * student);
	void setCampaignList(QVariantList campaignList);

private slots:
	void onStudentChanged(Student *);

private:
	bool isCampaignCompleted(const int &campaignId);

signals:
	void mapLoaded(const QString &uuid);
	void mapLoadingStarted(const QString &uuid);
	void mapDownloadRequest(const QString &uuid);
	void mapDownloadError();
	void mapDownloaded(const QString &uuid);
	void mapResultUpdated();

	void studentChanged(Student * student);
	void campaignListChanged(QVariantList campaignList);

private:
	QString m_uuid;
	Student * m_student;
	QVariantList m_campaignList;
};

#endif // STUDENTMAP_H
