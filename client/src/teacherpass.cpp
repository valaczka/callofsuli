/*
 * ---- Call of Suli ----
 *
 * teacherpass.cpp
 *
 * Created on: 2025. 08. 01.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * TeacherPass
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

#include "teacherpass.h"
#include "application.h"

#include <utils_.h>


TeacherPass::TeacherPass(QObject *parent)
	: QObject{parent}
	, m_passResultHash(TeacherPassResultModel::ModePass)
	, m_passItemResultHash(TeacherPassResultModel::ModePassItem)
	, m_client(Application::instance()->client())
	, m_passList(new PassList(this))
	, m_categoryList(new QSListModel(this))
	, m_userList(new UserList(this))
{
	m_categoryList->setRoleNames(Utils::getRolesFromObject(Category().metaObject()));
}



TeacherPass::~TeacherPass()
{
	for (TeacherPassResultModel *model : m_models) {
		model->setTeacherPass(nullptr);
	}
}


/**
 * @brief TeacherPass::reload
 */

void TeacherPass::reload()
{
	if (!m_teacherGroup)
		return;

	m_client->send(HttpConnection::ApiTeacher, QStringLiteral("group/%1/pass").arg(m_teacherGroup->groupid()))
			->done(this, [this](const QJsonObject &obj){
		QMutexLocker locker(&m_mutex);
		m_client->callReloadHandler(QStringLiteral("pass"), m_passList.get(), obj.value(QStringLiteral("list")).toArray());

		if (obj.contains(QStringLiteral("categories")))
			reloadCategoryList(obj.value(QStringLiteral("categories")).toArray());
	})
			->fail(this, [this](const QString &err){m_client->messageWarning(err, tr("Letöltési hiba"));});
}



/**
 * @brief TeacherPass::reloadCategories
 */

void TeacherPass::reloadCategories()
{
	m_client->send(HttpConnection::ApiTeacher, QStringLiteral("pass/categories"))
			->done(this, [this](const QJsonObject &obj){
		reloadCategoryList(obj.value(QStringLiteral("list")).toArray());
	})
			->fail(this, [this](const QString &err){m_client->messageWarning(err, tr("Letöltési hiba"));});
}



/**
 * @brief TeacherPass::reloadPassResult
 * @param pass
 */

void TeacherPass::reloadPassResult(Pass *pass)
{
	if (!pass)
		return;

	m_client->send(HttpConnection::ApiTeacher, QStringLiteral("pass/%1/result").arg(pass->passid()))
			->done(this, [this, i = QPointer(pass)](const QJsonObject &obj){
		if (!i)
			return;

		updatePassItemResult(obj.value(QStringLiteral("items")).toArray());
		updatePassResult(obj.value(QStringLiteral("sum")).toArray(), i->grading());

		emit passResultReloaded(i);
	})
			->fail(this, [this](const QString &err){m_client->messageWarning(err, tr("Letöltési hiba"));});
}



/**
 * @brief TeacherPass::reloadPassItemResult
 * @param item
 */

void TeacherPass::reloadPassItemResult(PassItem *item)
{
	if (!item)
		return;

	m_client->send(HttpConnection::ApiTeacher, QStringLiteral("passItem/%1/result").arg(item->itemid()))
			->done(this, [this, i = QPointer(item)](const QJsonObject &obj){
		updatePassItemResult(obj.value(QStringLiteral("list")).toArray());

		emit passItemResultReloaded(i);
	})
			->fail(this, [this](const QString &err){m_client->messageWarning(err, tr("Letöltési hiba"));});
}


/**
 * @brief TeacherPass::addChild
 * @param dest
 * @param passId
 */

bool TeacherPass::addChild(Pass *dest, const int &passId)
{
	if (!dest || passId <= 0)
		return false;

	if (dest->passid() == passId)
		return false;

	if (OlmLoader::find(dest->itemList(), "includePass", passId)) {
		LOG_CWARNING("client") << "Pass already include" << passId << "in" << dest->passid();
		return false;
	}

	m_client->send(HttpConnection::ApiTeacher, QStringLiteral("pass/%1/create").arg(dest->passid()), {
					   { QStringLiteral("includepass"), passId }
				   })
			->done(this, [d = QPointer(dest)](const QJsonObject &){
		d->reload(HttpConnection::ApiTeacher);
	})
			->fail(this, [this](const QString &err){m_client->messageWarning(err, tr("Sikertelen művelet"));});

	return true;
}

/**
 * @brief TeacherPass::teacherGroup
 * @return
 */

TeacherGroup *TeacherPass::teacherGroup() const
{
	return m_teacherGroup;
}

void TeacherPass::setTeacherGroup(TeacherGroup *newTeacherGroup)
{
	if (m_teacherGroup == newTeacherGroup)
		return;

	if (!newTeacherGroup)
		m_teacherGroup->disconnect(this);

	m_teacherGroup = newTeacherGroup;
	emit teacherGroupChanged();

	if (m_teacherGroup) {
		connect(m_teacherGroup, &TeacherGroup::memberListReloaded, this, &TeacherPass::reloadUserList);
		reloadUserList();
	}
}

PassList *TeacherPass::passList() const
{
	return m_passList.get();
}

QSListModel *TeacherPass::categoryList() const
{
	return m_categoryList.get();
}

UserList *TeacherPass::userList() const
{
	return m_userList.get();
}


/**
 * @brief TeacherPass::notifyModels
 * @param mode
 * @param contentId
 * @param username
 */

void TeacherPass::notifyModels(const TeacherPassResultModel::Mode &mode, const int &contentId, const QString &username)
{
	emit userDataChanged(mode, contentId, username);

	for (TeacherPassResultModel *model : m_models) {
		if (!model)
			continue;

		model->onContentModified(mode, contentId, username);
	}
}




/**
 * @brief TeacherPass::setResult
 * @param dest
 * @param contentId
 * @param username
 * @param result
 */

void TeacherPass::setResult(ResultHash &dest, const int &contentId, const QString &username, const Result &result)
{
	dest[qMakePair(contentId, username)] = result;
	notifyModels(dest.mode(), contentId, username);
}



/**
 * @brief TeacherPass::updateResult
 * @param dest
 * @param list
 */

QSet<int> TeacherPass::updateResult(ResultHash *dest, const QJsonArray &list, const char *fieldId,
									const QJsonObject &grading, const bool &calculateResult)
{
	Q_ASSERT(dest);

	//QSet<QPair<int, QString> > tmp;
	QSet<int> updatedIds;

	/*for (auto it=dest->constBegin(); it != dest->constEnd(); ++it)
		tmp.insert(it.key());*/


	for (const QJsonValue &v : list) {
		const QJsonObject &o = v.toObject();

		const int id = o.value(fieldId).toInt(-1);
		const QString username = o.value(QStringLiteral("username")).toString();

		if (id < 0 || username.isEmpty()) {
			LOG_CWARNING("client") << "Invalid result data" << o;
			continue;
		}

		updatedIds.insert(id);

		//tmp.remove(qMakePair(id, username));

		setResult(*dest, id, username, Result().fromJson(o, calculateResult).fromGrading(grading));
	}

	/*for (const auto &pair : tmp) {
		dest->remove(pair);
		notifyModels(dest->mode(), pair.first, pair.second);
	}*/

	return updatedIds;
}




/**
 * @brief TeacherPass::reloadUserList
 */

bool TeacherPass::reloadUserList()
{
	QMutexLocker locker(&m_mutex);

	if (!m_teacherGroup) {
		m_userList->clear();
		emit userListReloaded();
		return false;
	}

	bool rqReload = false;

	QSet<User *> tmp;

	tmp.reserve(m_userList->size());

	for (User *u : *m_userList.get())
		tmp.insert(u);

	for (User *u : *m_teacherGroup->memberList()) {
		User *user = OlmLoader::find<User>(m_userList.get(), "username", u->username());

		if (user)
			tmp.remove(user);
		else {
			rqReload = true;

			user = new User();
			m_userList->append(user);
		}

		user->loadFromJson(u->toJsonObject(), true);
	}

	for (User *u : tmp)
		m_userList->remove(u);

	emit userListReloaded();

	return rqReload;
}



/**
 * @brief TeacherPass::reloadCategoryList
 * @param list
 */

void TeacherPass::reloadCategoryList(const QJsonArray &list)
{
	QMutexLocker locker(&m_mutex);

	QJsonArray l = list;

	l.prepend(Category(0, tr("-- kategória nélkül --")).toJson());

	Utils::patchSListModel(m_categoryList.get(), l.toVariantList(), QStringLiteral("id"));
}




/**
 * @brief TeacherPassResultModel::TeacherPassResultModel
 * @param parent
 */

TeacherPassResultModel::TeacherPassResultModel(QObject *parent)
	: QAbstractTableModel(parent)
{

}


/**
 * @brief TeacherPassResultModel::~TeacherPassResultModel
 */

TeacherPassResultModel::~TeacherPassResultModel()
{
	if (m_teacherPass)
		m_teacherPass->removeModel(this);
}


/**
 * @brief TeacherPassResultModel::rowCount
 * @return
 */

int TeacherPassResultModel::rowCount(const QModelIndex &) const
{
	return m_showHeaderPlaceholders ? 10 : m_userList.size()+1;
}


/**
 * @brief TeacherPassResultModel::columnCount
 * @return
 */

int TeacherPassResultModel::columnCount(const QModelIndex &) const
{
	if (m_showHeaderPlaceholders)
		return 5;

	switch (m_mode) {
		case ModePass:
			return m_passItemList.size()+2;
			break;
		case ModePassItem:
			return m_passItemList.size()+1;
			break;
		default:
			return 0;
	}
}


/**
 * @brief TeacherPassResultModel::data
 * @param index
 * @param role
 * @return
 */

QVariant TeacherPassResultModel::data(const QModelIndex &index, int role) const
{
	const int &col = index.column();
	const int &row = index.row();

	switch (m_mode) {
		case ModePass:
			return dataAsPass(row, col, role);
			break;
		case ModePassItem:
			return dataAsPassItem(row, col, role);
			break;
		default:
			return QVariant();
	}
}


/**
 * @brief TeacherPassResultModel::roleNames
 * @return
 */

QHash<int, QByteArray> TeacherPassResultModel::roleNames() const
{
	return {
		{ Qt::DisplayRole, QByteArrayLiteral("display") },
		//{ Qt::CheckStateRole, QByteArrayLiteral("selected") },
		{ Qt::UserRole+1, QByteArrayLiteral("passUser") },
		{ Qt::UserRole+2, QByteArrayLiteral("isPlaceholder") },
		{ Qt::UserRole+3, QByteArrayLiteral("pass") },
		{ Qt::UserRole+4, QByteArrayLiteral("passItem") },
		{ Qt::UserRole+5, QByteArrayLiteral("result") },
	};
}




/**
 * @brief TeacherPassResultModel::getUserData
 * @param username
 * @return
 */

QVariantMap TeacherPassResultModel::getUserData(const User *user) const
{
	return getUserData(user ? user->username() : QString(), m_contentId);
}



/**
 * @brief TeacherPassResultModel::teacherPass
 * @return
 */

TeacherPass *TeacherPassResultModel::teacherPass() const
{
	return m_teacherPass;
}

void TeacherPassResultModel::setTeacherPass(TeacherPass *newTeacherPass)
{
	if (m_teacherPass == newTeacherPass)
		return;

	if (m_teacherPass) {
		m_teacherPass->disconnect(this);
		m_teacherPass->removeModel(this);
	}

	m_teacherPass = newTeacherPass;
	emit teacherPassChanged();

	if (m_teacherPass) {
		connect(m_teacherPass, &TeacherPass::userListReloaded, this, &TeacherPassResultModel::reload);
		/*connect(m_teacherPass, &TeacherPass::passResultReloaded, this, [this](Pass *){
			setShowCellPlaceholders(false);
		});
		connect(m_teacherPass, &TeacherPass::passItemResultReloaded, this, [this](PassItem *){
			setShowCellPlaceholders(false);
		});*/
		m_teacherPass->addModel(this);
		reload();
	}
}


/**
 * @brief TeacherPassResultModel::mode
 * @return
 */

TeacherPassResultModel::Mode TeacherPassResultModel::mode() const
{
	return m_mode;
}

void TeacherPassResultModel::setMode(const Mode &newMode)
{
	if (m_mode == newMode)
		return;
	m_mode = newMode;
	emit modeChanged();
}

int TeacherPassResultModel::contentId() const
{
	return m_contentId;
}

void TeacherPassResultModel::setContentId(int newContentId)
{
	if (m_contentId == newContentId)
		return;
	m_contentId = newContentId;
	emit contentIdChanged();

	reload();
}


/**
 * @brief TeacherPassResultModel::reload
 */

void TeacherPassResultModel::reload()
{
	LOG_CWARNING("client") << "Reload model" << this << m_teacherPass << m_mode << m_contentId;

	if (!m_teacherPass)
		return;

	QMutexLocker locker(&m_teacherPass->m_mutex);

	beginRemoveColumns(Utils::noParent(), 0, columnCount()-1);
	m_passList.clear();
	m_passItemList.clear();
	endRemoveColumns();

	beginRemoveRows(Utils::noParent(), 0, rowCount()-1);
	m_userList.clear();
	endRemoveRows();

	setShowHeaderPlaceholders(false);
	setShowCellPlaceholders(false);

	if (m_teacherPass->m_userList.get()->size()) {
		QHash<QString, User*> tmp;

		beginInsertRows(Utils::noParent(), 0, m_teacherPass->m_userList.get()->size()+1);

		for (User *u : *m_teacherPass->m_userList.get()) {
			if (!u)
				continue;

			tmp.insert(u->username(), u);
			m_userList.append(u->username());
		}

		std::sort(m_userList.begin(), m_userList.end(), [&tmp](const QString &l, const QString &r) {
			User *left = tmp.value(l);
			User *right = tmp.value(r);

			if (!left || !right)
				return true;

			return (QString::localeAwareCompare(left->fullName(), right->fullName()) < 0);
		});

		endInsertRows();
	}


	if (m_mode == ModePassItem || m_mode == ModePass) {
		Pass *pass = OlmLoader::find<Pass>(m_teacherPass->m_passList.get(), "passid", m_contentId);
		beginInsertColumns(Utils::noParent(), 0,
						   (pass ? pass->itemList()->size() : 0) + (m_mode == ModePassItem ? 1 : 2));

		if (pass && !pass->itemList()->empty()) {
			for (PassItem *i : *pass->itemList()) {
				if (i->includePass() <= 0)
					m_passItemList.append(i->itemid());
			}

			std::sort(m_passItemList.begin(), m_passItemList.end());
		}

		endInsertColumns();
	}

	emit modelReloaded();
}



/**
 * @brief TeacherPassResultModel::dataAsPass
 * @param row
 * @param column
 * @param role
 * @return
 */

QVariant TeacherPassResultModel::dataAsPass(const int &row, const int &column, int role) const
{
	if (role == Qt::DisplayRole) {											// display
		return QString();

	} else if (role == Qt::CheckStateRole) {								// selected

		return 0;


	} else if (role == Qt::UserRole+1) {									// passUser
		if (!m_teacherPass || row <= 0 || row > m_userList.size())
			return QVariant::fromValue(nullptr);

		QMutexLocker locker(&m_teacherPass->m_mutex);
		return QVariant::fromValue(OlmLoader::find<User>(m_teacherPass->userList(), "username", m_userList.at(row-1)));

	} else if (role == Qt::UserRole+2) {									// isPlacholder
		if (column == 0 || row == 0)
			return m_showHeaderPlaceholders;
		else
			return m_showCellPlaceholders;

	} else if (role == Qt::UserRole+3) {									// pass
		if (!m_teacherPass || m_contentId <= 0)
			return QVariant::fromValue(nullptr);
		else {
			QMutexLocker locker(&m_teacherPass->m_mutex);
			return QVariant::fromValue(OlmLoader::find<Pass>(m_teacherPass->m_passList.get(), "passid", m_contentId));
		}

	} else if (role == Qt::UserRole+4) {									// passItem
		if (!m_teacherPass || column < 2 || column > m_passItemList.size()+1)
			return QVariant::fromValue(nullptr);
		else {
			QMutexLocker locker(&m_teacherPass->m_mutex);
			Pass *pass = OlmLoader::find<Pass>(m_teacherPass->m_passList.get(), "passid", m_contentId);
			if (!pass)
				return QVariant::fromValue(nullptr);
			else
				return QVariant::fromValue(OlmLoader::find<PassItem>(pass->itemList(), "itemid", m_passItemList.at(column-2)));
		}

	} else if (role == Qt::UserRole+5) {									// result
		if (!m_teacherPass || column < 1 || column > m_passItemList.size()+1 || row <= 0 || row > m_userList.size())
			return getUserData(QString(), -1);
		else if (column == 1)
			return getPassData(m_userList.at(row-1), m_contentId);
		else
			return getUserData(m_userList.at(row-1), m_passItemList.at(column-2));
	}

	return QVariant();
}




/**
 * @brief TeacherPassResultModel::dataAsPassItem
 * @param row
 * @param column
 * @param role
 * @return
 */

QVariant TeacherPassResultModel::dataAsPassItem(const int &row, const int &column, int role) const
{
	if (role == Qt::DisplayRole) {											// display
		return QString();

	} else if (role == Qt::CheckStateRole) {								// selected

		return 0;


	} else if (role == Qt::UserRole+1) {									// passUser
		if (!m_teacherPass || row <= 0 || row > m_userList.size())
			return QVariant::fromValue(nullptr);

		QMutexLocker locker(&m_teacherPass->m_mutex);
		return QVariant::fromValue(OlmLoader::find<User>(m_teacherPass->userList(), "username", m_userList.at(row-1)));

	} else if (role == Qt::UserRole+2) {									// isPlacholder
		if (column == 0 || row == 0)
			return m_showHeaderPlaceholders;
		else
			return m_showCellPlaceholders;

	} else if (role == Qt::UserRole+3) {									// pass
		//if (m_passList.isEmpty())
		return QVariant::fromValue(nullptr);
		//else
		//	return QVariant::fromValue(m_passList.first());

	} else if (role == Qt::UserRole+4) {									// passItem
		//if (column < 2 || column > m_passItemList.size()+1)
		return QVariant::fromValue(nullptr);
		//else
		//	return QVariant::fromValue(m_passItemList.at(column-2));

	} else if (role == Qt::UserRole+5) {									// result
		return getUserData(QString(), -1);
	}
	return QVariant();
}




/**
 * @brief TeacherPassResultModel::getUserData
 * @param username
 * @param itemid
 * @return
 */

QVariantMap TeacherPassResultModel::getUserData(const QString &username, const int &itemid) const
{
	QVariantMap m = TeacherPass::Result().toVariantMap();
	m.insert(QStringLiteral("assigned"), false);

	if (username.isEmpty() || !m_teacherPass || itemid <= 0)
		return m;

	QMutexLocker locker(&m_teacherPass->m_mutex);
	if (const auto &ptr = m_teacherPass->getPassItemResult(itemid, username)) {
		m = ptr->toVariantMap();
		m.insert(QStringLiteral("assigned"), true);
	}

	return m;
}


/**
 * @brief TeacherPassResultModel::getPassData
 * @param username
 * @param passid
 * @return
 */

QVariantMap TeacherPassResultModel::getPassData(const QString &username, const int &passid) const
{
	QVariantMap m = TeacherPass::Result().toVariantMap();
	m.insert(QStringLiteral("assigned"), false);

	if (username.isEmpty() || !m_teacherPass || passid <= 0)
		return m;

	QMutexLocker locker(&m_teacherPass->m_mutex);
	if (const auto &ptr = m_teacherPass->getPassResult(passid, username)) {
		m = ptr->toVariantMap();
		m.insert(QStringLiteral("assigned"), true);
	}

	return m;
}




/**
 * @brief TeacherPassResultModel::showHeaderPlaceholders
 * @return
 */

bool TeacherPassResultModel::showHeaderPlaceholders() const
{
	return m_showHeaderPlaceholders;
}

void TeacherPassResultModel::setShowHeaderPlaceholders(bool newShowHeaderPlaceholders)
{
	if (m_showHeaderPlaceholders == newShowHeaderPlaceholders)
		return;

	const int c = columnCount();
	const int r = rowCount();

	m_showHeaderPlaceholders = newShowHeaderPlaceholders;
	emit showHeaderPlaceholdersChanged();

	if (c)
		emit dataChanged(index(0, 0), index(0, c-1));

	if (r)
		emit dataChanged(index(0, 0), index(r-1, 0));

}




bool TeacherPassResultModel::showCellPlaceholders() const
{
	return m_showCellPlaceholders;
}

void TeacherPassResultModel::setShowCellPlaceholders(bool newShowCellPlaceholders)
{
	if (m_showCellPlaceholders == newShowCellPlaceholders)
		return;


	const int c = columnCount()-1;
	const int r = rowCount()-1;

	m_showCellPlaceholders = newShowCellPlaceholders;
	emit showCellPlaceholdersChanged();

	if (c > 0 && r > 0)
		emit dataChanged(index(1, 1), index(c, r));


}



/**
 * @brief TeacherPassResultModel::onContentModified
 * @param mode
 * @param contentId
 * @param username
 */

void TeacherPassResultModel::onContentModified(const Mode &mode, const int &contentId, const QString &username)
{
	if (m_mode == mode && contentId != m_contentId)
		return;

	int row = m_userList.indexOf(username);

	if (row == -1)
		return reload();
	else
		++row;

	if (m_mode == ModePass) {
		if (mode == ModePass) {
			return reload();
		} else if (mode == ModePassItem) {
			if (int col = m_passItemList.indexOf(contentId); col != -1) {
				emit dataChanged(index(row, col+2), index(row, col+2));
			}
		}
	}
}





/**
 * @brief TeacherPass::Result::fromJson
 * @param obj
 */

TeacherPass::Result &TeacherPass::Result::fromJson(const QJsonObject &obj, const bool &calculateResult)
{
	if (obj.contains(QStringLiteral("pts")))
		pts = obj.value(QStringLiteral("pts")).toDouble();

	if (obj.contains(QStringLiteral("maxPts")))
		maxPts = obj.value(QStringLiteral("maxPts")).toDouble();

	if (calculateResult)
		result = maxPts > 0 ? pts/maxPts : 0.;
	else if (obj.contains(QStringLiteral("result")))
		result = obj.value(QStringLiteral("result")).toDouble();

	return *this;
}



/**
 * @brief TeacherPass::Result::fromGrading
 * @param grading
 */

TeacherPass::Result &TeacherPass::Result::fromGrading(const QJsonObject &grading)
{
	QList<Grade*> list = Pass::getGrades(result, grading,
										 dynamic_cast<GradeList*>(Application::instance()->client()->cache(QStringLiteral("gradeList"))));

	grades.clear();

	for (Grade *g : list)
		grades.append(g);

	return *this;
}



/**
 * @brief TeacherPass::Result::toVariantMap
 * @return
 */

QVariantMap TeacherPass::Result::toVariantMap() const
{
	QVariantMap m;

	m.insert(QStringLiteral("pts"), pts);
	m.insert(QStringLiteral("maxPts"), maxPts);
	m.insert(QStringLiteral("result"), Pass::round(result));

	QVariantList l;

	for (Grade *g : grades)
		l.append(QVariant::fromValue(g));

	m.insert(QStringLiteral("grades"), l);

	return m;
}
