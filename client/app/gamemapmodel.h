/*
 * ---- Call of Suli ----
 *
 * gamemapmodel.h
 *
 * Created on: 2021. 09. 25.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GameMapModel
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

#ifndef GAMEMAPMODEL_H
#define GAMEMAPMODEL_H

#include <QAbstractTableModel>
#include "gamemap.h"

class GameMapHeaderModel;

class GameMapModel : public QAbstractTableModel
{
	Q_OBJECT

	Q_PROPERTY(GameMapHeaderModel * headerModelTop READ headerModelTop WRITE setHeaderModelTop NOTIFY headerModelTopChanged)
	Q_PROPERTY(GameMapHeaderModel * headerModelLeft READ headerModelLeft WRITE setHeaderModelLeft NOTIFY headerModelLeftChanged)

public:
	explicit GameMapModel(QObject *parent = nullptr);
	virtual ~GameMapModel();

	struct MissionLevel {
		QString uuid;
		int level;
		bool deathmatch;
		bool success;
		int num;
	};

	struct Mission {
		QString uuid;
		QString name;
		QVector<MissionLevel> levels;
	};

	struct User {
		QString username;
		QString firstname;
		QString lastname;
		int xp;
		QVector<MissionLevel> levelData;

		MissionLevel getLevelData(const QString &uuid, const int &level, const bool &deathmatch) const;
		MissionLevel getLevelData(const MissionLevel &missionLevel) const;
	};

	int rowCount(const QModelIndex &) const override;
	int columnCount(const QModelIndex &) const override;
	QVariant data(const QModelIndex &index, int role) const override;
	QHash<int, QByteArray> roleNames() const override { return m_roleNames; }

	QVector<User> users() const;
	QVector<Mission> missions() const;
	inline Mission missionAt(const int &col) const;
	inline MissionLevel missionLevelAt(const int &col) const;

	void setGameMap(GameMap *map);
	void appendUser(const User &user);
	void appendUser(const QString &username, const QString &firstname, const QString &lastname);

	Q_INVOKABLE void clear();
	Q_INVOKABLE QVariantList missionsData() const;

	void addHeaderModel(GameMapHeaderModel *model);
	void deleteHeaderModel(GameMapHeaderModel *model);

	GameMapHeaderModel * headerModelTop() const { return m_headerModelTop; }
	GameMapHeaderModel * headerModelLeft() const { return m_headerModelLeft; }

public slots:
	void setHeaderModelTop(GameMapHeaderModel * headerModelTop);
	void setHeaderModelLeft(GameMapHeaderModel * headerModelLeft);

signals:
	void headerModelTopChanged(GameMapHeaderModel * headerModelTop);
	void headerModelLeftChanged(GameMapHeaderModel * headerModelLeft);


private:
	void doBeginRemoveRows(const int &first, const int &last);
	void doEndRemoveRows();
	void doBeginRemoveColumns(const int &first, const int &last);
	void doEndRemoveColumns();
	void doBeginInsertRows(const int &first, const int &last);
	void doEndInsertRows();
	void doBeginInsertColumns(const int &first, const int &last);
	void doEndInsertColumns();

	QHash<int, QByteArray> m_roleNames;
	QVector<Mission> m_missions;
	QVector<User> m_users;
	QVector<GameMapHeaderModel *> m_headerModels;
	GameMapHeaderModel * m_headerModelTop;
	GameMapHeaderModel * m_headerModelLeft;
};


/**
 * @brief The GameMapHeaderModel class
 */

class GameMapHeaderModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	explicit GameMapHeaderModel(GameMapModel *parent, const Qt::Orientation &orientation);
	virtual ~GameMapHeaderModel();

	int rowCount(const QModelIndex &modelIndex) const override;
	int columnCount(const QModelIndex &modelIndex) const override;
	QVariant data(const QModelIndex &index, int role) const override;
	QHash<int, QByteArray> roleNames() const override;

	void onDeleteParent();
	void onBeginRemoveRows(const int &first, const int &last);
	void onEndRemoveRows();
	void onBeginRemoveColumns(const int &first, const int &last);
	void onEndRemoveColumns();
	void onBeginInsertRows(const int &first, const int &last);
	void onEndInsertRows();
	void onBeginInsertColumns(const int &first, const int &last);
	void onEndInsertColumns();

private:
	GameMapModel *m_mapModel;
	Qt::Orientation m_orientation;

};

#endif // GAMEMAPMODEL_H
