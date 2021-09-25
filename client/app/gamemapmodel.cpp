/*
 * ---- Call of Suli ----
 *
 * gamemapmodel.cpp
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

#include "gamemapmodel.h"


GameMapModel::GameMapModel(QObject *parent)
	: QAbstractTableModel(parent)
	, m_roleNames()
	, m_missions()
	, m_users()
	, m_headerModelTop(nullptr)
	, m_headerModelLeft(nullptr)
{
	m_roleNames[Qt::DisplayRole] = "display";

	int i = Qt::UserRole+1;

	m_roleNames[i++] = "mission";
	m_roleNames[i++] = "level";
	m_roleNames[i++] = "deathmatch";
	m_roleNames[i++] = "success";
	m_roleNames[i++] = "fullname";
	m_roleNames[i++] = "xp";

	m_headerModelTop = new GameMapHeaderModel(this, Qt::Horizontal);
	m_headerModelLeft = new GameMapHeaderModel(this, Qt::Vertical);
}


/**
 * @brief GameMapModel::~GameMapModel
 */

GameMapModel::~GameMapModel()
{
	foreach (GameMapHeaderModel *model, m_headerModels) {
		model->onDeleteParent();
	}

	if (m_headerModelTop)
		delete m_headerModelTop;

	if (m_headerModelLeft)
		delete m_headerModelLeft;
}


/**
 * @brief GameMapModel::rowCount
 * @return
 */

int GameMapModel::rowCount(const QModelIndex &) const
{
	return m_users.count();
}


/**
 * @brief GameMapModel::columnCount
 * @return
 */

int GameMapModel::columnCount(const QModelIndex &) const
{
	int count = 0;

	foreach (GameMapModel::Mission m, m_missions)
		count += m.levels.count();

	return count;
}



/**
 * @brief GameMapModel::data
 * @param index
 * @param role
 * @return
 */

QVariant GameMapModel::data(const QModelIndex &index, int role) const
{
	int col = index.column();
	int row = index.row();

	QByteArray roleName = m_roleNames.value(role);

	if (roleName == "display") {
		GameMapModel::User u = m_users.at(row);
		GameMapModel::MissionLevel ml = u.getLevelData(missionLevelAt(col));
		return ml.level > -1 ? QVariant(ml.num) : QVariant("");
	} else if (roleName == "success") {
		GameMapModel::User u = m_users.at(row);
		GameMapModel::MissionLevel ml = u.getLevelData(missionLevelAt(col));
		return ml.level > -1 ? ml.success : false;
	}
	else if (roleName == "mission")
		return missionAt(col).name;
	else if (roleName == "level")
		return missionLevelAt(col).level;
	else if (roleName == "deathmatch")
		return missionLevelAt(col).deathmatch;
	else if (roleName == "fullname") {
		GameMapModel::User u = m_users.at(row);
		return u.firstname+" "+u.lastname;
	} else if (roleName == "xp") {
		GameMapModel::User u = m_users.at(row);
		return u.xp;
	}


	return QVariant();
}



/**
 * @brief GameMapModel::missions
 * @return
 */

QVector<GameMapModel::Mission> GameMapModel::missions() const
{
	return m_missions;
}


/**
 * @brief GameMapModel::missionAt
 * @param col
 * @return
 */

GameMapModel::Mission GameMapModel::missionAt(const int &col) const
{
	int i = 0;

	foreach (GameMapModel::Mission m, m_missions) {
		for (int j=0; j<m.levels.count(); j++) {
			if (i == col)
				return m;
			i++;
		}
	}

	GameMapModel::Mission m;
	m.name = "";
	m.uuid = "";
	m.levels = QVector<GameMapModel::MissionLevel>();

	return m;
}


/**
 * @brief GameMapModel::missionLevelAt
 * @param col
 * @return
 */

GameMapModel::MissionLevel GameMapModel::missionLevelAt(const int &col) const
{
	int i = 0;

	foreach (GameMapModel::Mission m, m_missions) {
		foreach (GameMapModel::MissionLevel ml, m.levels) {
			if (i == col)
				return ml;
			i++;
		}
	}

	GameMapModel::MissionLevel ml;
	ml.uuid = "";
	ml.level = -1;
	ml.deathmatch = false;
	ml.success = false;
	ml.num = 0;

	return ml;
}


/**
 * @brief GameMapModel::setGameMap
 * @param map
 */

void GameMapModel::setGameMap(GameMap *map)
{
	clear();

	if (!map)
		return;

	int cols = 0;

	foreach(GameMap::Mission *m, map->missions()) {
		GameMapModel::Mission mis;
		mis.uuid = QString::fromLatin1(m->uuid());
		mis.name = m->name();

		int n = 0;

		foreach(GameMap::MissionLevel *ml, m->levels()) {
			GameMapModel::MissionLevel misLevel;
			misLevel.uuid = mis.uuid;
			misLevel.level = ml->level();
			misLevel.deathmatch = false;

			mis.levels.append(misLevel);
			n++;

			if (ml->canDeathmatch()) {
				misLevel.uuid = mis.uuid;
				misLevel.deathmatch = true;
				mis.levels.append(misLevel);
				n++;
			}
		}

		if (n) {
			doBeginInsertColumns(cols, cols+n-1);
			m_missions.append(mis);
			cols += n;
			doEndInsertColumns();
		}
	}
}


/**
 * @brief GameMapModel::appendUser
 * @param user
 */


void GameMapModel::appendUser(const GameMapModel::User &user)
{
	int i = m_users.size();
	doBeginInsertRows(i, i);
	m_users.append(user);
	doEndInsertRows();
}


/**
 * @brief GameMapModel::appendUser
 * @param username
 * @param firstname
 * @param lastname
 */

void GameMapModel::appendUser(const QString &username, const QString &firstname, const QString &lastname)
{
	GameMapModel::User u;
	u.username = username;
	u.firstname = firstname;
	u.lastname = lastname;
	u.xp = 0;
	u.levelData = QVector<GameMapModel::MissionLevel>();
	appendUser(u);
}



/**
 * @brief GameMapModel::clear
 */

void GameMapModel::clear()
{
	doBeginRemoveRows(0, rowCount(QModelIndex()));
	doBeginRemoveColumns(0, columnCount(QModelIndex()));

	m_users.clear();
	m_missions.clear();

	doEndRemoveColumns();
	doEndRemoveRows();
}



/**
 * @brief GameMapModel::missionsData
 * @return
 */

QVariantList GameMapModel::missionsData() const
{
	QVariantList ret;

	foreach (GameMapModel::Mission m, m_missions) {
		QVariantMap r;
		r["name"] = m.name;
		r["levels"] = m.levels.size();
		ret.append(r);
	}

	return ret;
}





/**
 * @brief GameMapModel::addHeaderModel
 * @param model
 */

void GameMapModel::addHeaderModel(GameMapHeaderModel *model)
{
	m_headerModels.append(model);
}


/**
 * @brief GameMapModel::deleteHeaderModel
 * @param model
 */

void GameMapModel::deleteHeaderModel(GameMapHeaderModel *model)
{
	m_headerModels.removeAll(model);
}


void GameMapModel::setHeaderModelTop(GameMapHeaderModel *headerModelTop)
{
	if (m_headerModelTop == headerModelTop)
		return;

	m_headerModelTop = headerModelTop;
	emit headerModelTopChanged(m_headerModelTop);
}

void GameMapModel::setHeaderModelLeft(GameMapHeaderModel *headerModelLeft)
{
	if (m_headerModelLeft == headerModelLeft)
		return;

	m_headerModelLeft = headerModelLeft;
	emit headerModelLeftChanged(m_headerModelLeft);
}


/**
 * @brief GameMapModel::doBeginRemoveRows
 * @param first
 * @param last
 */


void GameMapModel::doBeginRemoveRows(const int &first, const int &last)
{
	foreach (GameMapHeaderModel *model, m_headerModels)
		model->onBeginRemoveRows(first, last);

	beginRemoveRows(QModelIndex(), first, last);
}


/**
 * @brief GameMapModel::doEndRemoveRows
 */

void GameMapModel::doEndRemoveRows()
{
	foreach (GameMapHeaderModel *model, m_headerModels)
		model->onEndRemoveRows();

	endRemoveRows();
}


/**
 * @brief GameMapModel::doBeginRemoveColumns
 * @param first
 * @param last
 */

void GameMapModel::doBeginRemoveColumns(const int &first, const int &last)
{
	foreach (GameMapHeaderModel *model, m_headerModels)
		model->onBeginRemoveColumns(first, last);

	beginRemoveColumns(QModelIndex(), first, last);
}


/**
 * @brief GameMapModel::doEndRemoveColumns
 */

void GameMapModel::doEndRemoveColumns()
{
	foreach (GameMapHeaderModel *model, m_headerModels)
		model->onEndRemoveColumns();

	endRemoveColumns();
}


/**
 * @brief GameMapModel::doBeginInsertRows
 * @param first
 * @param last
 */

void GameMapModel::doBeginInsertRows(const int &first, const int &last)
{
	foreach (GameMapHeaderModel *model, m_headerModels)
		model->onBeginInsertRows(first, last);

	beginInsertRows(QModelIndex(), first, last);
}


/**
 * @brief GameMapModel::doEndInsertRows
 */

void GameMapModel::doEndInsertRows()
{
	foreach (GameMapHeaderModel *model, m_headerModels)
		model->onEndInsertRows();

	endInsertRows();
}

/**
 * @brief GameMapModel::doBeginInsertColumns
 * @param first
 * @param last
 */

void GameMapModel::doBeginInsertColumns(const int &first, const int &last)
{
	foreach (GameMapHeaderModel *model, m_headerModels)
		model->onBeginInsertColumns(first, last);

	beginInsertColumns(QModelIndex(), first, last);
}


/**
 * @brief GameMapModel::doEndInsertColumns
 */

void GameMapModel::doEndInsertColumns()
{
	foreach (GameMapHeaderModel *model, m_headerModels)
		model->onEndInsertColumns();

	endInsertColumns();
}


/**
 * @brief GameMapModel::users
 * @return
 */

QVector<GameMapModel::User> GameMapModel::users() const
{
	return m_users;
}


/**
 * @brief GameMapModel::User::getLevelData
 * @param level
 * @param deathmatch
 * @return
 */

GameMapModel::MissionLevel GameMapModel::User::getLevelData(const QString &uuid, const int &level, const bool &deathmatch) const
{
	foreach(GameMapModel::MissionLevel l, levelData) {
		if (l.uuid == uuid && l.level == level && l.deathmatch == deathmatch)
			return l;
	}

	GameMapModel::MissionLevel l;
	l.uuid = "";
	l.level = -1;
	l.deathmatch = false;
	l.success = false;
	l.num = 0;
	return l;
}


/**
 * @brief GameMapModel::User::getLevelData
 * @param missionLevel
 * @return
 */

GameMapModel::MissionLevel GameMapModel::User::getLevelData(const GameMapModel::MissionLevel &missionLevel) const
{
	foreach(GameMapModel::MissionLevel l, levelData) {
		if (l.uuid == missionLevel.uuid && l.level == missionLevel.level && l.deathmatch == missionLevel.deathmatch)
			return l;
	}

	GameMapModel::MissionLevel l;
	l.uuid = "";
	l.level = -1;
	l.deathmatch = false;
	l.success = false;
	l.num = 0;
	return l;
}



/**
 * @brief GameMapHeaderModel::GameMapHeaderModel
 * @param parent
 */
GameMapHeaderModel::GameMapHeaderModel(GameMapModel *parent, const Qt::Orientation &orientation)
	: QAbstractTableModel(parent)
	, m_mapModel(parent)
	, m_orientation(orientation)
{

}


/**
 * @brief GameMapHeaderModel::~GameMapHeaderModel
 */

GameMapHeaderModel::~GameMapHeaderModel()
{
	if (m_mapModel)
		m_mapModel->deleteHeaderModel(this);
}


/**
 * @brief GameMapHeaderModel::rowCount
 * @return
 */

int GameMapHeaderModel::rowCount(const QModelIndex &modelIndex) const
{
	if (m_mapModel) {
		if (m_orientation == Qt::Horizontal)
			return 1;
		else
			return m_mapModel->rowCount(modelIndex);
	} else
		return 0;
}


/**
 * @brief GameMapHeaderModel::columnCount
 * @return
 */

int GameMapHeaderModel::columnCount(const QModelIndex &modelIndex) const
{
	if (m_mapModel) {
		if (m_orientation == Qt::Vertical)
			return 1;
		else
			return m_mapModel->columnCount(modelIndex);
	} else
		return 0;
}


/**
 * @brief GameMapHeaderModel::data
 * @param index
 * @param role
 * @return
 */

QVariant GameMapHeaderModel::data(const QModelIndex &index, int role) const
{
	int col = index.column();
	int row = index.row();

	QHash<int, QByteArray> roles = roleNames();

	if (roles.isEmpty() || !m_mapModel)
		return QVariant();

	QByteArray roleName = roles.value(role);

	if (roleName == "display") {
		if (m_orientation == Qt::Horizontal) {
			GameMapModel::MissionLevel ml = m_mapModel->missionLevelAt(col);
			QVariantMap r;
			r["level"] = ml.level;
			r["deathmatch"] = ml.deathmatch;
			return r;
		} else {
			if (row >= 0 && row < m_mapModel->users().size()) {
				QVariantMap r;
				GameMapModel::User u = m_mapModel->users().at(row);
				r["firstname"] = u.firstname;
				r["lastname"] = u.lastname;
				r["xp"] = u.xp;
				return r;
			}
		}
	}


	return QVariant();
}


/**
 * @brief GameMapHeaderModel::roleNames
 * @return
 */

QHash<int, QByteArray> GameMapHeaderModel::roleNames() const
{
	if (m_mapModel)
		return m_mapModel->roleNames();
	else
		return QHash<int, QByteArray>();
}



/**
 * @brief GameMapHeaderModel::onDeleteParent
 */

void GameMapHeaderModel::onDeleteParent()
{
	m_mapModel = nullptr;
}




/**
 * @brief GameMapHeaderModel::onRemoveRows
 * @param first
 * @param last
 */

void GameMapHeaderModel::onBeginRemoveRows(const int &first, const int &last)
{
	if (m_orientation == Qt::Vertical)
		beginRemoveRows(QModelIndex(), first, last);
}


/**
 * @brief GameMapHeaderModel::onEndRemoveRows
 * @param first
 * @param last
 */

void GameMapHeaderModel::onEndRemoveRows()
{
	if (m_orientation == Qt::Vertical)
		endRemoveRows();
}



/**
 * @brief GameMapHeaderModel::onBeginRemoveColumns
 * @param first
 * @param last
 */

void GameMapHeaderModel::onBeginRemoveColumns(const int &first, const int &last)
{
	if (m_orientation == Qt::Horizontal)
		beginRemoveColumns(QModelIndex(), first, last);
}


/**
 * @brief GameMapHeaderModel::onEndRemoveColumns
 */

void GameMapHeaderModel::onEndRemoveColumns()
{
	if (m_orientation == Qt::Horizontal)
		endRemoveColumns();
}


/**
 * @brief GameMapHeaderModel::onBeginInsertRows
 * @param first
 * @param last
 */

void GameMapHeaderModel::onBeginInsertRows(const int &first, const int &last)
{
	if (m_orientation == Qt::Vertical)
		beginInsertRows(QModelIndex(), first, last);
}


/**
 * @brief GameMapHeaderModel::onEndInsertRows
 */

void GameMapHeaderModel::onEndInsertRows()
{
	if (m_orientation == Qt::Vertical)
		endInsertRows();
}


/**
 * @brief GameMapHeaderModel::onBeginInsertColumns
 * @param first
 * @param last
 */

void GameMapHeaderModel::onBeginInsertColumns(const int &first, const int &last)
{
	if (m_orientation == Qt::Horizontal)
		beginInsertColumns(QModelIndex(), first, last);
}

/**
 * @brief GameMapHeaderModel::onEndInsertColumns
 */

void GameMapHeaderModel::onEndInsertColumns()
{
	if (m_orientation == Qt::Horizontal)
		endInsertColumns();
}
