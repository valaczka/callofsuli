/*
 * ---- Call of Suli ----
 *
 * contexthelper.h
 *
 * Created on: 2025. 07. 06.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ContextHelper
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

#ifndef CONTEXTHELPER_H
#define CONTEXTHELPER_H

#include "qtimer.h"
#include <QObject>
#include <QSerializer>

class Client;


/**
 * @brief The ContextHelperData class
 */

class ContextHelperData : public QSerializer
{
	Q_GADGET

public:
	enum Context {
		ContextInvalid,
		ContextStart,
		ContextStudentDasboard,
		ContextTeacherDasboard,
	};

	Q_ENUM(Context);

	ContextHelperData()
		: QSerializer()
		, id(0)
	{}

	QS_SERIALIZABLE

	QS_FIELD(int, id)
	QS_FIELD(QString, title)
	QS_FIELD(QString, description)
	QS_FIELD(QString, icon)
	QS_FIELD(QString, image)
	QS_FIELD(QString, iconColor)
};

Q_DECLARE_METATYPE(ContextHelperData);





class ContextHelperStorage;


/**
 * @brief The ContextHelper class
 */

class ContextHelper : public QObject
{
	Q_OBJECT

	Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged FINAL)

public:
	explicit ContextHelper(Client *client);
	virtual ~ContextHelper();

	Q_INVOKABLE void download();
	Q_INVOKABLE void setCurrentContext(const ContextHelperData::Context &context);
	Q_INVOKABLE void unsetContext(const ContextHelperData::Context &context);

	void loadSettings();
	void saveSettings();

	bool enabled() const;
	void setEnabled(bool newEnabled);

signals:
	void enabledChanged();

private:
	void onTimerTimeout();
	std::optional<ContextHelperData> takeCurrentData();

	Client *const m_client;
	ContextHelperStorage *m_storage = nullptr;
	bool m_enabled = true;
	ContextHelperData::Context m_currentContext = ContextHelperData::ContextInvalid;
	QTimer m_timer;
};




#endif // CONTEXTHELPER_H
