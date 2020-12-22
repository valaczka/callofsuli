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
#include "gamematch.h"

class MapEditorWorker;


class MapEditor : public AbstractActivity
{
	Q_OBJECT

	Q_PROPERTY(QString mapName READ mapName WRITE setMapName NOTIFY mapNameChanged)
	Q_PROPERTY(qreal loadProgress READ loadProgress WRITE setLoadProgress NOTIFY loadProgressChanged)
	Q_PROPERTY(QPair<qreal, qreal> loadProgressFraction READ loadProgressFraction WRITE setLoadProgressFraction NOTIFY loadProgressFractionChanged)

	Q_PROPERTY(bool modified READ modified WRITE setModified NOTIFY modifiedChanged)

public:
	MapEditor(QQuickItem *parent = nullptr);
	~MapEditor();

	QString mapName() const { return m_mapName; }

	Q_INVOKABLE void checkBackup();
	Q_INVOKABLE void removeBackup();
	Q_INVOKABLE void removeDatabase();
	Q_INVOKABLE void loadAbort();

	Q_INVOKABLE virtual void run(const QString &func, QVariantMap data = QVariantMap()) override { AbstractActivity::run(m_map, func, data); };

	Q_INVOKABLE QVariantMap moduleData(const QString &module, const bool &isObjective = true) const;

	qreal loadProgress() const { return m_loadProgress; }
	QPair<qreal, qreal> loadProgressFraction() const { return m_loadProgressFraction; }

	bool modified() const { return m_modified; }

public slots:
	void setMapName(QString mapName);
	bool setLoadProgress(qreal loadProgress);
	void setLoadProgressFraction(QPair<qreal, qreal> loadProgressFraction);

	void setModified(bool modified);

	void loadFromFile(QVariantMap data);
	void saveToFile(QVariantMap data);
	void createNew(QVariantMap data);
	void play(QVariantMap data);

protected:
	void loadFromFilePrivate(QVariantMap data);
	void saveToFilePrivate(QVariantMap data);
	void createNewPrivate(QVariantMap data);
	QStringList checkTerrains(GameMap *game) const;
	bool checkGame(GameMap *game) const;

	void campaignAdd(QVariantMap data);
	void campaignModify(QVariantMap data);
	void campaignRemove(QVariantMap data);
	void campaignListReload(QVariantMap = QVariantMap());
	void campaignLoad(QVariantMap data);
	void campaignLockAdd(QVariantMap data);
	void campaignLockRemove(QVariantMap data);
	void campaignLockGetList(QVariantMap data);

	void missionAdd(QVariantMap data);
	void missionModify(QVariantMap data);
	void missionRemove(QVariantMap data);
	void missionLoad(QVariantMap data);
	void missionLockAdd(QVariantMap data);
	void missionLockRemove(QVariantMap data);
	void missionLockGetList(QVariantMap data);
	void missionLockGetLevelList(QVariantMap data);
	void missionLockModify(QVariantMap data);
	void missionLevelAdd(QVariantMap data);
	void missionLevelRemove(QVariantMap data);

	void chapterAdd(QVariantMap data);
	void chapterModify(QVariantMap data);
	void chapterRemove(QVariantMap data);
	void chapterListReload(QVariantMap = QVariantMap());

	void objectiveAdd(QVariantMap data);
	void objectiveModify(QVariantMap data);
	void objectiveRemove(QVariantMap data);
	void objectiveLoad(QVariantMap data);

	void levelLoad(QVariantMap data);
	void levelModify(QVariantMap data);

	void blockChapterMapLoad(QVariantMap data);
	void blockChapterMapAdd(QVariantMap data);
	void blockChapterMapRemove(QVariantMap data);

	void blockChapterMapBlockGetList(QVariantMap data);
	void blockChapterMapBlockAdd(QVariantMap data);
	void blockChapterMapBlockRemove(QVariantMap data);

	void blockChapterMapChapterGetList(QVariantMap data);
	void blockChapterMapChapterAdd(QVariantMap data);
	void blockChapterMapChapterRemove(QVariantMap data);

signals:
	void backupReady(QString mapName, QString details);
	void backupUnavailable();

	void loadStarted();
	void loadFinished();
	void loadFailed();

	void saveStarted();
	void saveFinished();
	void saveFailed();

	void playReady(GameMatch *gameMatch);
	void playFailed();

	void campaignListReloaded(const QVariantList &list);
	void campaignAdded(const int &rowid);
	void campaignModified(const int &id);
	void campaignRemoved(const int &id);
	void campaignLoaded(const QVariantMap &data);
	void campaignSelected(const int &id);
	void campaignLockListLoaded(const int &id, const QVariantList &list);

	void missionAdded(const int &rowid);
	void missionModified(const QString &uuid);
	void missionRemoved(const QString &uuid);
	void missionLoaded(const QVariantMap &data);
	void missionSelected(const QString &uuid);
	void missionLockListLoaded(const QString &uuid, const QVariantList &list);
	void missionLockLevelListLoaded(const QString &uuid, const QString &lock, const QVariantList &list);

	void chapterListReloaded(const QVariantList &list);
	void chapterAdded(const int &rowid);
	void chapterModified(const int &id);
	void chapterRemoved(const int &id);
	void chapterSelected(const int &id);

	void objectiveAdded(const int &rowid);
	void objectiveModified(const QString &uuid);
	void objectiveRemoved(const QString &uuid);
	void objectiveLoaded(const QVariantMap &data);
	void objectiveSelected(const QString &uuid);

	void levelSelected(const int &rowid, const QString &missionUuid);
	void levelLoaded(const QVariantMap &data);

	void blockChapterMapLoaded(const QVariantMap &data);
	void blockChapterMapSelected(const int &id);
	void blockChapterMapRemoved(const int &id);

	void blockChapterMapBlockListLoaded(const int &id, const QVariantList &list);
	void blockChapterMapChapterListLoaded(const int &id, const QVariantList &list);

	void mapNameChanged(QString mapName);
	void loadProgressChanged(qreal loadProgress);
	void loadProgressFractionChanged(QPair<qreal, qreal> loadProgressFraction);

	void modifiedChanged(bool modified);
	void gameMatchChanged(GameMatch * gameMatch);

protected slots:
	//void clientSetup() override;
	//void onMessageReceived(const CosMessage &message) override;
	//void onMessageFrameReceived(const CosMessage &message) override;

private:
	bool _createDatabase(GameMap *game);
	bool _createTriggers();
	QList<int> _blockChapterMapBlockGetListPrivate(const QString &mission, const int &level, const QString &terrain);

	QString m_mapName;
	qreal m_loadProgress;
	QPair<qreal, qreal> m_loadProgressFraction;
	bool m_loadAbortRequest;
	QHash<QString, void (MapEditor::*)(QVariantMap)> m_map;
	bool m_modified;
};



#endif // MAPEDITOR_H
