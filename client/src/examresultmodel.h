/*
 * ---- Call of Suli ----
 *
 * examresultmodel.h
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

#ifndef EXAMRESULTMODEL_H
#define EXAMRESULTMODEL_H

#include "exam.h"
#include "teachergroup.h"
#include <QAbstractTableModel>


/**
 * @brief The ExamResultResult class
 */

class ExamResultResult {
	Q_GADGET

	Q_PROPERTY(Grade* grade MEMBER grade);
	Q_PROPERTY(int xp MEMBER xp);
	Q_PROPERTY(int picked MEMBER picked);
	Q_PROPERTY(bool joker MEMBER joker);
	Q_PROPERTY(qreal result MEMBER result);

public:
	Grade *grade = nullptr;
	int xp = 0;
	int picked = -1;
	bool joker = false;
	qreal result = 0;
};


Q_DECLARE_METATYPE(ExamResultResult)



/**
 * @brief The ExamResultModel class
 */

class ExamResultModel : public QAbstractTableModel
{
	Q_OBJECT

	Q_PROPERTY(TeacherGroup *teacherGroup READ teacherGroup WRITE setTeacherGroup NOTIFY teacherGroupChanged FINAL)
	Q_PROPERTY(ExamList *groupExamList READ groupExamList WRITE setGroupExamList NOTIFY groupExamListChanged FINAL)
	Q_PROPERTY(bool showHeaderPlaceholders READ showHeaderPlaceholders WRITE setShowHeaderPlaceholders NOTIFY showHeaderPlaceholdersChanged FINAL)
	Q_PROPERTY(bool showCellPlaceholders READ showCellPlaceholders WRITE setShowCellPlaceholders NOTIFY showCellPlaceholdersChanged FINAL)

public:
	explicit ExamResultModel(QObject *parent = nullptr);

	struct ExamResult {
		QPointer<User> user;
		QPointer<Exam> exam;
		ExamResultResult result;
	};

	int rowCount(const QModelIndex & = QModelIndex()) const override { return m_showHeaderPlaceholders ? 10 : m_userList.size()+1; }
	int columnCount(const QModelIndex & = QModelIndex()) const override { return m_showHeaderPlaceholders ? 5 : m_examList.size()+1; }
	QVariant data(const QModelIndex &index, int role) const override;
	QHash<int, QByteArray> roleNames() const override;

	Q_INVOKABLE User* userAt(const int &row) const;
	Q_INVOKABLE Exam* examAt(const int &column) const;

	TeacherGroup *teacherGroup() const;
	void setTeacherGroup(TeacherGroup *newTeacherGroup);

	bool showHeaderPlaceholders() const;
	void setShowHeaderPlaceholders(bool newShowHeaderPlaceholders);

	bool showCellPlaceholders() const;
	void setShowCellPlaceholders(bool newShowCellPlaceholders);

	ExamList *groupExamList() const;
	void setGroupExamList(ExamList *newGroupExamList);

	const QVector<ExamResult> &resultList() const { return m_resultList; }

public slots:
	void reload();
	void reloadContent();

signals:
	void modelReloaded();
	void contentReloaded();

	void teacherGroupChanged();
	void showHeaderPlaceholdersChanged();
	void showCellPlaceholdersChanged();
	void groupExamListChanged();

private:
	void reloadFromJson(const QJsonObject &data);

	int findResult(const User *user, const Exam *exam) const;
	int findResult(const QString &username, const int &examid) const;
	User *findUser(const QString &username) const;
	Exam *findExam(const int &id) const;

	TeacherGroup *m_teacherGroup = nullptr;
	ExamList *m_groupExamList = nullptr;
	QVector<QPointer<User>> m_userList;
	QVector<QPointer<Exam>> m_examList;
	QVector<ExamResult> m_resultList;

	bool m_showHeaderPlaceholders = true;
	bool m_showCellPlaceholders = true;
};


#endif // EXAMRESULTMODEL_H
