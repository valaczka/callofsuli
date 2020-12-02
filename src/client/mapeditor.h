/*
 * ---- Call of Suli ----
 *
 * mapeditor.h
 *
 * Created on: 2020. 11. 28.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * MapEditor
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

#ifndef MAPEDITOR_H
#define MAPEDITOR_H

#include "abstractactivity.h"
#include "../../common/gamemap.h"
#include "variantmapmodel.h"
#include "variantmapdata.h"

class MapEditorWorker;


class MapEditor : public AbstractActivity
{
	Q_OBJECT

	Q_PROPERTY(QString mapName READ mapName WRITE setMapName NOTIFY mapNameChanged)
	Q_PROPERTY(qreal loadProgress READ loadProgress WRITE setLoadProgress NOTIFY loadProgressChanged)
	Q_PROPERTY(QPair<qreal, qreal> loadProgressFraction READ loadProgressFraction WRITE setLoadProgressFraction NOTIFY loadProgressFractionChanged)

	Q_PROPERTY(VariantMapModel* campaignModel READ campaignModel WRITE setCampaignModel NOTIFY campaignModelChanged)
	Q_PROPERTY(int campaignModelKey READ campaignModelKey WRITE setCampaignModelKey NOTIFY campaignModelKeyChanged)

public:
	MapEditor(QQuickItem *parent = nullptr);
	~MapEditor();

	QString mapName() const { return m_mapName; }

	Q_INVOKABLE void loadFromFile(const QString &filename);
	Q_INVOKABLE void createNew(const QString &name = QString(), const QString &uuid = QString());
	Q_INVOKABLE void loadFromBackup();
	Q_INVOKABLE void checkBackup();
	Q_INVOKABLE void removeBackup();
	Q_INVOKABLE void removeDatabase();
	Q_INVOKABLE void loadAbort();

	qreal loadProgress() const { return m_loadProgress; }
	QPair<qreal, qreal> loadProgressFraction() const { return m_loadProgressFraction; }

	VariantMapModel* campaignModel() const { return m_campaignModel; }
	int campaignModelKey() const { return m_campaignModelKey; }

public slots:
	void setMapName(QString mapName);
	bool setLoadProgress(qreal loadProgress);
	void setLoadProgressFraction(QPair<qreal, qreal> loadProgressFraction);

	void campaignListReload();


	void setCampaignModel(VariantMapModel* campaignModel);
	void setCampaignModelKey(int campaignModelKey);

signals:
	void backupReady(QString mapName, QString details);
	void backupUnavailable();

	void loadStarted();
	void loadFinished();
	void loadFailed();

	void tableMapChanged();
	void tableChaptersChanged();
	void tableStoragesChanged();
	void tableCampaignsChanged();
	void tableCampaignLocksChanged();
	void tableMissionsChanged();
	void tableMissionLocksChanged();
	void tableMissionLevelsChanged();
	void tableBlockChapterMapsChanged();
	void tableBlockChapterMapBlocksChanged();
	void tableBlockChapterMapChaptersChanged();
	void tableBlockChapterMapFavoritesChanged();
	void tableInventoriesChanged();
	void tableImagesChanged();

	void mapNameChanged(QString mapName);
	void loadProgressChanged(qreal loadProgress);
	void loadProgressFractionChanged(QPair<qreal, qreal> loadProgressFraction);

	void campaignModelChanged(VariantMapModel* campaignModel);
	void campaignModelKeyChanged(int campaignModelKey);

protected slots:
	void clientSetup() override;
	void onMessageReceived(const CosMessage &message) override;
	//void onMessageFrameReceived(const CosMessage &message) override;

private:
	bool _createDatabase();
	bool _createTriggers();
	void _loadFromFile(QString filename);
	void _loadFromNew();

	QString m_mapName;
	GameMap *m_game;
	qreal m_loadProgress;
	QPair<qreal, qreal> m_loadProgressFraction;
	bool m_loadAbortRequest;
	VariantMapData m_campaignData;
	VariantMapModel* m_campaignModel;
	int m_campaignModelKey;
};



#endif // MAPEDITOR_H
