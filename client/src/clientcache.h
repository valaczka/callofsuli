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
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#include "QOlm/Details/QOlmBase.hpp"
#include "QOlm/QOlm.hpp"
#pragma GCC diagnostic warning "-Wunused-parameter"
#pragma GCC diagnostic warning "-Wunused-variable"
#include "qdebug.h"
#include "qjsonarray.h"
#include "qjsonobject.h"
#include "websocket.h"


typedef std::function<void(qolm::QOlmBase *olm, const QJsonArray &, const char *, const char *, const bool &)> OlmLoaderFunc;
typedef std::function<QObject*(qolm::QOlmBase *olm, const char *, const QVariant &)> OlmFinderFunc;

/**
 * @brief The ClientCache class
 */

class ClientCache : public QObject
{
	Q_OBJECT

public:
	explicit ClientCache(QObject *parent = nullptr);
	virtual ~ClientCache();

	/**
	 * @brief The CacheItem class
	 */

	struct CacheItem {
		qolm::QOlmBase *list;
		OlmLoaderFunc func;
		OlmFinderFunc finder;
		WebSocket::API api = WebSocket::ApiInvalid;
		const char *path = nullptr;
		const char *jsonField = nullptr;
		const char *property = nullptr;
		bool allFieldOverride = false;
	};


	template <typename T>
	void add(const QString &key, qolm::QOlm<T> *list,
			 void (*handler)(qolm::QOlmBase *, const QJsonArray &, const char *, const char *, const bool &),
			 T* (*finder)(qolm::QOlmBase *, const char *, const QVariant &),
			 const char *jsonField = nullptr, const char *property = nullptr, const bool &allFieldOverride = true,
			 const WebSocket::API &api = WebSocket::ApiInvalid, const char *path = nullptr);

	bool contains(const QString &key) const;
	void remove(const QString &key);
	void remove(qolm::QOlmBase *olm);
	void removeAll();

	qolm::QOlmBase* get(const QString &key) const;

	bool set(const QString &key, const QJsonArray &list);

	bool reload(WebSocket *websocket, const QString &key);
	bool reload(WebSocket *websocket, const QString &key, QJSValue func);

	QObject *find(const QString &key, const QVariant &value);

	void clear();



	template <typename T>
	void addHandler(const QString &key,
					void (*handler)(qolm::QOlmBase *, const QJsonArray &, const char *, const char *, const bool &),
					T* (*finder)(qolm::QOlmBase *, const char *, const QVariant &),
					const char *jsonField = nullptr, const char *property = nullptr, const bool &allFieldOverride = true);

	bool callReloadHandler(const QString &key, qolm::QOlmBase *list, const QJsonArray &array) const;
	QObject *callFinderHandler(const QString &key, qolm::QOlmBase *list, const QVariant &value) const;


private:
	QMap<QString, CacheItem> m_list;
	QMap<QString, CacheItem> m_handlers;

};




/**
 * @brief The OlmLoader class
 */

class OlmLoader
{
public:
	explicit OlmLoader() {}


	template <typename T>
	static void loadFromJsonArray(qolm::QOlmBase *list, const QJsonArray &jsonArray,
								  const char *jsonField, const char *property, const bool &allFieldOverride);

	template <typename T>
	static void map(QMap<QString, OlmLoaderFunc> *map, const QString &key,
					void (*handler)(qolm::QOlmBase *, const QJsonArray &, const char *, const char *, const bool &));


	template <typename T>
	void map(const QString &key,
			 void (*handler)(qolm::QOlmBase *, const QJsonArray &, const char *, const char *, const bool &)) {
		map<T>(&m_list, key, handler);
	}


	template <typename T>
	static T* find(qolm::QOlmBase *list, const char *property, const QVariant &value);


	bool contains(const QString &key) const {
		return m_list.contains(key);
	}

	static bool call(const QMap<QString, OlmLoaderFunc> &map, const QString &key,
					 qolm::QOlmBase *list, const QJsonArray &array,
					 const char *jsonField = nullptr, const char *property = nullptr, const bool &allFieldOverride = true);

	bool call(const QString &key,
			  qolm::QOlmBase *list, const QJsonArray &array,
			  const char *jsonField = nullptr, const char *property = nullptr, const bool &allFieldOverride = true ) const {
		return call(m_list, key, list, array, jsonField, property, allFieldOverride);
	}

	static void call(const OlmLoaderFunc &func, qolm::QOlmBase *list, const QJsonArray &array,
					 const char *jsonField, const char *property , const bool &allFieldOverride);


private:
	QMap<QString, OlmLoaderFunc> m_list;

};





/// ---------------------------------------------------------------




template <typename T>
void ClientCache::add(const QString &key, qolm::QOlm<T> *list,
					  void (*handler)(qolm::QOlmBase *, const QJsonArray &, const char *, const char *, const bool &),
					  T* (*finder)(qolm::QOlmBase *, const char *, const QVariant &),
					  const char *jsonField, const char *property,
					  const bool &allFieldOverride,
					  const WebSocket::API &api, const char *path)
{
	Q_ASSERT(list);

	if (m_list.contains(key)) {
		LOG_CWARNING("client") << "Key already exists:" << key;
		return;
	}

	CacheItem it;
	it.list = list;
	it.func = handler;
	it.finder = finder;
	it.api = api;
	it.path = path;
	it.jsonField = jsonField;
	it.property = property;
	it.allFieldOverride = allFieldOverride;

	m_list.insert(key, it);

	QObject::connect(list, &QObject::destroyed, this, [this, key](){remove(key);});
}





/**
 * @brief ClientCache::addHandler
 * @param key
 * @param jsonField
 * @param property
 */

template<typename T>
void ClientCache::addHandler(const QString &key,
							 void (*handler)(qolm::QOlmBase *, const QJsonArray &, const char *, const char *, const bool &),
							 T* (*finder)(qolm::QOlmBase *, const char *, const QVariant &),
							 const char *jsonField, const char *property, const bool &allFieldOverride)
{
	if (m_handlers.contains(key)) {
		LOG_CWARNING("client") << "Handler key already exists:" << key;
		return;
	}

	CacheItem it;
	it.list = nullptr;
	it.func = handler;
	it.finder = finder;
	it.jsonField = jsonField;
	it.property = property;
	it.allFieldOverride = allFieldOverride;

	m_list.insert(key, it);
}



/**
 * @brief OlmLoader::loadFromJsonArray
 * @param list
 * @param jsonArray
 * @param jsonField
 * @param property
 */



template<typename T>
void OlmLoader::loadFromJsonArray(qolm::QOlmBase *list, const QJsonArray &jsonArray, const char *jsonField, const char *property,
								  const bool &allFieldOverride)
{
	Q_ASSERT(list);

	qolm::QOlm<T> *t = dynamic_cast<qolm::QOlm<T>*>(list);

	if (jsonField && property) {
		QVector<T*> tmp;

		for (T *u : *t)
			tmp.append(u);

		foreach (const QJsonValue &v, jsonArray) {
			const QJsonObject &obj = v.toObject();
			T *record = find<T>(t, property, obj.value(jsonField).toVariant());
			if (record) {
				tmp.removeAll(record);
				record->loadFromJson(obj, allFieldOverride);
			} else {
				record = new T(list);
				record->loadFromJson(obj, allFieldOverride);
				t->append(record);
			}
		}

		foreach (T *u, tmp) {
			t->remove(u);
			u->deleteLater();
		}

	} else {

		if (!t) {
			LOG_CWARNING("client") << "OlmLoader type error" << list;
			return;
		}

		for (T *o : *t)
			o->deleteLater();

		list->clear();

		foreach (const QJsonValue &v, jsonArray) {
			T* record = new T(list);
			record->loadFromJson(v.toObject(), allFieldOverride);
			t->append(record);
		}
	}
}




/**
 * @brief OlmLoader::map
 * @param map
 * @param key
 */


template<typename T>
void OlmLoader::map(QMap<QString, OlmLoaderFunc> *map, const QString &key,
					void (*handler)(qolm::QOlmBase *, const QJsonArray &, const char *, const char *, const bool &))
{
	Q_ASSERT(map);

	if (map->contains(key)) {
		LOG_CWARNING("client") << "Key already exist:" << key;
	}

	map->insert(key, handler);
}


/**
 * @brief OlmLoader::find
 * @param list
 * @param property
 * @param value
 * @return
 */

template<typename T>
T *OlmLoader::find(qolm::QOlmBase *list, const char *property, const QVariant &value)
{
	Q_ASSERT(list);

	qolm::QOlm<T> *t = dynamic_cast<qolm::QOlm<T>*>(list);

	for (T *u : *t) {
		if (u->property(property) == value)
			return u;
	}

	return nullptr;
}







#endif // CLIENTCACHE_H
