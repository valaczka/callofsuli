/*
 * ---- Call of Suli ----
 *
 * ModuleWriter.cpp
 *
 * Created on: 2021. 11. 07.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ModuleWriter
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

#include "modulewriter.h"
#include "qqml.h"
#include "writerengine.h"
#include <QRandomGenerator>


ModuleWriter::ModuleWriter(QObject *parent) : QObject(parent)
{

}



/**
 * @brief ModuleWriter::testResult
 * @param data
 * @param answer
 * @param success
 * @return
 */

QString ModuleWriter::testResult(const QVariantMap &, const QVariantMap &answer, const bool &success) const
{
	QString html;

	if (success)
		html = QStringLiteral("<p class=\"answer\">");
	else
		html = QStringLiteral("<p class=\"answerFail\">");

	html += answer.value(QStringLiteral("text")).toString();

	html += QStringLiteral("</p>");

	return html;
}


/**
 * @brief ModuleWriter::details
 * @param data
 * @param storage
 * @param storageData
 * @return
 */

QVariantMap ModuleWriter::details(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData) const
{
	if (!storage) {
		QVariantMap m;
		m[QStringLiteral("title")] = data.value(QStringLiteral("question")).toString();
		m[QStringLiteral("details")] = data.value(QStringLiteral("correct")).toString();
		m[QStringLiteral("image")] = QLatin1String("");

		return m;
	} else if (storage->name() == QStringLiteral("binding")) {
		QStringList answers;

		bool isBindToRight = data.value(QStringLiteral("mode")).toString() == QStringLiteral("right");

		foreach (QVariant v, storageData.value(QStringLiteral("bindings")).toList()) {
			QVariantMap m = v.toMap();
			QString left = m.value(QStringLiteral("first")).toString();
			QString right = m.value(QStringLiteral("second")).toString();

			if (left.isEmpty() || right.isEmpty())
				continue;

			answers.append(QStringLiteral("%1 — %2").arg(isBindToRight ? right : left, isBindToRight ? left : right));
		}

		QVariantMap m;
		m[QStringLiteral("title")] = data.value(QStringLiteral("question")).toString();
		m[QStringLiteral("details")] = answers.join(QStringLiteral(", "));
		m[QStringLiteral("image")] = QLatin1String("");

		return m;
	} else if (storage->name() == QStringLiteral("images")) {
		QStringList answers;

		QString image = QLatin1String("");

		const QString &mode = data.value(QStringLiteral("mode")).toString();

		foreach (QVariant v, storageData.value(QStringLiteral("images")).toList()) {
			QVariantMap m = v.toMap();
			const int &imgId = m.value(QStringLiteral("image"), -1).toInt();
			const QString &text = m.value(QStringLiteral("text")).toString();

			if (imgId != -1 && image.isEmpty())
				image = QStringLiteral("image://mapimage/%1").arg(imgId);

			if (!text.isEmpty()) {
				const QString &answersPattern = data.value(QStringLiteral("answers")).toString();
				if (mode == QStringLiteral("image") && answersPattern.contains(QStringLiteral("%1")))
					answers.append(answersPattern.arg(text));
				else
					answers.append(text);
			}
		}

		QVariantMap m;
		m[QStringLiteral("title")] = data.value(QStringLiteral("question")).toString();
		m[QStringLiteral("details")] = answers.join(QStringLiteral(", "));
		m[QStringLiteral("image")] = image;

		return m;
	}

	return QVariantMap({{QStringLiteral("title"), QLatin1String("")},
						{QStringLiteral("details"), QLatin1String("")},
						{QStringLiteral("image"), QLatin1String("")}
					   });
}


/**
 * @brief ModuleWriter::generateAll
 * @param data
 * @param storage
 * @param storageData
 * @return
 */

QVariantList ModuleWriter::generateAll(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData) const
{
	if (!storage) {
		QVariantList list;
		QVariantMap m;

		m[QStringLiteral("question")] = data.value(QStringLiteral("question")).toString();

		QString correct = data.value(QStringLiteral("correct")).toString();

		if (correct.isEmpty())
			correct = QStringLiteral(" ");

		m[QStringLiteral("answer")] = data.value(QStringLiteral("correct")).toString();

		list.append(m);

		return list;
	}

	if (storage->name() == QStringLiteral("binding"))
		return generateBinding(data, storageData);

	if (storage->name() == QStringLiteral("images"))
		return generateImages(data, storageData);


	return QVariantList();
}




/**
 * @brief ModuleWriter::preview
 * @return
 */

QVariantMap ModuleWriter::preview(const QVariantList &generatedList) const
{
	QVariantMap m;
	QString s;

	foreach (QVariant v, generatedList) {
		QVariantMap m = v.toMap();

		const QString &image = m.value(QStringLiteral("image")).toString();

		s.append((image.isEmpty() ? QLatin1String("") : tr("[KÉP] "))
				 +QStringLiteral("**")+m.value(QStringLiteral("question")).toString()+QStringLiteral("**\n"));

		const QString &answer = m.value(QStringLiteral("answer")).toString();

		s.append(QStringLiteral("- ")+answer+QStringLiteral("\n"));

		s.append(QStringLiteral("---\n"));
	}

	m[QStringLiteral("text")] = s;

	return m;
}




/**
 * @brief ModuleWriter::registerQmlTypes
 */

void ModuleWriter::registerQmlTypes() const
{
	qmlRegisterType<WriterEngine>("CallOfSuli", 1, 0, "WriterEngine");
}


/**
 * @brief ModuleWriter::generateBinding
 * @param data
 * @param storageData
 * @return
 */

QVariantList ModuleWriter::generateBinding(const QVariantMap &data, const QVariantMap &storageData) const
{
	QVariantList ret;

	bool isBindToRight = data.value(QStringLiteral("mode")).toString() == QStringLiteral("right");
	QString question = data.value(QStringLiteral("question")).toString();

	foreach (QVariant v, storageData.value(QStringLiteral("bindings")).toList()) {
		QVariantMap m = v.toMap();
		QString left = m.value(QStringLiteral("first")).toString();
		QString right = m.value(QStringLiteral("second")).toString();

		if (left.isEmpty() || right.isEmpty())
			continue;

		QVariantMap retMap;

		if (question.isEmpty())
			retMap[QStringLiteral("question")] = (isBindToRight ? right : left);
		else if (question.contains(QStringLiteral("%1")))
			retMap[QStringLiteral("question")] = question.arg(isBindToRight ? right : left);
		else
			retMap[QStringLiteral("question")] = question;


		retMap[QStringLiteral("answer")] = isBindToRight ? left : right;

		ret.append(retMap);
	}

	return ret;
}


/**
 * @brief ModuleWriter::generateImages
 * @param data
 * @param storageData
 * @return
 */

QVariantList ModuleWriter::generateImages(const QVariantMap &data, const QVariantMap &storageData) const
{
	QVariantList ret;

	const QString &question = data.value(QStringLiteral("question")).toString();

	foreach (const QVariant &v, storageData.value(QStringLiteral("images")).toList()) {
		const QVariantMap &m = v.toMap();
		const int &imgId = m.value(QStringLiteral("second"), -1).toInt();
		const QString &text = m.value(QStringLiteral("first")).toString();

		if (imgId == -1 || text.isEmpty())
			continue;

		QVariantMap retMap;

		retMap[QStringLiteral("question")] = question;
		retMap[QStringLiteral("image")] = QStringLiteral("image://mapimage/%1").arg(imgId);
		retMap[QStringLiteral("answer")] = text;

		ret.append(retMap);
	}

	return ret;
}

