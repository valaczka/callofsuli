/*
 * ---- Call of Suli ----
 *
 * tiledtransport.h
 *
 * Created on: 2024. 03. 07.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * TiledTransport
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

#ifndef TILEDTRANSPORT_H
#define TILEDTRANSPORT_H

#include "tiledscene.h"


/**
 * @brief The TiledTransport class
 */

class TiledTransport : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged FINAL)
	Q_PROPERTY(bool isOpen READ isOpen WRITE setIsOpen NOTIFY isOpenChanged FINAL)
	Q_PROPERTY(bool isActive READ isActive WRITE setIsActive NOTIFY isActiveChanged FINAL)
	Q_PROPERTY(TiledScene *sceneA READ sceneA WRITE setSceneA NOTIFY sceneAChanged FINAL)
	Q_PROPERTY(TiledObjectBase *objectA READ objectA WRITE setObjectA NOTIFY objectAChanged FINAL)
	Q_PROPERTY(TiledScene *sceneB READ sceneB WRITE setSceneB NOTIFY sceneBChanged FINAL)
	Q_PROPERTY(TiledObjectBase *objectB READ objectB WRITE setObjectB NOTIFY objectBChanged FINAL)
	Q_PROPERTY(TransportType type READ type WRITE setType NOTIFY typeChanged FINAL)

public:
	enum TransportType {
		TransportInvalid = 0,
		TransportGate
	};

	Q_ENUM(TransportType);

	TiledTransport(const TransportType &type, const QString &name,
				   TiledScene *sceneA, TiledObjectBase *objectA,
				   TiledScene *sceneB, TiledObjectBase *objectB);

	TiledTransport(const QString &name,
				   TiledScene *sceneA, TiledObjectBase *objectA,
				   TiledScene *sceneB, TiledObjectBase *objectB)
		: TiledTransport(TransportInvalid, name, sceneA, objectA, sceneB, objectB) {}

	TiledTransport(const TransportType &type, const QString &name, TiledScene *sceneA, TiledObjectBase *objectA)
		: TiledTransport(type, name, sceneA, objectA, nullptr, nullptr) {}

	TiledTransport(const QString &name, TiledScene *sceneA, TiledObjectBase *objectA)
		: TiledTransport(name, sceneA, objectA, nullptr, nullptr) {}

	TiledTransport(const QString &name)
		: TiledTransport(name, nullptr, nullptr) {}

	TiledTransport()
		: TiledTransport(QStringLiteral("")) {}


	static TransportType typeFromString(const QString &str);


	bool addObject(TiledScene *scene, TiledObjectBase *object);
	TiledScene *otherScene(TiledScene *scene) const;
	TiledScene *otherScene(TiledObjectBase *object) const;
	TiledObjectBase *otherObject(TiledScene *scene) const;
	TiledObjectBase *otherObject(TiledObjectBase *object) const;

	QString name() const;
	void setName(const QString &newName);

	TiledScene *sceneA() const;
	void setSceneA(TiledScene *newSceneA);

	TiledObjectBase *objectA() const;
	void setObjectA(TiledObjectBase *newObjectA);

	TiledScene *sceneB() const;
	void setSceneB(TiledScene *newSceneB);

	TiledObjectBase *objectB() const;
	void setObjectB(TiledObjectBase *newObjectB);

	bool isOpen() const;
	void setIsOpen(bool newIsOpen);

	bool isActive() const;
	void setIsActive(bool newIsActive);

	TransportType type() const;
	void setType(const TransportType &newType);

signals:
	void nameChanged();
	void sceneAChanged();
	void objectAChanged();
	void sceneBChanged();
	void objectBChanged();
	void isOpenChanged();
	void isActiveChanged();
	void typeChanged();

protected:
	TransportType m_type = TransportInvalid;
	QString m_name;
	bool m_isOpen = true;
	bool m_isActive = false;

	QPointer<TiledScene> m_sceneA;
	QPointer<TiledObjectBase> m_objectA;

	QPointer<TiledScene> m_sceneB;
	QPointer<TiledObjectBase> m_objectB;
};





/**
 * @brief The TiledTransportList class
 */

class TiledTransportList : public std::vector<std::unique_ptr<TiledTransport>>
{
public:
	TiledTransportList()
		: std::vector<std::unique_ptr<TiledTransport>>()
	{}

	bool add(const TiledTransport::TransportType &type, const QString &name, TiledScene *scene = nullptr, TiledObjectBase *object = nullptr);
	bool add(const QString &name, TiledScene *scene = nullptr, TiledObjectBase *object = nullptr);

	TiledTransport* find(const QString &name) const;
	TiledTransport* find(const TiledScene *scene) const;
	TiledTransport* find(const TiledObjectBase *object) const;
};

#endif // TILEDTRANSPORT_H
