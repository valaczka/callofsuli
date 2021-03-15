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


class MapEditor : public AbstractActivity
{
	Q_OBJECT

	Q_PROPERTY(qreal loadProgress READ loadProgress WRITE setLoadProgress NOTIFY loadProgressChanged)
	Q_PROPERTY(QPair<qreal, qreal> loadProgressFraction READ loadProgressFraction WRITE setLoadProgressFraction NOTIFY loadProgressFractionChanged)

	Q_PROPERTY(QString mapName READ mapName WRITE setMapName NOTIFY mapNameChanged)
	Q_PROPERTY(QString fileName READ fileName WRITE setFileName NOTIFY fileNameChanged)
	Q_PROPERTY(CosDb* database READ database WRITE setDatabase NOTIFY databaseChanged)
	Q_PROPERTY(QString databaseTable READ databaseTable WRITE setDatabaseTable NOTIFY databaseTableChanged)
	Q_PROPERTY(QString databaseUuid READ databaseUuid WRITE setDatabaseUuid NOTIFY databaseUuidChanged)

	Q_PROPERTY(bool modified READ modified WRITE setModified NOTIFY modifiedChanged)

	Q_PROPERTY(VariantMapModel * modelTerrains READ modelTerrains WRITE setModelTerrains NOTIFY modelTerrainsChanged)
	Q_PROPERTY(VariantMapModel * modelObjectives READ modelObjectives WRITE setModelObjectives NOTIFY modelObjectivesChanged)

public:
	MapEditor(QQuickItem *parent = nullptr);
	~MapEditor();

	Q_INVOKABLE bool loadDefault();
	Q_INVOKABLE bool saveDefault();

	Q_INVOKABLE void checkBackup();
	Q_INVOKABLE void removeBackup();
	Q_INVOKABLE void loadAbort();
	Q_INVOKABLE void removeImageProvider();

	Q_INVOKABLE virtual void run(const QString &func, QVariantMap data = QVariantMap()) override { AbstractActivity::run(m_map, func, data); };

	Q_INVOKABLE QVariantMap storageInfo(const QString &module) const { return Question::storageInfo(module); }
	Q_INVOKABLE QVariantMap objectiveInfo(const QString &module) const { return Question::objectiveInfo(module); }
	Q_INVOKABLE QStringList objectiveDataToStringList(const QString &module, const QVariantMap &data,
													  const QString &storageModule = "", const QVariantMap &storageData = QVariantMap()) const
	{ return Question::objectiveDataToStringList(module, data, storageModule, storageData); }
	Q_INVOKABLE QStringList objectiveDataToStringList(const QString &module, const QString &dataString,
													  const QString &storageModule = "", const QString &storageDataString = "") const
	{ return Question::objectiveDataToStringList(module, dataString, storageModule, storageDataString); }

	qreal loadProgress() const { return m_loadProgress; }
	QPair<qreal, qreal> loadProgressFraction() const { return m_loadProgressFraction; }

	bool modified() const { return m_modified; }

	QString mapName() const { return m_mapName; }
	QString fileName() const { return m_fileName; }
	CosDb* database() const { return m_database; }
	QString databaseUuid() const { return m_databaseUuid; }
	QString databaseTable() const { return m_databaseTable; }

	VariantMapModel * modelTerrains() const { return m_modelTerrains; }
	VariantMapModel * modelObjectives() const { return m_modelObjectives; }

public slots:
	bool setLoadProgress(qreal loadProgress);
	void setLoadProgressFraction(QPair<qreal, qreal> loadProgressFraction);

	void setModified(bool modified);

	void loadFromFile(const QString &filename);
	void saveToFile(const QString &filename);
	void play(QVariantMap data);


	void setMapName(QString mapName);
	void setFileName(QString fileName);
	void setDatabase(CosDb* database);
	void setDatabaseUuid(QString databaseUuid);
	void setDatabaseTable(QString databaseTable);

	void setModelTerrains(VariantMapModel * modelTerrains);
	void setModelObjectives(VariantMapModel * modelObjectives);

protected:
	void loadFromFilePrivate(QVariantMap data);
	void loadFromDbPrivate(QVariantMap = QVariantMap());
	void saveToFilePrivate(QVariantMap data);
	void saveToDbPrivate(QVariantMap = QVariantMap());
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
	void chapterImport(QVariantMap data);

	void objectiveAdd(QVariantMap data);
	void objectiveModify(QVariantMap data);
	void objectiveRemove(QVariantMap data);
	void objectiveLoad(QVariantMap data);
	void objectiveImport(QVariantMap data);

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

	void missionAdded(const int &rowid, const QString &uuid);
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
	void chapterImportFailed(const QString &errorString);
	void chapterImportReady(const QVariantList &list);

	void objectiveAdded(const int &rowid, const QString &uuid);
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

	void loadProgressChanged(qreal loadProgress);
	void loadProgressFractionChanged(QPair<qreal, qreal> loadProgressFraction);

	void modifiedChanged(bool modified);
	void gameMatchChanged(GameMatch * gameMatch);

	void mapNameChanged(QString mapName);
	void fileNameChanged(QString fileName);
	void databaseChanged(CosDb* database);
	void databaseUuidChanged(QString databaseUuid);
	void databaseTableChanged(QString databaseTable);

	void modelTerrainsChanged(VariantMapModel * modelTerrains);
	void modelObjectivesChanged(VariantMapModel * modelObjectives);

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
	QString m_fileName;
	CosDb* m_database;
	QString m_databaseUuid;
	QString m_databaseTable;
	VariantMapModel * m_modelTerrains;
	VariantMapModel * m_modelObjectives;
};



#endif // MAPEDITOR_H
