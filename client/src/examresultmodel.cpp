/*
 * ---- Call of Suli ----
 *
 * examresultmodel.cpp
 *
 * Created on: 2024. 01. 13.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ExamResultModel
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

#include "examresultmodel.h"
#include "application.h"
#include "utils_.h"

ExamResultModel::ExamResultModel(QObject *parent)
	: QAbstractTableModel{parent}
{

}



/**
 * @brief ExamResultModel::data
 * @param index
 * @param role
 * @return
 */

QVariant ExamResultModel::data(const QModelIndex &index, int role) const
{
	const int &col = index.column();
	const int &row = index.row();

	if (role == Qt::DisplayRole) {											// display
		if (col == 0) {
			if (m_showHeaderPlaceholders)
				return QStringLiteral("");

			if (row <= 0 || row > m_userList.size())
				return QStringLiteral("???");

			User *u = m_userList.at(row-1);
			return u ? u->fullName() : QStringLiteral("???");
		} else if (row == 0) {
			if (m_showHeaderPlaceholders)
				return QStringLiteral("");

			if (col <= 0 || col > m_examList.size())
				return QStringLiteral("???");

			Exam *c = m_examList.at(col-1);

			if (c) {
				if (c->description().isEmpty())
					return tr("Dolgozat #%1").arg(c->examId());
				else
					return c->description();
			} else {
				return QStringLiteral("???");
			}
		}

		return QStringLiteral("");

	} else if (role == Qt::CheckStateRole) {								// examState
		if (col <= 0 || col > m_examList.size())
			return Exam::Prepare;

		Exam *c = m_examList.at(col-1);

		return c ? c->state() : Exam::Prepare;


	} else if (role == Qt::UserRole+1) {									// result
		ExamResultResult r;

		if (row <= 0 || row > m_userList.size() || col <= 0 || col > m_examList.size())
			return QVariant::fromValue(r);

		User *user = m_userList.at(row-1);
		Exam *exam = m_examList.at(col-1);

		int idx = findResult(user, exam);

		if (idx != -1)
			return QVariant::fromValue(m_resultList.at(idx).result);

		return QVariant::fromValue(r);

	} else if (role == Qt::UserRole+2) {									// isPlacholder
		if (col == 0 || row == 0)
			return m_showHeaderPlaceholders;
		else
			return m_showCellPlaceholders;

	} else if (role == Qt::UserRole+3) {									// examMode
		if (col <= 0 || col > m_examList.size())
			return Exam::ExamPaper;

		Exam *c = m_examList.at(col-1);

		return c ? c->mode() : Exam::ExamPaper;
	}

	return QVariant();
}



/**
 * @brief ExamResultModel::roleNames
 * @return
 */

QHash<int, QByteArray> ExamResultModel::roleNames() const
{
	return {
		{ Qt::DisplayRole, QByteArrayLiteral("display") },
		{ Qt::CheckStateRole, QByteArrayLiteral("examState") },
		{ Qt::UserRole+1, QByteArrayLiteral("result") },
		{ Qt::UserRole+2, QByteArrayLiteral("isPlaceholder") },
		{ Qt::UserRole+3, QByteArrayLiteral("examMode") },
	};
}


/**
 * @brief ExamResultModel::userAt
 * @param row
 * @return
 */

User *ExamResultModel::userAt(const int &row) const
{
	if (row >= 0 && row < m_userList.size())
		return m_userList.at(row);

	return nullptr;
}



/**
 * @brief ExamResultModel::examAt
 * @param column
 * @return
 */

Exam *ExamResultModel::examAt(const int &column) const
{
	if (column >= 0 && column < m_examList.size())
		return m_examList.at(column);

	return nullptr;
}




/**
 * @brief ExamResultModel::reload
 */

void ExamResultModel::reload()
{
	if (!m_teacherGroup)
		return;

	LOG_CDEBUG("client") << "Reload model" << this;

	beginRemoveColumns(Utils::noParent(), 0, columnCount()-1);
	m_examList.clear();
	endRemoveColumns();

	beginRemoveRows(Utils::noParent(), 0, rowCount()-1);
	m_userList.clear();
	endRemoveRows();

	setShowHeaderPlaceholders(false);

	m_resultList.clear();

	if (m_teacherGroup->memberList()->size()) {
		beginInsertRows(Utils::noParent(), 0, m_teacherGroup->memberList()->size()+1);

		for (User *u : *m_teacherGroup->memberList())
			m_userList.append(u);

		std::sort(m_userList.begin(), m_userList.end(), [](User *left, User *right) {
			if (!left || !right)
				return true;

			return (QString::localeAwareCompare(left->fullName(), right->fullName()) < 0);
		});

		endInsertRows();
	}

	if (m_groupExamList && m_groupExamList->size()) {
		beginInsertColumns(Utils::noParent(), 0, m_groupExamList->size()+1);

		for (Exam *c : *m_groupExamList) {
			if (c->state() >= Exam::Active)
				m_examList.append(c);
		}


		std::sort(m_examList.begin(), m_examList.end(), [](Exam *left, Exam *right) {
			if (!left || !right)
				return true;

			/*if (left->state() < Exam::Active && right->state() >= Exam::Active)
				return false;

			if (left->state() >= Exam::Active && right->state() < Exam::Active)
				return true;*/

			if (!left->timestamp().isValid() && right->timestamp().isValid())
				return false;

			if (left->timestamp().isValid() && !right->timestamp().isValid())
				return true;

			if (left->timestamp().isValid() && right->timestamp().isValid() && left->timestamp() < right->timestamp())
				return true;

			if (left->timestamp().isValid() && right->timestamp().isValid() && left->timestamp() > right->timestamp())
				return false;

			return (left->examId() < right->examId());
		});

		endInsertColumns();
	}

	emit modelReloaded();

	reloadContent();
}





/**
 * @brief ExamResultModel::reloadFromJson
 * @param data
 */

void ExamResultModel::reloadContent()
{
	if (!m_teacherGroup) {
		LOG_CWARNING("client") << "Missing Campaign or TeacherGroup";
		return;
	}

	Client *client = Application::instance()->client();

	client->send(HttpConnection::ApiTeacher, QStringLiteral("group/%1/exam/result").arg(m_teacherGroup->groupid()))
			->fail(client, [client](const QString &err){client->messageWarning(err, tr("Letöltési hiba"));})
			->done(this, &ExamResultModel::reloadFromJson);
}




/**
 * @brief ExamResultModel::reloadFromJson
 * @param data
 */

void ExamResultModel::reloadFromJson(const QJsonObject &data)
{
	m_resultList.clear();

	if (!m_groupExamList)
		return;


	const QJsonArray &list = data.value(QStringLiteral("list")).toArray();

	for (const QJsonValue &v : std::as_const(list)) {
		const QJsonObject &obj = v.toObject();

		const int examid = obj.value(QStringLiteral("examid")).toInt();
		const QString &username = obj.value(QStringLiteral("username")).toString();
		const double result = obj.value(QStringLiteral("result")).toDouble();
		const int gradeid = obj.value(QStringLiteral("gradeid")).toInt();
		const int state = obj.value(QStringLiteral("state")).toInt();
		const QJsonArray &examData = obj.value(QStringLiteral("data")).toArray();

		if (state < Exam::Active || username.isEmpty() || examid <= 0)
			continue;

		Grade *grade = qobject_cast<Grade*>(Application::instance()->client()->findCacheObject(QStringLiteral("gradeList"), gradeid));

		ExamResultResult r;

		r.grade = grade;
		r.result = result;

		if (!examData.isEmpty()) {
			const QJsonObject &o = examData.at(0).toObject();

			r.picked = o.value(QStringLiteral("picked")).toBool();
			r.joker = o.value(QStringLiteral("joker")).toBool();
		}

		int idx = findResult(username, examid);

		if (idx != -1) {
			m_resultList[idx].result = r;
		} else {
			Exam *exam = findExam(examid);
			User *user = findUser(username);

			if (!exam) {
				LOG_CWARNING("client") << "Invalid exam:" << examid;
				continue;
			}

			if (!user) {
				LOG_CWARNING("client") << "Invalid user:" << username;
				continue;
			}

			m_resultList.append({user, exam, r});
		}
	}

	setShowCellPlaceholders(false);

	emit dataChanged(index(0, 0), index(m_userList.size(), m_examList.size()));

}




/**
 * @brief ExamResultModel::findResult
 * @param user
 * @param exam
 * @return
 */

int ExamResultModel::findResult(const User *user, const Exam *exam) const
{
	for (int i=0; i<m_resultList.size(); ++i) {
		const ExamResult &r = m_resultList.at(i);

		if (r.user && r.exam && r.user == user && r.exam == exam)
			return i;
	}

	return -1;
}



/**
 * @brief ExamResultModel::findResult
 * @param username
 * @param examid
 * @return
 */

int ExamResultModel::findResult(const QString &username, const int &examid) const
{
	for (int i=0; i<m_resultList.size(); ++i) {
		const ExamResult &r = m_resultList.at(i);

		if (r.user && r.exam && r.user->username() == username && r.exam->examId() == examid)
			return i;
	}

	return -1;
}



/**
 * @brief ExamResultModel::findUser
 * @param username
 * @return
 */

User *ExamResultModel::findUser(const QString &username) const
{
	foreach (User *u, m_userList)
		if (u && u->username() == username)
			return u;

	return nullptr;
}




/**
 * @brief ExamResultModel::findExam
 * @param id
 * @return
 */

Exam *ExamResultModel::findExam(const int &id) const
{
	foreach (Exam *c, m_examList)
		if (c && c->examId() == id)
			return c;

	return nullptr;
}



/**
 * @brief ExamResultModel::groupExamList
 * @return
 */

ExamList *ExamResultModel::groupExamList() const
{
	return m_groupExamList;
}

void ExamResultModel::setGroupExamList(ExamList *newGroupExamList)
{
	if (m_groupExamList == newGroupExamList)
		return;
	m_groupExamList = newGroupExamList;
	emit groupExamListChanged();

	/*connect(m_groupExamList, &qolm::QOlmBase::countChanged, this, &ExamResultModel::reload);
	connect(m_groupExamList, &qolm::QOlmBase::objectRemoved, this, &ExamResultModel::reload);
	connect(m_groupExamList, &qolm::QOlmBase::objectMoved, this, &ExamResultModel::reload);*/
}

bool ExamResultModel::showCellPlaceholders() const
{
	return m_showCellPlaceholders;
}

void ExamResultModel::setShowCellPlaceholders(bool newShowCellPlaceholders)
{
	if (m_showCellPlaceholders == newShowCellPlaceholders)
		return;
	m_showCellPlaceholders = newShowCellPlaceholders;
	emit showCellPlaceholdersChanged();
}

bool ExamResultModel::showHeaderPlaceholders() const
{
	return m_showHeaderPlaceholders;
}

void ExamResultModel::setShowHeaderPlaceholders(bool newShowHeaderPlaceholders)
{
	if (m_showHeaderPlaceholders == newShowHeaderPlaceholders)
		return;
	m_showHeaderPlaceholders = newShowHeaderPlaceholders;
	emit showHeaderPlaceholdersChanged();
}

TeacherGroup *ExamResultModel::teacherGroup() const
{
	return m_teacherGroup;
}

void ExamResultModel::setTeacherGroup(TeacherGroup *newTeacherGroup)
{
	if (m_teacherGroup == newTeacherGroup)
		return;
	m_teacherGroup = newTeacherGroup;
	emit teacherGroupChanged();
}
