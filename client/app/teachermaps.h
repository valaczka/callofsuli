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

class TeacherMaps : public AbstractActivity
{
	Q_OBJECT

	Q_PROPERTY(VariantMapModel * modelMapList READ modelMapList NOTIFY modelMapListChanged)
	Q_PROPERTY(VariantMapModel * modelGroupList READ modelGroupList WRITE setModelGroupList NOTIFY modelGroupListChanged)

	Q_PROPERTY(QString selectedMapId READ selectedMapId WRITE setSelectedMapId NOTIFY selectedMapIdChanged)

	Q_PROPERTY(bool isUploading READ isUploading WRITE setIsUploading NOTIFY isUploadingChanged)

public:
	explicit TeacherMaps(QQuickItem *parent = nullptr);
	~TeacherMaps();

	static CosDb *teacherMapsDb(Client *client, QObject *parent = nullptr, const QString &connectionName = "teacherMapsDb");

	//Q_INVOKABLE virtual void run(const QString &func, QVariantMap data = QVariantMap()) override { AbstractActivity::run(m_map, func, data); };

	VariantMapModel * modelMapList() const { return m_modelMapList; }
	VariantMapModel * modelGroupList() const { return m_modelGroupList; }

	bool isUploading() const { return m_isUploading; }

	QString selectedMapId() const { return m_selectedMapId; }

public slots:
	void mapSelect(const QString &uuid);
	void mapAdd(QVariantMap data);
	void mapDownload(QVariantMap data);
	void mapUpload(QVariantMap data);
	void mapExport(QVariantMap data);
	void mapImport(QVariantMap data = QVariantMap());
	void mapRename(QVariantMap data);
	void mapLocalCopy(QVariantMap data);
	void mapLocalRemove(QVariantMap data);

	void setIsUploading(bool isUploading);
	void setModelGroupList(VariantMapModel * modelGroupList);
	void setSelectedMapId(QString selectedMapId);

protected slots:
	void clientSetup() override;

private slots:
	void onMapGet(QJsonObject jsonData, QByteArray);
	void onMapListGet(QJsonObject jsonData, QByteArray);
	void onMapUpdated(QJsonObject jsonData, QByteArray);
	void onOneDownloadFinished(const CosDownloaderItem &item, const QByteArray &data, const QJsonObject &jsonData);


signals:
	void mapDownloadRequest(QString formattedDataSize);
	void mapListGet(QJsonObject jsonData, QByteArray binaryData);
	void mapUpdate(QJsonObject jsonData, QByteArray binaryData);
	void mapRemove(QJsonObject jsonData, QByteArray binaryData);
	void mapGet(QJsonObject jsonData, QByteArray binaryData);
	void mapExcludedGroupListGet(QJsonObject jsonData, QByteArray binaryData);
	void mapGroupAdd(QJsonObject jsonData, QByteArray binaryData);
	void mapGroupRemove(QJsonObject jsonData, QByteArray binaryData);

	void modelMapListChanged(VariantMapModel * modelMapList);
	void modelGroupListChanged(VariantMapModel * modelGroupList);
	void isUploadingChanged(bool isUploading);
	void selectedMapIdChanged(QString selectedMapId);

private:
	//QHash<QString, void (TeacherMaps::*)(QVariantMap)> m_map;
	VariantMapModel * m_modelMapList;
	bool m_isUploading;
	VariantMapModel * m_modelGroupList;
	QString m_selectedMapId;
};

#endif // TEACHERMAPS_H
