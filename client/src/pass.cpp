/*
 * ---- Call of Suli ----
 *
 * pass.cpp
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

#include "pass.h"
#include "clientcache.h"
#include "application.h"



/**
 * @brief Pass::Pass
 * @param parent
 */

Pass::Pass(QObject *parent)
	: SelectableObject(parent)
	, m_itemList(new PassItemList(this))
	, m_gradeList(new GradeList(this))
{

}


/**
 * @brief Pass::~Pass
 */

Pass::~Pass()
{

}



/**
 * @brief Pass::loadFromJson
 * @param object
 * @param allField
 */

void Pass::loadFromJson(const QJsonObject &object, const bool &allField)
{
	LOG_CINFO("client") << "LOAD" << object;

	if (object.contains(QStringLiteral("id")) || allField)
		setPassid(object.value(QStringLiteral("id")).toInt());

	if (object.contains(QStringLiteral("title")) || allField)
		setTitle(object.value(QStringLiteral("title")).toString());

	if (object.contains(QStringLiteral("starttime")) || allField)
		setStartTime(QDateTime::fromSecsSinceEpoch(object.value(QStringLiteral("starttime")).toInteger()));

	if (object.contains(QStringLiteral("endtime")) || allField)
		setEndTime(QDateTime::fromSecsSinceEpoch(object.value(QStringLiteral("endtime")).toInteger()));


	if (object.contains(QStringLiteral("childless")) || allField)
		setChildless(object.value(QStringLiteral("childless")).toVariant().toBool());

	/*if (object.contains(QStringLiteral("defaultGrade")) || allField)
		setDefaultGrade(qobject_cast<Grade*>(Application::instance()->client()->findCacheObject(QStringLiteral("gradeList"),
																								object.value(QStringLiteral("defaultGrade")).toInt())));
*/

	if (object.contains(QStringLiteral("items")) || allField) {
		OlmLoader::loadFromJsonArray<PassItem>(m_itemList.get(), object.value(QStringLiteral("items")).toArray(), "passitemid", "itemid", false);
	}

	if (object.contains(QStringLiteral("grading")) || allField)
		setGrading(object.value(QStringLiteral("grading")).toObject());

	if (object.contains(QStringLiteral("pts")) || allField)
		setPts(object.value(QStringLiteral("pts")).toDouble());

	if (object.contains(QStringLiteral("maxPts")) || allField)
		setMaxPts(object.value(QStringLiteral("maxPts")).toDouble());
}





/**
 * @brief Pass::reload
 */

void Pass::reload()
{
	LOG_CDEBUG("client") << "RELOAD" << m_passid;

	if (m_passid <= 0)
		return;

	Application::instance()->client()->httpConnection()->send(HttpConnection::ApiUser, QStringLiteral("pass/%1").arg(m_passid))
			->done(this, [this](const QJsonObject &data) {
		loadFromJson(data, false);
		emit itemsLoaded();
	});
}




/**
 * @brief PassItem::PassItem
 * @param parent
 */

PassItem::PassItem(QObject *parent)
	: SelectableObject(parent)
{

}


/**
 * @brief PassItem::~PassItem
 */

PassItem::~PassItem()
{

}


/**
 * @brief PassItem::loadFromJson
 * @param object
 * @param allField
 */

void PassItem::loadFromJson(const QJsonObject &object, const bool &allField)
{
	LOG_CINFO("client") << "LOAD" << object;

	if (object.contains(QStringLiteral("id")) || allField)
		setItemid(object.value(QStringLiteral("id")).toInt());

	if (object.contains(QStringLiteral("description")) || allField)
		setDescription(object.value(QStringLiteral("description")).toString());

	if (object.contains(QStringLiteral("extra")) || allField)
		setExtra(object.value(QStringLiteral("extra")).toVariant().toBool());

	if (object.contains(QStringLiteral("categoryid")) || allField)
		setCategoryId(object.value(QStringLiteral("categoryid")).toInt());

	if (object.contains(QStringLiteral("category")) || allField)
		setCategory(object.value(QStringLiteral("category")).toString());

	if (object.contains(QStringLiteral("pts")) || allField)
		setPts(object.value(QStringLiteral("pts")).toDouble());

	if (object.contains(QStringLiteral("maxPts")) || allField)
		setMaxPts(object.value(QStringLiteral("maxPts")).toDouble());

	if (object.contains(QStringLiteral("result")) || allField)
		setResult(object.value(QStringLiteral("result")).toDouble());
}



/**
 * @brief PassItem::categoryId
 * @return
 */

int PassItem::categoryId() const
{
	return m_categoryId;
}

void PassItem::setCategoryId(int newCategoryId)
{
	if (m_categoryId == newCategoryId)
		return;
	m_categoryId = newCategoryId;
	emit categoryIdChanged();
}

QString PassItem::category() const
{
	return m_category;
}

void PassItem::setCategory(const QString &newCategory)
{
	if (m_category == newCategory)
		return;
	m_category = newCategory;
	emit categoryChanged();
}

QString PassItem::description() const
{
	return m_description;
}

void PassItem::setDescription(const QString &newDescription)
{
	if (m_description == newDescription)
		return;
	m_description = newDescription;
	emit descriptionChanged();
}

bool PassItem::extra() const
{
	return m_extra;
}

void PassItem::setExtra(bool newExtra)
{
	if (m_extra == newExtra)
		return;
	m_extra = newExtra;
	emit extraChanged();
}

qreal PassItem::pts() const
{
	return m_pts;
}

void PassItem::setPts(qreal newPts)
{
	if (qFuzzyCompare(m_pts, newPts))
		return;
	m_pts = newPts;
	emit ptsChanged();
}

qreal PassItem::maxPts() const
{
	return m_maxPts;
}

void PassItem::setMaxPts(qreal newMaxPts)
{
	if (qFuzzyCompare(m_maxPts, newMaxPts))
		return;
	m_maxPts = newMaxPts;
	emit maxPtsChanged();
}

qreal PassItem::result() const
{
	return m_result;
}

void PassItem::setResult(qreal newResult)
{
	if (qFuzzyCompare(m_result, newResult))
		return;
	m_result = newResult;
	emit resultChanged();
}


int PassItem::itemid() const
{
	return m_itemid;
}

void PassItem::setItemid(int newItemid)
{
	if (m_itemid == newItemid)
		return;
	m_itemid = newItemid;
	emit itemidChanged();
}




/**
 * @brief Pass::passid
 * @return
 */

int Pass::passid() const
{
	return m_passid;
}

void Pass::setPassid(int newPassid)
{
	if (m_passid == newPassid)
		return;
	m_passid = newPassid;
	emit passidChanged();
}

QDateTime Pass::startTime() const
{
	return m_startTime;
}

void Pass::setStartTime(const QDateTime &newStartTime)
{
	if (m_startTime == newStartTime)
		return;
	m_startTime = newStartTime;
	emit startTimeChanged();
	emit isActiveChanged();
}

QDateTime Pass::endTime() const
{
	return m_endTime;
}

void Pass::setEndTime(const QDateTime &newEndTime)
{
	if (m_endTime == newEndTime)
		return;
	m_endTime = newEndTime;
	emit endTimeChanged();
	emit isActiveChanged();
}


/**
 * @brief Pass::isActive
 * @return
 */

bool Pass::isActive() const
{
	return m_startTime.isValid() && m_startTime <= QDateTime::currentDateTime() &&
			(!m_endTime.isValid() || m_endTime > QDateTime::currentDateTime());
}

QString Pass::title() const
{
	return m_title;
}

void Pass::setTitle(const QString &newTitle)
{
	if (m_title == newTitle)
		return;
	m_title = newTitle;
	emit titleChanged();
}

PassItemList *Pass::itemList() const
{
	return m_itemList.get();
}

bool Pass::childless() const
{
	return m_childless;
}

void Pass::setChildless(bool newChildless)
{
	if (m_childless == newChildless)
		return;
	m_childless = newChildless;
	emit childlessChanged();
}

QJsonObject Pass::grading() const
{
	return m_grading;
}

void Pass::setGrading(const QJsonObject &newGrading)
{
	if (m_grading == newGrading)
		return;
	m_grading = newGrading;
	emit gradingChanged();
}

GradeList *Pass::gradeList() const
{
	return m_gradeList.get();
}

qreal Pass::pts() const
{
	return m_pts;
}

void Pass::setPts(qreal newPts)
{
	if (qFuzzyCompare(m_pts, newPts))
		return;
	m_pts = newPts;
	emit ptsChanged();
}

qreal Pass::maxPts() const
{
	return m_maxPts;
}

void Pass::setMaxPts(qreal newMaxPts)
{
	if (qFuzzyCompare(m_maxPts, newMaxPts))
		return;
	m_maxPts = newMaxPts;
	emit maxPtsChanged();
}

qreal Pass::result() const
{
	return m_maxPts > 0 ? (m_pts/m_maxPts) : -1;
}

