/*
 * ---- Call of Suli ----
 *
 * clientcache.h
 *
 * Created on: 2023. 01. 20.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ClientCache
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

#ifndef CLIENTCACHE_H
#define CLIENTCACHE_H

#include "Logger.h"
#include "QOlm/Details/QOlmBase.hpp"
#include "QOlm/QOlm.hpp"
#include "qdebug.h"
#include "qjsonarray.h"
#include "qjsonobject.h"
#include "websocket.h"

/**
 * @brief The CacheItemBase class
 */

class CacheItemBase {
public:
	CacheItemBase() {}
	CacheItemBase(const QString &key, const QString &id, qolm::QOlmBase *olm = nullptr)
		: m_key(key), m_id(id), m_olm(olm) {}
	virtual ~CacheItemBase() {  }

	qolm::QOlmBase *olmBase() const { return m_olm; }

	virtual bool load(const QJsonArray &) const { return false; };

	friend inline bool operator==(const CacheItemBase &c, const CacheItemBase &other) { return c.m_key==other.m_key && c.m_id==other.m_id; }
	friend inline bool operator==(const CacheItemBase &c, CacheItemBase *other) { return other && c.m_key==other->m_key && c.m_id==other->m_id; }
	friend inline bool operator==(CacheItemBase *c, const CacheItemBase &other) { return c && c->m_key==other.m_key && c->m_id==other.m_id; }

	friend QDebug operator<<(QDebug out, const CacheItemBase &id)
	{
		QDebugStateSaver saver(out);
		out.nospace() << '(' << qPrintable(id.m_key) << ',' << qPrintable(id.m_id) << ':' << id.m_olm << ')';
		return out;
	}

	const WebSocket::API &api() const { return m_api; }
	const QString &path() const { return m_path; }
	QString fullPath() const;

protected:
	WebSocket::API m_api = WebSocket::ApiInvalid;
	QString m_path;

private:
	QString m_key;
	QString m_id;
	qolm::QOlmBase *m_olm = nullptr;
};


template <typename T, typename = void>
class CacheItem;

template <typename T>
using QOlmBase_SFINAE = typename std::enable_if<std::is_base_of<qolm::QOlmBase, T>::value>::type;

template <typename T>
class CacheItem<T, QOlmBase_SFINAE<T>> : public CacheItemBase
{
public:

	typedef std::function<bool(T *olm, const QJsonArray &)> LoadFunc;

	explicit CacheItem(const QString &key, const QString &id, T* olm, LoadFunc func = nullptr,
					   const WebSocket::API &api = WebSocket::ApiInvalid, const QString &path = QLatin1String(""))
		: CacheItemBase(key, id, olm)
		, m_olm(olm)
		, m_func(func)
	{
		m_api = api;
		m_path = path;
	}

	explicit CacheItem(const QString &key, T* olm, LoadFunc func = nullptr,
					   const WebSocket::API &api = WebSocket::ApiInvalid, const QString &path = QLatin1String(""))
		: CacheItem(key, QString(), olm, func, api, path) {}

	T *olm() const { return m_olm; }


	bool load(const QJsonArray &list) const override {
		if (m_func)
			return std::invoke(m_func, m_olm, list);
		else
			return false;
	}


private:
	T *const m_olm;
	LoadFunc m_func;

};




template <typename T, typename = void>
class CacheItem2;

template <typename T>
class CacheItem2<T, QOlmBase_SFINAE<T>> : public CacheItemBase
{
public:

	explicit CacheItem2(const QString &id, T* olm, bool (*handler)(T *, const QJsonArray &))
		: CacheItemBase("", id, olm)
		, m_olm(olm)
		, m_func(std::bind(handler, std::placeholders::_1, std::placeholders::_2))
	{

	}


	bool load2(T* olm, const QJsonArray &list) const {
		if (m_func)
			return std::invoke(m_func, olm, list);
		else
			return false;
	}

private:
	T *const m_olm;
	std::function<bool(T *olm, const QJsonArray &)> m_func;

};


/**
 * @brief The ClientCache class
 */

class ClientCache
{
public:
	explicit ClientCache();
	virtual ~ClientCache();

	bool contains(const CacheItemBase &item) const;
	bool contains(const char *key, const char *id) const { return contains(CacheItemBase(key, id)); }
	bool contains(const char *key, const int &id) const { return contains(CacheItemBase(key, QString::number(id))); }
	bool contains(const char *key) const { return contains(CacheItemBase(key, QString())); }

	bool add(CacheItemBase *item);

	bool remove(const CacheItemBase &item);
	bool remove(const char *key, const char *id) { return remove(CacheItemBase(key, id)); }
	bool remove(const char *key, const int &id) { return remove(CacheItemBase(key, QString::number(id))); }
	bool remove(const char *key) { return remove(CacheItemBase(key, QString())); }
	bool removeAll(qolm::QOlmBase *olm);
	void removeAll();

	bool set(const CacheItemBase &id, const QJsonArray &list);
	bool set(const char *key, const char *id, const QJsonArray &list) { return set(CacheItemBase(key, id), list); }
	bool set(const char *key, const int &id, const QJsonArray &list) { return set(CacheItemBase(key, QString::number(id)), list); }
	bool set(const char *key, const QJsonArray &list) { return set(CacheItemBase(key, QString()), list); }

	bool reload(WebSocket *websocket, const CacheItemBase &id);
	bool reload(WebSocket *websocket, const char *key, const char *id) { return reload(websocket, CacheItemBase(key, id)); }
	bool reload(WebSocket *websocket, const char *key, const int &id) { return reload(websocket, CacheItemBase(key, QString::number(id))); }
	bool reload(WebSocket *websocket, const char *key) { return reload(websocket, CacheItemBase(key, QString())); }

	qolm::QOlmBase* get(const CacheItemBase &id) const;
	qolm::QOlmBase* get(const char *key, const char *id) const { return get(CacheItemBase(key, id)); }
	qolm::QOlmBase* get(const char *key, const int &id) const { return get(CacheItemBase(key, QString::number(id))); }
	qolm::QOlmBase* get(const char *key) const { return get(CacheItemBase(key, QString())); }

	template <typename T>
	T* getAs(const CacheItemBase &id) const;

	template <typename T>
	T* getAs(const char *key, const char *id) const { return getAs<T>(CacheItemBase(key, id)); }

	template <typename T>
	T* getAs(const char *key, const int &id) const { return getAs<T>(CacheItemBase(key, QString::number(id))); }

	template <typename T>
	T* getAs(const char *key) const { return getAs<T>(CacheItemBase(key, QString())); }


	template <typename T>
	static bool loadFromJsonArray(qolm::QOlm<T> *list, const QJsonArray &jsonArray, const char *jsonField, const char *property);

	template <typename T>
	static bool loadFromJsonArray2(qolm::QOlm<T> *list, const QJsonArray &jsonArray);

	template <typename T>
	static T* find(qolm::QOlm<T> *list, const char *property, const QVariant &value);

private:
	QVector<CacheItemBase*> m_list;

};






/// ---------------------------------------------------------------

template <typename T>
T* ClientCache::getAs(const CacheItemBase &id) const
{
	for (auto i=m_list.begin(); i != m_list.end(); ++i) {
		if (*i == id) {
			CacheItem<T> *t = dynamic_cast<CacheItem<T>*>(*i);
			if (t)
				return t->olm();
			else
				return nullptr;
		}
	}
	return nullptr;
}




template<typename T>
bool ClientCache::loadFromJsonArray(qolm::QOlm<T> *list, const QJsonArray &jsonArray, const char *jsonField, const char *property)
{
	if (!list)
		return false;

	QVector<T*> tmp;

	for (T *u : *list)
		tmp.append(u);

	foreach (const QJsonValue &v, jsonArray) {
		const QJsonObject &obj = v.toObject();
		T *record = find<T>(list, property, obj.value(jsonField).toVariant());
		if (record) {
			tmp.removeAll(record);
			record->loadFromJson(obj, true);				// All fields overwrite
		} else {
			record = new T();
			record->loadFromJson(obj, true);				// All fields overwrite
			list->append(record);
		}
	}

	foreach (T *u, tmp)
		list->remove(u);

	return true;
}



template<typename T>
bool ClientCache::loadFromJsonArray2(qolm::QOlm<T> *list, const QJsonArray &jsonArray)
{
	LOG_CDEBUG("client") << "Test point X";
	if (!list)
		return false;

	LOG_CDEBUG("client") << "Test point X1";

	list->clear();

	LOG_CDEBUG("client") << "Test point X2";

	foreach (const QJsonValue &v, jsonArray) {
		LOG_CDEBUG("client") << "Test point X3" << v;
		T* record = new T();
		LOG_CDEBUG("client") << "Test point X4";
		record->loadFromJson(v.toObject(), true);				// All fields overwrite
		LOG_CDEBUG("client") << "Test point X5";
		list->append(record);
		LOG_CDEBUG("client") << "Test point X6";
	}

	LOG_CDEBUG("client") << "Test point X7";
}





template<typename T>
T* ClientCache::find(qolm::QOlm<T> *list, const char *property, const QVariant &value)
{
	if (!list)
		return nullptr;

	for (T *u : *list) {
		if (u->property(property) == value)
			return u;
	}

	return nullptr;
}


#endif // CLIENTCACHE_H
