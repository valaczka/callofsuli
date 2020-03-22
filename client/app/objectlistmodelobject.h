/*
 * ---- Call of Suli ----
 *
 * objectlistmodelobject.h
 *
 * Created on: 2021. 12. 22.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ObjectListModelObject
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

#ifndef OBJECTLISTMODELOBJECT_H
#define OBJECTLISTMODELOBJECT_H

#include <QObject>
#include <QVariant>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMetaObject>
#include <QMetaProperty>
#include <QtJsonSerializer>

class ObjectListModelObject : public QObject
{
	Q_OBJECT

	Q_PROPERTY(bool selected READ selected WRITE setSelected NOTIFY selectedChanged STORED false)

public:
	Q_INVOKABLE explicit ObjectListModelObject(QObject *parent = nullptr);
	virtual ~ObjectListModelObject();

	bool selected() const;
	void setSelected(bool newSelected);


	template <typename TObject>
	static TObject* fromJsonObject(const QJsonObject &jsonObject, QObject *parent = nullptr)
	{
		QtJsonSerializer::JsonSerializer serializer;

		try {
			TObject *object = serializer.deserialize<TObject*>(jsonObject, parent);
			return object;
		} catch (QtJsonSerializer::Exception &e) {
			qWarning() << e.what();
			qDebug() << "FROM" << jsonObject;
		}

		return nullptr;
	}


	template <typename TObject>
	static TObject* fromJsonObject(const QByteArray &json, QObject *parent = nullptr)
	{
		return fromJsonObject<TObject>(QJsonDocument::fromJson(json).object(), parent);
	}

	template <typename TObject>
	static TObject* fromJsonObject(const QString &json, QObject *parent = nullptr)
	{
		return fromJsonObject<TObject>(json.toUtf8(), parent);
	}

	template <typename TObject>
	void updateProperties(TObject *other)
	{
		const QMetaObject *metaobject = this->metaObject();

		int count = metaobject->propertyCount();
		for (int i=0; i<count; ++i) {
			QMetaProperty metaproperty = metaobject->property(i);
			if (metaproperty.isWritable())
				metaproperty.write(this, metaproperty.read(other));
		}
	}



signals:
	void selectedChanged();

private:
	bool m_selected;
};


#endif // OBJECTLISTMODELOBJECT_H
