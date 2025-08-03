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
#include "utils_.h"



/**
 * @brief Pass::Pass
 * @param parent
 */

Pass::Pass(QObject *parent)
	: SelectableObject(parent)
	, m_itemList(new PassItemList(this))
	, m_categoryList(new QSListModel(this))
{
	m_categoryList->setRoleNames({QStringLiteral("id"), QStringLiteral("description")});
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
	if (object.contains(QStringLiteral("id")) || allField)
		setPassid(object.value(QStringLiteral("id")).toInt());

	if (object.contains(QStringLiteral("groupid")) || allField)
		setGroupid(object.value(QStringLiteral("groupid")).toInt());

	if (object.contains(QStringLiteral("title")) || allField)
		setTitle(object.value(QStringLiteral("title")).toString());

	if (object.contains(QStringLiteral("starttime")) || allField) {
		if (const int s = object.value(QStringLiteral("starttime")).toInteger(); s > 0)
			setStartTime(QDateTime::fromSecsSinceEpoch(s));
		else
			setStartTime({});
	}

	if (object.contains(QStringLiteral("endtime")) || allField) {
		if (const int s = object.value(QStringLiteral("endtime")).toInteger(); s > 0)
			setEndTime(QDateTime::fromSecsSinceEpoch(s));
		else
			setEndTime({});
	}

	if (object.contains(QStringLiteral("childless")) || allField)
		setChildless(object.value(QStringLiteral("childless")).toVariant().toBool());

	if (object.contains(QStringLiteral("items")) || allField) {
		OlmLoader::loadFromJsonArray<PassItem>(m_itemList.get(), object.value(QStringLiteral("items")).toArray(), "passitemid", "itemid", false);
		updateCategoryList();
	}

	if (object.contains(QStringLiteral("grading")) || allField)
		setGrading(object.value(QStringLiteral("grading")).toObject());

	if (object.contains(QStringLiteral("pts")) || allField)
		setPts(object.value(QStringLiteral("pts")).toDouble());

	if (object.contains(QStringLiteral("maxPts")) || allField)
		setMaxPts(object.value(QStringLiteral("maxPts")).toDouble());


	updateGrading();
}





/**
 * @brief Pass::reload
 */

void Pass::reload(const HttpConnection::API &api)
{
	if (m_passid <= 0)
		return;

	Application::instance()->client()->httpConnection()->send(api, QStringLiteral("pass/%1").arg(m_passid))
			->done(this, [this](const QJsonObject &data) {
		loadFromJson(data, false);
		emit itemsLoaded();
	});
}


/**
 * @brief Pass::getGradingFromData
 * @param grading
 * @return
 */

QVariantList Pass::getGradingFromData(const QJsonObject &grading)
{
	return mapToList(getGradingMap(grading));
}


/**
 * @brief Pass::getMapFromUi
 * @param data
 * @return
 */

QJsonObject Pass::getMapFromUi(const QVariantList &data)
{
	return mapToObject(getGradingMap(data));
}



/**
 * @brief Pass::getGradingFromUi
 * @param data
 * @return
 */

QVariantList Pass::getGradingFromUi(const QVariantList &data)
{
	return mapToList(getGradingMap(data));
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


	if (int id = object.value(QStringLiteral("includepass")).toInt(-1); id > 0) {
		setIncludePass(id);
		setMaxPts(-1);
		setDescription(object.value(QStringLiteral("includeTitle")).toString());
	} else {
		setIncludePass(-1);
	}
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
	m_pts = Pass::round(newPts);
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
	m_maxPts = Pass::round(newMaxPts);
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
	m_result = Pass::round(newResult);
	emit resultChanged();
}

int PassItem::includePass() const
{
	return m_includePass;
}

void PassItem::setIncludePass(int newIncludePass)
{
	if (m_includePass == newIncludePass)
		return;
	m_includePass = newIncludePass;
	emit includePassChanged();
}

PassItem::LinkType PassItem::linkType() const
{
	return m_linkType;
}

void PassItem::setLinkType(const LinkType &newLinkType)
{
	if (m_linkType == newLinkType)
		return;
	m_linkType = newLinkType;
	emit linkTypeChanged();
}

int PassItem::linkId() const
{
	return m_linkId;
}

void PassItem::setLinkId(int newLinkId)
{
	if (m_linkId == newLinkId)
		return;
	m_linkId = newLinkId;
	emit linkIdChanged();
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
	return !m_startTime.isNull() && m_startTime <= QDateTime::currentDateTime() &&
			(m_endTime.isNull() || m_endTime > QDateTime::currentDateTime());
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

qreal Pass::pts() const
{
	return m_pts;
}

void Pass::setPts(qreal newPts)
{
	if (qFuzzyCompare(m_pts, newPts))
		return;
	m_pts = Pass::round(newPts);
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
	m_maxPts = Pass::round(newMaxPts);
	emit maxPtsChanged();
}

qreal Pass::result() const
{
	return m_maxPts > 0 ? Pass::round(m_pts/m_maxPts) : -1;
}

QSListModel *Pass::categoryList() const
{
	return m_categoryList.get();
}



/**
 * @brief Pass::updateCategoryList
 */

void Pass::updateCategoryList()
{
	struct Category {
		int id = -1;
		QString title;

		bool operator < (const Category &other) const {
			return other.id <= 0 || (id > 0 && id < other.id);
		}

		QVariantMap toMap() const {
			return QVariantMap{
				{ QStringLiteral("id"), id },
				{ QStringLiteral("description"), title }
			};
		}
	};

	std::vector<Category> list;

	for (PassItem *i : *m_itemList) {
		if (list.cend() == std::find_if(list.cbegin(), list.cend(), [i](const Category &c) { return c.id == i->categoryId(); }))
			list.emplace_back(i->categoryId(), i->category());
	}

	std::sort(list.begin(), list.end(), std::less<Category>());

	QVariantList l;

	for (const Category &c : list)
		l.append(c.toMap());

	Utils::patchSListModel(m_categoryList.get(), l, QStringLiteral("id"));
}


/**
 * @brief Pass::updateGrading
 */

void Pass::updateGrading()
{
	QList<Grade*> list = getGrades(result(), m_grading,
								   dynamic_cast<GradeList*>(Application::instance()->client()->cache(QStringLiteral("gradeList"))));

	QVariantList l;
	for (Grade *g : list)
		l.append(QVariant::fromValue(g));

	setGradeList(l);
}



/**
 * @brief Pass::Pass::round
 * @param num
 * @return
 */

qreal Pass::round(const qreal &num)
{
	float value = (int)(num * 100 + .5);
	return (float) value / 100;
}


/**
 * @brief Pass::getGrades
 * @param result
 * @param grading
 * @param list
 * @return
 */

QList<Grade *> Pass::getGrades(const qreal &result, const QJsonObject &grading, GradeList *list)
{
	if (grading.isEmpty() || !list || list->empty()) {
		return {};
	}

	const QMap<int, QList<int> > values = getGradingMap(grading);

	auto it = values.upperBound(result*100);

	if (it == values.constBegin())
		return {};

	it = std::prev(it);

	QList<Grade*> ret;

	for (const int id : it.value()) {
		Grade *g = OlmLoader::find<Grade>(list, "gradeid", id);

		if (g) {
			ret.append(g);
		} else {
			LOG_CERROR("client") << "Invalid grade id" << id << "in" << grading;
		}
	}

	return ret;
}



/**
 * @brief Pass::getGradingMap
 * @param grading
 * @return
 */

QMap<int, QList<int> > Pass::getGradingMap(const QJsonObject &grading)
{
	QMap<int, QList<int>> values;

	for (auto it = grading.constBegin(); it != grading.constEnd(); ++it) {
		int key = it.key().toInt();

		QList<int> list;

		QJsonArray array = it.value().toArray();

		for (const QJsonValue &v : array) {
			list.append(v.toInt());
		}

		values[key] = list;
	}

	return values;
}


/**
 * @brief Pass::getGradingMap
 * @param grading
 * @return
 */

QMap<int, QList<int> > Pass::getGradingMap(const QVariantList &grading)
{
	QMap<int, QList<int>> values;

	for (const QVariant &v : grading) {
		const QVariantMap &m = v.toMap();

		int key = m.value(QStringLiteral("key"), -1.).toInt();

		if (key < 0)
			continue;

		const QVariantList &list = m.value(QStringLiteral("list")).toList();

		QList<int> l;

		for (const QVariant &v : list)
			l.append(v.toInt());

		values[key] = l;
	}

	return values;
}


/**
 * @brief Pass::mapToList
 * @param map
 * @return
 */

QVariantList Pass::mapToList(const QMap<int, QList<int> > &map)
{
	QVariantList list;

	for (const auto &[key, l] : map.asKeyValueRange()) {
		QVariantList ll;

		for (const int i : l)
			ll << i;

		list.append(QVariantMap {
						{ QStringLiteral("key"), key },
						{ QStringLiteral("list"), ll }
					});
	}

	return list;
}


/**
 * @brief Pass::mapToObject
 * @param map
 * @return
 */

QJsonObject Pass::mapToObject(const QMap<int, QList<int> > &map)
{
	QJsonObject obj;

	for (const auto &[key, l] : map.asKeyValueRange()) {
		QJsonArray ll;

		for (const int i : l)
			ll << i;

		obj.insert(QString::number(key), ll);
	}

	return obj;
}




int Pass::groupid() const
{
	return m_groupid;
}

void Pass::setGroupid(int newGroupid)
{
	if (m_groupid == newGroupid)
		return;
	m_groupid = newGroupid;
	emit groupidChanged();
}


/**
 * @brief Pass::gradeList
 * @return
 */

QVariantList Pass::gradeList() const
{
	return m_gradeList;
}

void Pass::setGradeList(const QVariantList &newGradeList)
{
	if (m_gradeList == newGradeList)
		return;
	m_gradeList = newGradeList;
	emit gradeListChanged();
}
