/*
 * ---- Call of Suli ----
 *
 * mapeditor.h
 *
 * Created on: 2021. 05. 24.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * MapEditor
 *
 *  This file is part of Call of Suli.
 *
 *  Call of Suli is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MAPEDITOR_H
#define MAPEDITOR_H

#include "abstractactivity.h"
#include <QObject>
#include "variantmapmodel.h"
#include "variantmapdata.h"

class MapEditor : public AbstractActivity
{
	Q_OBJECT

	Q_PROPERTY(QString filename READ filename WRITE setFilename NOTIFY filenameChanged)
	Q_PROPERTY(QString readableFilename READ readableFilename NOTIFY filenameChanged)
	Q_PROPERTY(bool loaded READ loaded NOTIFY loadedChanged)
	Q_PROPERTY(bool modified READ modified WRITE setModified NOTIFY modifiedChanged)

	Q_PROPERTY(qreal loadProgress READ loadProgress WRITE setLoadProgress NOTIFY loadProgressChanged)
	Q_PROPERTY(QPair<qreal, qreal> loadProgressFraction READ loadProgressFraction WRITE setLoadProgressFraction NOTIFY loadProgressFractionChanged)

	Q_PROPERTY(bool isWithGraphviz READ isWithGraphviz NOTIFY isWithGraphvizChanged)


	// Missions

	Q_PROPERTY(QString currentMission READ currentMission WRITE setCurrentMission NOTIFY currentMissionChanged)
	Q_PROPERTY(VariantMapModel * modelMissionList READ modelMissionList WRITE setModelMissionList NOTIFY modelMissionListChanged)
	Q_PROPERTY(VariantMapModel * modelTerrainList READ modelTerrainList WRITE setModelTerrainList NOTIFY modelTerrainListChanged)
	Q_PROPERTY(VariantMapModel * modelLevelChapterList READ modelLevelChapterList WRITE setModelLevelChapterList NOTIFY modelLevelChapterListChanged)


public:
	explicit MapEditor(QQuickItem *parent = nullptr);
	virtual ~MapEditor();

	//Q_INVOKABLE virtual void run(const QString &func, QVariantMap data = QVariantMap()) override { AbstractActivity::run(m_map, func, data); };

	qreal loadProgress() const { return m_loadProgress; }
	QPair<qreal, qreal> loadProgressFraction() const { return m_loadProgressFraction; }

	QString filename() const { return m_filename; }
	QString readableFilename() const;
	bool isWithGraphviz() const;
	bool modified() const { return m_modified; }
	bool loaded() const { return m_loaded; }

	QString currentMission() const { return m_currentMission; }
	VariantMapModel * modelMissionList() const { return m_modelMissionList; }
	VariantMapModel * modelTerrainList() const { return m_modelTerrainList; }
	VariantMapModel * modelLevelChapterList() const { return m_modelLevelChapterList; }

public slots:
	void open(const QString &filename);
	void create(const QString &filename = "");
	void save(const QString &filename = "");
	void saveCopy(const QString &filename);
	void openUrl(const QUrl &url) { open(url.toLocalFile()); }
	void saveUrl(const QUrl &url) { save(url.toLocalFile()); }
	void saveCopyUrl(const QUrl &url) { saveCopy(url.toLocalFile()); }
	void loadAbort();
	bool setLoadProgress(qreal loadProgress);
	void setLoadProgressFraction(QPair<qreal, qreal> loadProgressFraction);
	void setFilename(QString filename);
	void setModified(bool modified);
	void setLoaded(bool loaded);

	void setCurrentMission(QString currentMission);
	void setModelMissionList(VariantMapModel * modelMissionList);
	void setModelTerrainList(VariantMapModel * modelTerrainList);
	void setModelLevelChapterList(VariantMapModel * modelLevelChapterList);

	void getMissionList();
	void getCurrentMissionData();

	void missionModify(QVariantMap data);
	void missionRemove();
	void missionLevelModify(QVariantMap data);


signals:
	void loadStarted(const QString &filename);
	void loadFailed();
	void loadSucceed();
	void loadProgressChanged(qreal loadProgress);
	void loadProgressFractionChanged(QPair<qreal, qreal> loadProgressFraction);

	void saveFailed();
	void saveSucceed(const QString &filename, const bool &isCopy);
	void saveDialogRequest(const bool &isNew);

	void filenameChanged(QString filename);
	void isWithGraphvizChanged(bool isWithGraphviz);
	void modifiedChanged(bool modified);
	void loadedChanged(bool loaded);

	void currentMissionDataChanged(QVariantMap data);

	void currentMissionChanged(QString currentMission);
	void modelMissionListChanged(VariantMapModel * modelMissionList);
	void modelTerrainListChanged(VariantMapModel * modelTerrainList);
	void modelLevelChapterListChanged(VariantMapModel * modelLevelChapterList);

protected:
	void openPrivate(QVariantMap data);
	void createPrivate(QVariantMap data);
	void savePrivate(QVariantMap data);
	bool loadDatabasePrivate(GameMap *game, const QString &filename = "");
	bool createTriggersPrivate();

protected slots:
	void clientSetup() override;

private:
	qreal m_loadProgress;
	QPair<qreal, qreal> m_loadProgressFraction;
	bool m_loadAbortRequest;
	QString m_filename;
	bool m_modified;
	bool m_loaded;
	QString m_currentMission;
	VariantMapModel * m_modelMissionList;
	VariantMapModel * m_modelTerrainList;
	VariantMapModel * m_modelLevelChapterList;
};

#endif // MAPEDITOR_H
