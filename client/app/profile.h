/*
 * ---- Call of Suli ----
 *
 * profile.h
 *
 * Created on: 2021. 10. 03.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * profile
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

#ifndef PROFILE_H
#define PROFILE_H

#include "abstractactivity.h"
#include "profilescoreobject.h"
#include "objectlistmodel.h"

class Profile : public AbstractActivity
{
	Q_OBJECT

	Q_PROPERTY(QVariantList characterList READ characterList WRITE setCharacterList NOTIFY characterListChanged)
	Q_PROPERTY(ObjectGenericListModel<ProfileScoreObject>* scoreModel READ scoreModel WRITE setScoreModel NOTIFY scoreModelChanged)

public:
	Profile(QQuickItem *parent = nullptr);
	virtual ~Profile();

	QVariantList characterList() const { return m_characterList; }

	ObjectGenericListModel<ProfileScoreObject> *scoreModel() const;
	void setScoreModel(ObjectGenericListModel<ProfileScoreObject> *newScoreModel);

public slots:
	void setCharacterList(QVariantList characterList);
	int findCharacter(const QString &name);

	void scoreModelUpdate(const QJsonArray &list);

protected slots:
	void clientSetup() override;

signals:
	void userGet(QJsonObject jsonData, QByteArray binaryData);
	void userModify(QJsonObject jsonData, QByteArray binaryData);
	void userPasswordChange(QJsonObject jsonData, QByteArray binaryData);

	//void getUserScore(QJsonObject jsonData, QByteArray binaryData);
	void getAllUser(QJsonObject jsonData, QByteArray binaryData);
	void getUser(QJsonObject jsonData, QByteArray binaryData);
	void getMyGroups(QJsonObject jsonData, QByteArray binaryData);

	void groupCreate(QJsonObject jsonData, QByteArray binaryData);

	void characterListChanged(QVariantList characterList);
	void scoreModelChanged();

private:
	QVariantList m_characterList;
	ObjectGenericListModel<ProfileScoreObject> *m_scoreModel;
};

#endif // PROFILE_H
