/*
 * ---- Call of Suli ----
 *
 * userimporter.cpp
 *
 * Created on: 2023. 07. 10.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * UserImporter
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

#include "userimporter.h"
#include "application.h"
#include "client.h"
#include "xlsxdocument.h"
#include "utils.h"

#ifdef Q_OS_WASM
#include "onlineclient.h"
#endif



const QHash<UserImporter::Field, QString> UserImporter::m_fieldMap = {
	{ UserImporter::Username, QStringLiteral("username") },
	{ UserImporter::FamilyName, QStringLiteral("familyName") },
	{ UserImporter::GivenName, QStringLiteral("givenName") },
	{ UserImporter::NickName, QStringLiteral("nickName") },
	{ UserImporter::Picture, QStringLiteral("picture") },
	{ UserImporter::OAuth2, QStringLiteral("oauth2") },
	{ UserImporter::Password, QStringLiteral("password") },
};


/**
 * @brief UserImporter::UserImporter
 * @param parent
 */

UserImporter::UserImporter(QObject *parent)
	: QObject{parent}
	, m_client(Application::instance()->client())
	#ifndef Q_OS_WASM
	, m_worker(new QLambdaThreadWorker())
	#endif
{
#ifdef Q_OS_WASM
	m_onlineClient = dynamic_cast<OnlineClient*>(m_client);
	Q_ASSERT(m_onlineClient);
#endif
}


/**
 * @brief UserImporter::~UserImporter
 */

UserImporter::~UserImporter()
{
#ifndef Q_OS_WASM
	delete m_worker;
	m_worker = nullptr;
#endif
}



/**
 * @brief UserImporter::downloadTemplate
 */

void UserImporter::downloadTemplate(const QUrl &file)
{
	if (m_templateContent.isEmpty())
		_generateTemplate();

	const QString &fileName = file.toLocalFile();

	QFile f(fileName);

	if (!f.open(QIODevice::WriteOnly)) {
		LOG_CWARNING("client") << "Can't write file:" << qPrintable(fileName);
		return;
	}

	f.write(m_templateContent);

	f.close();

	m_client->messageInfo(tr("Sablon letöltve:\n").append(fileName), tr("Letöltés"));
}






/**
 * @brief UserImporter::upload
 * @param file
 */

void UserImporter::upload(const QUrl &file)
{
	const QString &fileName = file.toLocalFile();
	const auto &b = Utils::fileContent(fileName);

	if (!b)
		return m_client->messageWarning(tr("Nem lehet megnyitni a fájlt:\n")+fileName, tr("Importálás"));

	LOG_CDEBUG("client") << "Import file:" << qPrintable(fileName);

	_load(*b);
}







/**
 * @brief UserImporter::_generateTemplate
 */

void UserImporter::_generateTemplate()
{
	QXlsx::Document doc;

	QXlsx::Format format;
	format.setBottomBorderStyle(QXlsx::Format::BorderMedium);
	format.setFontBold(true);


	QVector<QMap<Field, QString>> records = {
		{
			{ Username, tr("tothjanos") },
			{ FamilyName, tr("Tóth") },
			{ GivenName, tr("János") },
			{ NickName, tr("Jancsika") },
			{ Password, tr("TothJanosJelszava24")}
		},
		{
			{ Username, tr("kiss.ferenc@gmail.com") },
			{ NickName, tr("kiss_feri_beceneve") },
			{ OAuth2, QStringLiteral("google")}
		},
		{
			{ Username, tr("nagy.peter@microsoft.com") },
			{ FamilyName, tr("Nagy") },
			{ GivenName, tr("Péter") },
			{ Picture, tr("http://kep.hu/nagypeter.jpg")},
			{ OAuth2, QStringLiteral("microsoft")}
		}
	};

	int cell = 1;

	const QMetaEnum enumId = QMetaEnum::fromType<Field>();

	// Header

	for (int i=0; i<enumId.keyCount(); ++i) {
		const Field &field = QVariant(enumId.value(i)).value<Field>();
		const QString &name = m_fieldMap.value(field);
		if (name.isEmpty())
			continue;

		int row = 1;
		doc.write(row, cell, name, format);

		// Example records

		for (auto it = records.constBegin(); it != records.constEnd(); ++it) {
			const QString &txt = it->value(field);
			++row;

			if (txt.isEmpty())
				continue;

			doc.write(row, cell, txt);
		}

		++cell;
	}

	const double w = doc.columnWidth(1) * 6.;
	doc.setColumnWidth(1, cell, w);


	QBuffer buf;
	doc.saveAs(&buf);
	m_templateContent = buf.data();
}




/**
 * @brief UserImporter::_load
 * @param data
 */

void UserImporter::_load(const QByteArray &data)
{
	emit loadStarted();

	m_records = {};
	m_errorRecords = {};

#ifndef Q_OS_WASM
	m_worker->execInThread([this, data](){
#endif
		QBuffer buf;
		buf.setData(data);
		buf.open(QIODevice::ReadOnly);

		QXlsx::Document doc(&buf);

		buf.close();

		QHash<int, Field> headers;

		const QXlsx::CellRange &range = doc.dimension();

		for (int i=range.firstColumn(); i<=range.lastColumn(); ++i) {
			const QString &txt = doc.read(range.firstRow(), i).toString();

			const Field &field = m_fieldMap.key(txt, Invalid);

			if (field != Invalid)
				headers.insert(i, field);
		}

		if (headers.isEmpty()) {
			emit emptyDocument();
			emit recordsChanged();
			emit errorRecordsChanged();
			return;
		}

		QStringList usernameList;

		for (int row=range.firstRow()+1; row<=range.lastRow(); ++row) {
			QJsonObject rec;

			for (auto it = headers.constBegin(); it != headers.constEnd(); ++it) {
				const QVariant &cell = doc.read(row, it.key());

				if (cell.isNull())
					continue;

				rec.insert(m_fieldMap.value(it.value()), cell.toJsonValue());
			}

			if (rec.isEmpty())
				continue;

			QJsonArray errors;

			const QString &username = rec.value(m_fieldMap.value(Username)).toString();
			const QString &oauth = rec.value(m_fieldMap.value(OAuth2)).toString();
			const QString &password = rec.value(m_fieldMap.value(Password)).toString();

			if (username.isEmpty())
				errors.append(tr("Hiányzó felhasználónév"));

			if (usernameList.contains(username))
				errors.append(tr("A felhasználónév már szerepel az adatok között"));
			else
				usernameList.append(username);

			if (oauth.isEmpty() && password.isEmpty())
				errors.append(tr("Hiányzó jelszó"));
			else if (!oauth.isEmpty() &&
					 oauth != QLatin1String("google") &&
					 oauth != QLatin1String("microsoft"))
				errors.append(tr("Érvénytelen OAuth2 szolgáltató"));

			if (!errors.isEmpty()) {
				rec.insert(QStringLiteral("error"), errors);
				rec.insert(QStringLiteral("errorRow"), row);
				m_errorRecords.append(rec);
			} else
				m_records.append(rec);
		}

		emit recordsChanged();
		emit errorRecordsChanged();
		emit loadFinished();
#ifndef Q_OS_WASM
	});
#endif

}





const QJsonArray &UserImporter::errorRecords() const
{
	return m_errorRecords;
}

const QJsonArray &UserImporter::records() const
{
	return m_records;
}




#ifdef Q_OS_WASM

/**
 * @brief UserImporter::wasmDownloadTempate
 */

void UserImporter::wasmDownloadTemplate()
{
	if (m_templateContent.isEmpty())
		_generateTemplate();


	m_onlineClient->wasmSaveContent(m_templateContent, tr("user_import_template.xlsx"));
}


/**
 * @brief UserImporter::wasmUpload
 */

void UserImporter::wasmUpload()
{
	m_onlineClient->wasmLoadFileToFileSystem(QStringLiteral("*"), std::bind(&UserImporter::_load, this, std::placeholders::_2));
}

#endif

