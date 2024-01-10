/*
 * ---- Call of Suli ----
 *
 * exam.cpp
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

#include "exam.h"
#include "application.h"
#include "utils_.h"

Exam::Exam(QObject *parent)
	: SelectableObject{parent}
{

}

Exam::~Exam()
{

}


/**
 * @brief Exam::loadFromJson
 * @param object
 * @param allField
 */

void Exam::loadFromJson(const QJsonObject &object, const bool &allField)
{
	if (object.contains(QStringLiteral("id")) || allField)
		setExamId(object.value(QStringLiteral("id")).toInt());

	if (object.contains(QStringLiteral("mode")) || allField)
		setMode(object.value(QStringLiteral("mode")).toVariant().value<Mode>());

	if (object.contains(QStringLiteral("state")) || allField)
		setState(object.value(QStringLiteral("state")).toVariant().value<State>());

	if (object.contains(QStringLiteral("mapuuid")) || allField)
		setMapUuid(object.value(QStringLiteral("mapuuid")).toString());

	if (object.contains(QStringLiteral("description")) || allField)
		setDescription(object.value(QStringLiteral("description")).toString());

	if (object.contains(QStringLiteral("timestamp")) || allField)
		setTimestamp(QDateTime::fromSecsSinceEpoch(object.value(QStringLiteral("timestamp")).toInteger()));

	if (object.contains(QStringLiteral("engineData")) || allField)
		setEngineData(object.value(QStringLiteral("engineData")).toObject());

	// -- User --

	if (object.contains(QStringLiteral("data")) || allField)
		setExamData(object.value(QStringLiteral("data")).toArray());

	if (object.contains(QStringLiteral("answer")) || allField)
		setAnswerData(object.value(QStringLiteral("answer")).toArray());

	if (object.contains(QStringLiteral("correction")) || allField)
		setCorrectionData(object.value(QStringLiteral("correction")).toArray());

	if (object.contains(QStringLiteral("result")) || allField)
		setResult(object.value(QStringLiteral("result")).toDouble());

	if (object.contains(QStringLiteral("gradeid")) || allField)
		setResultGrade(qobject_cast<Grade*>(Application::instance()->client()->findCacheObject(QStringLiteral("gradeList"),
																							   object.value(QStringLiteral("gradeid")).toInt())));


}



/**
 * @brief Exam::resultoToTextDocument
 * @param document
 */

void Exam::resultToTextDocument(QTextDocument *document) const
{
	if (!document) {
		LOG_CERROR("client") << "Missing QTextDocument";
		return;
	}

	QFont font(QStringLiteral("Rajdhani"), 14);

	document->setDefaultFont(font);
	document->setDefaultStyleSheet(Utils::fileContent(QStringLiteral(":/gametest.css")).value_or(QByteArrayLiteral("")));
	document->setHtml(toHtml());

	QImage img = QImage::fromData(Utils::fileContent(":/internal/img/checkmark_green.png").value_or(QByteArray{}));

	document->addResource(QTextDocument::ImageResource, QUrl("imgdata://check.png"),
						  QVariant(img));
}



/**
 * @brief Exam::resultoToQuickTextDocument
 * @param document
 */

void Exam::resultToQuickTextDocument(QQuickTextDocument *document) const
{
	if (!document) {
		LOG_CERROR("client") << "Missing QQuickTextDocument";
		return;
	}

	resultToTextDocument(document->textDocument());
}



int Exam::examId() const
{
	return m_examId;
}

void Exam::setExamId(int newExamId)
{
	if (m_examId == newExamId)
		return;
	m_examId = newExamId;
	emit examIdChanged();
}

Exam::State Exam::state() const
{
	return m_state;
}

void Exam::setState(const State &newState)
{
	if (m_state == newState)
		return;
	m_state = newState;
	emit stateChanged();
}

Exam::Mode Exam::mode() const
{
	return m_mode;
}

void Exam::setMode(const Mode &newMode)
{
	if (m_mode == newMode)
		return;
	m_mode = newMode;
	emit modeChanged();
}

QString Exam::mapUuid() const
{
	return m_mapUuid;
}

void Exam::setMapUuid(const QString &newMapUuid)
{
	if (m_mapUuid == newMapUuid)
		return;
	m_mapUuid = newMapUuid;
	emit mapUuidChanged();
}

QString Exam::description() const
{
	return m_description;
}

void Exam::setDescription(const QString &newDescription)
{
	if (m_description == newDescription)
		return;
	m_description = newDescription;
	emit descriptionChanged();
}

QDateTime Exam::timestamp() const
{
	return m_timestamp;
}

void Exam::setTimestamp(const QDateTime &newTimestamp)
{
	if (m_timestamp == newTimestamp)
		return;
	m_timestamp = newTimestamp;
	emit timestampChanged();
}

QJsonObject Exam::engineData() const
{
	return m_engineData;
}

void Exam::setEngineData(const QJsonObject &newEngineData)
{
	if (m_engineData == newEngineData)
		return;
	m_engineData = newEngineData;
	emit engineDataChanged();
}

QJsonArray Exam::examData() const
{
	return m_examData;
}

void Exam::setExamData(const QJsonArray &newExamData)
{
	if (m_examData == newExamData)
		return;
	m_examData = newExamData;
	emit examDataChanged();
}

QJsonArray Exam::answerData() const
{
	return m_answerData;
}

void Exam::setAnswerData(const QJsonArray &newAnswerData)
{
	if (m_answerData == newAnswerData)
		return;
	m_answerData = newAnswerData;
	emit answerDataChanged();
}

QJsonArray Exam::correctionData() const
{
	return m_correctionData;
}

void Exam::setCorrectionData(const QJsonArray &newCorrectionData)
{
	if (m_correctionData == newCorrectionData)
		return;
	m_correctionData = newCorrectionData;
	emit correctionDataChanged();
}

qreal Exam::result() const
{
	return m_result;
}

void Exam::setResult(qreal newResult)
{
	if (qFuzzyCompare(m_result, newResult))
		return;
	m_result = newResult;
	emit resultChanged();
}

Grade *Exam::resultGrade() const
{
	return m_resultGrade;
}

void Exam::setResultGrade(Grade *newResultGrade)
{
	if (m_resultGrade == newResultGrade)
		return;
	m_resultGrade = newResultGrade;
	emit resultGradeChanged();
}



/**
 * @brief Exam::toHtml
 * @return
 */

QString Exam::toHtml() const
{
	TestGame::QuestionResult result;
	result.isExam = true;
	result.resultData = toQuestionData();

	QString html;

	html += QStringLiteral("<h1>%1</h1>").arg(m_description);


	// Result

	const qreal &percent = result.maxPoints > 0 ? result.points/result.maxPoints : 0;

	if (result.success)
		html += QStringLiteral("<p class=\"resultSuccess\">%1 (%2%)</p>").arg(tr("Sikeres megoldás")).arg(qFloor(percent*100));
	else
		html += QStringLiteral("<p class=\"resultFail\">%1 (%2%)</p>").arg(tr("Sikertelen megoldás")).arg(qFloor(percent*100));

	return TestGame::questionDataResultToHtml(html, result);
}


/**
 * @brief Exam::toQuestionData
 * @return
 */

QVector<TestGame::QuestionData> Exam::toQuestionData() const
{
	QVector<TestGame::QuestionData> list;

	for (int i=0; i<m_examData.size(); ++i) {
		const QJsonObject &obj = m_examData.at(i).toObject();
		TestGame::QuestionData d;
		d.data = obj.toVariantMap();
		d.module = obj.value(QStringLiteral("module")).toString();
		d.uuid = obj.value(QStringLiteral("uuid")).toString();

		if (i<m_answerData.size()) {
			d.answer = m_answerData.at(i).toObject().toVariantMap();
			d.success = d.answer.value(QStringLiteral("success"), false).toBool();
		}

		list.append(d);
	}

	return list;
}
