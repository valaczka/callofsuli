/*
 * ---- Call of Suli ----
 *
 * exam.h
 *
 * Created on: 2023. 12. 31.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * Exam
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

#ifndef EXAM_H
#define EXAM_H

#include "qdatetime.h"
#include "qjsonobject.h"
#include "teachergroup.h"
#include <selectableobject.h>

#include <QObject>
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#include "QOlm/QOlm.hpp"
#pragma GCC diagnostic warning "-Wunused-parameter"
#pragma GCC diagnostic warning "-Wunused-variable"

class Exam;
using ExamList = qolm::QOlm<Exam>;
Q_DECLARE_METATYPE(ExamList*)


class Exam : public SelectableObject
{
	Q_OBJECT

	Q_PROPERTY(int examId READ examId WRITE setExamId NOTIFY examIdChanged FINAL)
	Q_PROPERTY(State state READ state WRITE setState NOTIFY stateChanged FINAL)
	Q_PROPERTY(Mode mode READ mode WRITE setMode NOTIFY modeChanged FINAL)
	Q_PROPERTY(QString mapUuid READ mapUuid WRITE setMapUuid NOTIFY mapUuidChanged FINAL)
	Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged FINAL)
	Q_PROPERTY(QDateTime timestamp READ timestamp WRITE setTimestamp NOTIFY timestampChanged FINAL)
	Q_PROPERTY(QJsonObject engineData READ engineData WRITE setEngineData NOTIFY engineDataChanged FINAL)

public:
	explicit Exam(QObject *parent = nullptr);
	virtual ~Exam();

	enum State {
		Prepare,
		Assigned,
		Active,
		Grading,
		Finished
	};

	Q_ENUM(State);

	enum Mode {
		ExamPaper,
		ExamOnline
	};

	Q_ENUM(Mode);

	Q_INVOKABLE void generateRandom(TeacherMapHandler *handler, TeacherGroup *group) const;

	Q_INVOKABLE void loadFromJson(const QJsonObject &object, const bool &allField = true);

	int examId() const;
	void setExamId(int newExamId);

	State state() const;
	void setState(const State &newState);

	Mode mode() const;
	void setMode(const Mode &newMode);

	QString mapUuid() const;
	void setMapUuid(const QString &newMapUuid);

	QString description() const;
	void setDescription(const QString &newDescription);

	QDateTime timestamp() const;
	void setTimestamp(const QDateTime &newTimestamp);

	QJsonObject engineData() const;
	void setEngineData(const QJsonObject &newEngineData);

signals:
	void examIdChanged();
	void stateChanged();
	void modeChanged();
	void mapUuidChanged();
	void descriptionChanged();
	void timestampChanged();
	void engineDataChanged();

private:
	int m_examId = -1;
	State m_state = Prepare;
	Mode m_mode = ExamPaper;
	QString m_mapUuid;
	QString m_description;
	QDateTime m_timestamp;
	QJsonObject m_engineData;
};

#endif // EXAM_H
