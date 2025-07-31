/*
 * ---- Call of Suli ----
 *
 * pass.h
 *
 * Created on: 2025. 07. 31.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * Pass
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

#ifndef PASS_H
#define PASS_H

#include "grade.h"
#include "qdatetime.h"
#include "qjsonobject.h"
#include <QPointer>
#include <selectableobject.h>
#include <QObject>
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#include "QOlm/QOlm.hpp"
#pragma GCC diagnostic warning "-Wunused-parameter"
#pragma GCC diagnostic warning "-Wunused-variable"

class Pass;
using PassList = qolm::QOlm<Pass>;
Q_DECLARE_METATYPE(PassList*)


class PassItem;
using PassItemList = qolm::QOlm<PassItem>;
Q_DECLARE_METATYPE(PassItemList*)




/**
 * @brief The PassItem class
 */

class PassItem : public SelectableObject
{
	Q_OBJECT

	Q_PROPERTY(int itemid READ itemid WRITE setItemid NOTIFY itemidChanged FINAL)
	Q_PROPERTY(int categoryId READ categoryId WRITE setCategoryId NOTIFY categoryIdChanged FINAL)
	Q_PROPERTY(QString category READ category WRITE setCategory NOTIFY categoryChanged FINAL)
	Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged FINAL)
	Q_PROPERTY(bool extra READ extra WRITE setExtra NOTIFY extraChanged FINAL)
	Q_PROPERTY(qreal pts READ pts WRITE setPts NOTIFY ptsChanged FINAL)
	Q_PROPERTY(qreal maxPts READ maxPts WRITE setMaxPts NOTIFY maxPtsChanged FINAL)
	Q_PROPERTY(qreal result READ result WRITE setResult NOTIFY resultChanged FINAL)

public:
	explicit PassItem(QObject *parent = nullptr);
	virtual ~PassItem();

	Q_INVOKABLE void loadFromJson(const QJsonObject &object, const bool &allField = true);

	int itemid() const;
	void setItemid(int newItemid);

	int categoryId() const;
	void setCategoryId(int newCategoryId);

	QString category() const;
	void setCategory(const QString &newCategory);

	QString description() const;
	void setDescription(const QString &newDescription);

	bool extra() const;
	void setExtra(bool newExtra);

	qreal pts() const;
	void setPts(qreal newPts);

	qreal maxPts() const;
	void setMaxPts(qreal newMaxPts);

	qreal result() const;
	void setResult(qreal newResult);

signals:
	void itemidChanged();
	void categoryIdChanged();
	void categoryChanged();
	void descriptionChanged();
	void extraChanged();
	void ptsChanged();
	void maxPtsChanged();
	void resultChanged();

private:
	int m_itemid = -1;
	int m_categoryId = -1;
	QString m_category;
	QString m_description;
	bool m_extra = false;
	qreal m_pts = 0.;
	qreal m_maxPts = 0.;
	qreal m_result = 0.;
};




/**
 * @brief The Pass class
 */

class Pass : public SelectableObject
{
	Q_OBJECT

	Q_PROPERTY(int passid READ passid WRITE setPassid NOTIFY passidChanged FINAL)
	Q_PROPERTY(QDateTime startTime READ startTime WRITE setStartTime NOTIFY startTimeChanged FINAL)
	Q_PROPERTY(QDateTime endTime READ endTime WRITE setEndTime NOTIFY endTimeChanged FINAL)
	Q_PROPERTY(bool isActive READ isActive NOTIFY isActiveChanged FINAL)
	Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged FINAL)
	Q_PROPERTY(PassItemList *itemList READ itemList CONSTANT FINAL)
	Q_PROPERTY(bool childless READ childless WRITE setChildless NOTIFY childlessChanged FINAL)
	Q_PROPERTY(QJsonObject grading READ grading WRITE setGrading NOTIFY gradingChanged FINAL)
	Q_PROPERTY(GradeList *gradeList READ gradeList CONSTANT FINAL)
	Q_PROPERTY(qreal pts READ pts WRITE setPts NOTIFY ptsChanged FINAL)
	Q_PROPERTY(qreal maxPts READ maxPts WRITE setMaxPts NOTIFY maxPtsChanged FINAL)
	Q_PROPERTY(qreal result READ result NOTIFY resultChanged FINAL)

public:
	explicit Pass(QObject *parent = nullptr);
	virtual ~Pass();

	Q_INVOKABLE void loadFromJson(const QJsonObject &object, const bool &allField = true);

	Q_INVOKABLE void reload();

	int passid() const;
	void setPassid(int newPassid);

	QDateTime startTime() const;
	void setStartTime(const QDateTime &newStartTime);

	QDateTime endTime() const;
	void setEndTime(const QDateTime &newEndTime);

	bool isActive() const;

	QString title() const;
	void setTitle(const QString &newTitle);

	PassItemList *itemList() const;

	bool childless() const;
	void setChildless(bool newChildless);

	QJsonObject grading() const;
	void setGrading(const QJsonObject &newGrading);

	GradeList *gradeList() const;

	qreal pts() const;
	void setPts(qreal newPts);

	qreal maxPts() const;
	void setMaxPts(qreal newMaxPts);

	qreal result() const;

signals:
	void itemsLoaded();
	void passidChanged();
	void startTimeChanged();
	void endTimeChanged();
	void isActiveChanged();
	void titleChanged();
	void childlessChanged();
	void gradingChanged();
	void ptsChanged();
	void maxPtsChanged();
	void resultChanged();

private:
	int m_passid;
	QDateTime m_startTime;
	QDateTime m_endTime;
	QString m_title;
	std::unique_ptr<PassItemList> m_itemList;
	bool m_childless;
	QJsonObject m_grading;
	std::unique_ptr<GradeList> m_gradeList;
	qreal m_pts;
	qreal m_maxPts;
};

#endif // PASS_H
