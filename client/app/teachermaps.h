/*
 * ---- Call of Suli ----
 *
 * teachermaps.h
 *
 * Created on: 2020. 12. 26.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * TeacherMaps
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

#ifndef TEACHERMAPS_H
#define TEACHERMAPS_H

#include "abstractactivity.h"
#include "variantmapmodel.h"
#include "variantmapdata.h"
#include "gamemap.h"

class TeacherMaps : public AbstractActivity
{
	Q_OBJECT

	Q_PROPERTY(VariantMapModel * modelMapList READ modelMapList NOTIFY modelMapListChanged)

	Q_PROPERTY(QString selectedMapId READ selectedMapId WRITE setSelectedMapId NOTIFY selectedMapIdChanged)

public:
	explicit TeacherMaps(QQuickItem *parent = nullptr);
	~TeacherMaps();

	static CosDb *teacherMapsDb(Client *client, QObject *parent = nullptr, const QString &connectionName = "teacherMapsDb");


	static QVariantMap mapDownloadInfo(CosDb *db);
	static void mapDownloadPrivate(const QVariantMap &data, CosDownloader *downloader, VariantMapModel *mapModel);
	static void mapDownloadFinished(CosDb *db, const CosDownloaderItem &item, const QByteArray &data);

	VariantMapModel * modelMapList() const { return m_modelMapList; }
	QString selectedMapId() const { return m_selectedMapId; }

public slots:
	void mapDownload(QVariantMap data);
	void mapUpload(const QUrl &url);
	void mapOverride(const QUrl &url);
	void mapExport(const QUrl &url);

	void setSelectedMapId(QString selectedMapId);

protected slots:
	void clientSetup() override;

private slots:
	void onMapListGet(QJsonObject jsonData, QByteArray);
	void onOneDownloadFinished(const CosDownloaderItem &item, const QByteArray &data, const QJsonObject &);
	void onMapSelected(const QString & = QString());


signals:
	void mapDownloadRequest(QString formattedDataSize);

	void mapListGet(QJsonObject jsonData, QByteArray binaryData);
	void mapRemove(QJsonObject jsonData, QByteArray binaryData);
	void mapModify(QJsonObject jsonData, QByteArray binaryData);
	void mapAdd(QJsonObject jsonData, QByteArray binaryData);

	void mapDataReady(const QVariantMap data);
	void modelMapListChanged(VariantMapModel * modelMapList);
	void selectedMapIdChanged(QString selectedMapId);

private:
	VariantMapModel * m_modelMapList;
	QString m_selectedMapId;
};

#endif // TEACHERMAPS_H
