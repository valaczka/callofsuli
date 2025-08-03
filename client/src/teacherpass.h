/*
 * ---- Call of Suli ----
 *
 * teacherpass.h
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

#ifndef TEACHERPASS_H
#define TEACHERPASS_H

#include "pass.h"
#include "teachergroup.h"
#include <QObject>


class TeacherPass;

#ifndef OPAQUE_PTR_TeacherPass
#define OPAQUE_PTR_TeacherPass
Q_DECLARE_OPAQUE_POINTER(TeacherPass*)
#endif



/**
 * @brief The TeacherPassResultModel class
 */

class TeacherPassResultModel : public QAbstractTableModel
{
	Q_OBJECT

	Q_PROPERTY(TeacherPass *teacherPass READ teacherPass WRITE setTeacherPass NOTIFY teacherPassChanged FINAL)
	Q_PROPERTY(bool showHeaderPlaceholders READ showHeaderPlaceholders WRITE setShowHeaderPlaceholders NOTIFY showHeaderPlaceholdersChanged)
	Q_PROPERTY(bool showCellPlaceholders READ showCellPlaceholders WRITE setShowCellPlaceholders NOTIFY showCellPlaceholdersChanged)
	Q_PROPERTY(Mode mode READ mode WRITE setMode NOTIFY modeChanged FINAL)
	Q_PROPERTY(int contentId READ contentId WRITE setContentId NOTIFY contentIdChanged FINAL)

public:
	explicit TeacherPassResultModel(QObject *parent = nullptr);
	virtual ~TeacherPassResultModel();

	enum Mode {
		ModePass = 0,
		ModePassItem
	};

	Q_ENUM(Mode);

	int rowCount(const QModelIndex & = QModelIndex()) const override;
	int columnCount(const QModelIndex & = QModelIndex()) const override;
	QVariant data(const QModelIndex &index, int role) const override;
	QHash<int, QByteArray> roleNames() const override;

	Q_INVOKABLE QVariantMap getUserData(const User *user) const;

	TeacherPass *teacherPass() const;
	void setTeacherPass(TeacherPass *newTeacherPass);

	Mode mode() const;
	void setMode(const Mode &newMode);

	int contentId() const;
	void setContentId(int newContentId);

	bool showHeaderPlaceholders() const;
	void setShowHeaderPlaceholders(bool newShowHeaderPlaceholders);

	bool showCellPlaceholders() const;
	void setShowCellPlaceholders(bool newShowCellPlaceholders);

	void onContentModified(const Mode &mode, const int &contentId, const QString &username);

public slots:
	void reload();

signals:
	void modelReloaded();
	void teacherPassChanged();
	void showHeaderPlaceholdersChanged();
	void showCellPlaceholdersChanged();
	void modeChanged();
	void contentIdChanged();

private:
	QVariant dataAsPass(const int &row, const int &column, int role) const;
	QVariant dataAsPassItem(const int &row, const int &column, int role) const;
	QVariantMap getUserData(const QString &username, const int &itemid) const;
	QVariantMap getPassData(const QString &username, const int &passid) const;

	TeacherPass *m_teacherPass = nullptr;
	QStringList m_userList;
	QList<int> m_passList;
	QList<int> m_passItemList;

	bool m_showHeaderPlaceholders = true;
	bool m_showCellPlaceholders = true;

	Mode m_mode = ModePass;
	int m_contentId = -1;
};



/**
 * @brief The TeacherPass class
 */

class TeacherPass : public QObject
{
	Q_OBJECT

	Q_PROPERTY(TeacherGroup *teacherGroup READ teacherGroup WRITE setTeacherGroup NOTIFY teacherGroupChanged FINAL)
	Q_PROPERTY(PassList *passList READ passList CONSTANT FINAL)
	Q_PROPERTY(UserList *userList READ userList CONSTANT FINAL)
	Q_PROPERTY(QSListModel *categoryList READ categoryList CONSTANT FINAL)

public:
	class Category;

	explicit TeacherPass(QObject *parent = nullptr);
	virtual ~TeacherPass();

	Q_INVOKABLE void reload();
	Q_INVOKABLE void reloadCategories();
	Q_INVOKABLE void reloadPassResult(Pass *pass);
	Q_INVOKABLE void reloadPassItemResult(PassItem *item);

	Q_INVOKABLE bool addChild(Pass *dest, const int &passId);

	TeacherGroup *teacherGroup() const;
	void setTeacherGroup(TeacherGroup *newTeacherGroup);

	PassList *passList() const;

	QSListModel *categoryList() const;

	UserList *userList() const;

signals:
	void userListReloaded();
	void userDataChanged(const TeacherPassResultModel::Mode &mode, const int &id, const QString &username);
	void passResultReloaded(Pass *item);
	void passItemResultReloaded(PassItem *item);
	void teacherGroupChanged();

private:
	struct Result {
		qreal pts = 0.;
		qreal maxPts = 0.;
		qreal result = 0.;
		QList<QPointer<Grade>> grades;

		Result &fromJson(const QJsonObject &obj, const bool &calculateResult = false);
		Result &fromGrading(const QJsonObject &grading);
		QVariantMap toVariantMap() const;
	};

	class ResultHash : public QHash<QPair<int, QString>, Result>
	{
	public:
		ResultHash(const TeacherPassResultModel::Mode &mode)
			: QHash<QPair<int, QString>, Result>()
			, m_mode(mode)
		{}

		virtual ~ResultHash() {}

		const TeacherPassResultModel::Mode &mode() const { return m_mode; }

	private:
		const TeacherPassResultModel::Mode m_mode;
	};


	ResultHash m_passResultHash;
	ResultHash m_passItemResultHash;

	QSet<TeacherPassResultModel*> m_models;

	void addModel(TeacherPassResultModel *model) { if (model) m_models.insert(model); }
	void removeModel(TeacherPassResultModel *model) { if (model) m_models.remove(model); }
	void notifyModels(const TeacherPassResultModel::Mode &mode, const int &contentId, const QString &username);


	ResultHash::const_iterator findPass(const int &contentId, const QString &username) const {
		return m_passResultHash.find(qMakePair(contentId, username));
	}

	ResultHash::iterator findPass(const int &contentId, const QString &username) {
		return m_passResultHash.find(qMakePair(contentId, username));
	}


	ResultHash::const_iterator findPassItem(const int &contentId, const QString &username) const {
		return m_passItemResultHash.find(qMakePair(contentId, username));
	}

	ResultHash::iterator findPassItem(const int &contentId, const QString &username) {
		return m_passItemResultHash.find(qMakePair(contentId, username));
	}


	std::optional<Result> getPassResult(const int &contentId, const QString &username) const {
		if (const auto &it = findPass(contentId, username); it != m_passResultHash.constEnd())
			return *it;
		else
			return std::nullopt;
	}

	std::optional<Result> getPassItemResult(const int &contentId, const QString &username) const {
		if (const auto &it = findPassItem(contentId, username); it != m_passItemResultHash.constEnd())
			return *it;
		else
			return std::nullopt;
	}


	void setResult(ResultHash &dest, const int &contentId, const QString &username, const Result &result);

	void setPassResult(const int &contentId, const QString &username, const Result &result) {
		setResult(m_passResultHash, contentId, username, result);
	}

	void setPassItemResult(const int &contentId, const QString &username, const Result &result) {
		setResult(m_passItemResultHash, contentId, username, result);
	}

	QSet<int> updateResult(ResultHash *dest, const QJsonArray &list, const char *fieldId,
					  const QJsonObject &grading = {}, const bool &calculateResult = false);

	QSet<int> updatePassResult(const QJsonArray &list, const QJsonObject &grading) {
		return updateResult(&m_passResultHash, list, "passid", grading, true);
	}

	QSet<int> updatePassItemResult(const QJsonArray &list) {
		return updateResult(&m_passItemResultHash, list, "passitemid");
	}



	friend class TeacherPassResultModel;

private:
	bool reloadUserList();
	void reloadCategoryList(const QJsonArray &list);

	Client *const m_client;
	QPointer<TeacherGroup> m_teacherGroup;

	mutable QRecursiveMutex m_mutex;

	std::unique_ptr<PassList> m_passList;
	std::unique_ptr<QSListModel> m_categoryList;
	std::unique_ptr<UserList> m_userList;

};



/**
 * @brief The TeacherPass::Category class
 */

class TeacherPass::Category : public QSerializer
{
	Q_GADGET

public:
	Category(const int &_id = -1, const QString &_desc =  {})
		: QSerializer()
		, id(_id)
		, description(_desc)
	{}
	virtual ~Category() {}

	QS_SERIALIZABLE

	QS_FIELD(int, id)
	QS_FIELD(QString, description)
};

#endif // TEACHERPASS_H
