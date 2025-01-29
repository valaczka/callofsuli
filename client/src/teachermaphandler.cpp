/*
 * ---- Call of Suli ----
 *
 * teachermaphandler.cpp
 *
 * Created on: 2023. 03. 31.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * TeacherMapHandler
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

#include "teachermaphandler.h"
#include "gamemap.h"
#include "utils_.h"
#include <sstream>
#include "tar_to_stream.h"

#ifdef Q_OS_WASM
#include "onlineclient.h"
#endif


/**
 * @brief TeacherMapHandler::TeacherMapHandler
 * @param parent
 */

TeacherMapHandler::TeacherMapHandler(QObject *parent)
	: BaseMapHandler{QStringLiteral("teachermaps"), parent}
	, m_mapList(new TeacherMapList(this))
{
	LOG_CTRACE("client") << "TeacherMapHandler created" << this;
}


/**
 * @brief TeacherMapHandler::~TeacherMapHandler
 */

TeacherMapHandler::~TeacherMapHandler()
{
	m_mapList->deleteLater();
	LOG_CTRACE("client") << "TeacherMapHandler destroyed" << this;
}



/**
 * @brief TeacherMapHandler::mapCreate
 * @param name
 */

void TeacherMapHandler::mapCreate(const QString &name)
{
	LOG_CDEBUG("client") << "Create map:" << name;

	GameMap *map = new GameMap();
	map->regenerateUuids();

	const QByteArray &b = qCompress(map->toBinaryData());

	delete map;
	map = nullptr;

	m_client->httpConnection()->send(HttpConnection::ApiTeacher, QStringLiteral("map/create/%1").arg(name), b)
			->fail(this, [this](const QString &err){m_client->messageWarning(err, tr("Pálya létrehozási hiba"));})
			->done(this, [this](const QJsonObject &){
		m_client->snack(tr("A pálya létrehozása sikerült"));
		reload();
	});
}





/**
 * @brief TeacherMapHandler::check
 * @param list
 */

void TeacherMapHandler::checkDownloads()
{
	for (TeacherMap *map : *m_mapList)
		BaseMapHandler::check(map);
}


/**
 * @brief TeacherMapHandler::mapExport
 * @param file
 * @param list
 * @return
 */

bool TeacherMapHandler::mapExport(const QUrl &file, const QList<TeacherMap *> &list) const
{
	QFile f(file.toLocalFile());

	if (!f.open(QIODevice::WriteOnly)) {
		m_client->messageError(tr("Nem lehet létrehozni a fájlt:\n%1").arg(file.toLocalFile()), tr("Belső hiba"));
		return false;
	}

	std::ostringstream _stream;

	for (const TeacherMap *map : list) {
		if (!map)
			continue;

		const auto &data = read(map);

		if (!data) {
			m_client->messageWarning(tr("A pálya nem olvasható: %1").arg(map->name()), tr("Hiba"));
			continue;
		}

		const QByteArray fname = map->name().toUtf8()+QByteArrayLiteral(" - ")+map->uuid().toUtf8()+QByteArrayLiteral(".map");

		tar_to_stream(_stream, fname.constData(), data->constData(), data->size(), static_cast<uint64_t>(std::time(0)));

		LOG_CDEBUG("client") << "Map" << map->uuid() << "exported:" << qPrintable(f.fileName());
	}

	tar_to_stream_tail(_stream);

	f.write(QByteArray::fromStdString(_stream.str()));

	f.close();

	return true;
}




/**
 * @brief TeacherMapHandler::reloadList
 */

void TeacherMapHandler::reloadList()
{
	m_client->httpConnection()->send(HttpConnection::ApiTeacher, QStringLiteral("map"))
			->fail(this, [this](const QString &err){m_client->messageWarning(err, tr("Letöltési hiba"));})
			->done(this, [this](const QJsonObject &data){
		const QJsonArray &list = data.value(QStringLiteral("list")).toArray();
		OlmLoader::loadFromJsonArray<TeacherMap>(m_mapList, list, "uuid", "uuid", true);
		checkDownloads();
		emit reloaded();
	});
}




/**
 * @brief TeacherMapHandler::loadEditorPage
 */

void TeacherMapHandler::loadEditorPage()
{
	if (!m_mapEditor)
		return;

	LOG_CTRACE("client") << "Teacher map editor page loaded" << m_mapEditor;

	QQuickItem *page = m_client->stackPushPage(QStringLiteral("PageMapEditor.qml"), {
												   { QStringLiteral("online"), true },
												   { QStringLiteral("mapEditor"), QVariant::fromValue(m_mapEditor) }
											   });

	if (page)
		connect(page, &QQuickItem::destroyed, this, &TeacherMapHandler::unsetMapEditor);
}





/**
 * @brief TeacherMapHandler::_mapImportContent
 * @param name
 * @param content
 */

void TeacherMapHandler::_mapImportContent(const QString &name, const QByteArray &content)
{
	GameMap *map = GameMap::fromBinaryData(content);

	if (!map)
		return m_client->messageError(tr("A fájl nem Call of Suli pályát tartalmaz!"), tr("Érvénytelen fájl"));

	delete map;
	map = nullptr;

	LOG_CDEBUG("client") << "Import map:" << qPrintable(name);

	const QByteArray &comp = qCompress(content);

	m_client->httpConnection()->send(HttpConnection::ApiTeacher, QStringLiteral("map/create/%1").arg(name), comp)
			->fail(this, [this](const QString &err){m_client->messageWarning(err, tr("Importálási hiba"));})
			->done(this, [this](const QJsonObject &){
		m_client->snack(tr("Az importálás sikerült"));
		reload();
	});
}



/**
 * @brief TeacherMapHandler::mapEditor
 * @return
 */

TeacherMapEditor *TeacherMapHandler::mapEditor() const
{
	return m_mapEditor;
}

void TeacherMapHandler::setMapEditor(TeacherMapEditor *newMapEditor)
{
	if (m_mapEditor == newMapEditor)
		return;
	m_mapEditor = newMapEditor;
	emit mapEditorChanged();
}


/**
 * @brief TeacherMapHandler::findMap
 * @param uuid
 * @return
 */

TeacherMap *TeacherMapHandler::findMap(const QString &uuid) const
{
	for (TeacherMap *m : *m_mapList) {
		if (m && m->uuid() == uuid)
			return m;
	}

	return nullptr;
}



#ifdef Q_OS_WASM

/**
 * @brief TeacherMapHandler::mapImportWasm
 */

void TeacherMapHandler::mapImportWasm()
{
	OnlineClient *client = dynamic_cast<OnlineClient*>(m_client);

	if (!client)
		return;

	client->wasmLoadFileToFileSystem(QStringLiteral("*"), this, &TeacherMapHandler::_mapImportContent);
}

#endif



/**
 * @brief TeacherMapHandler::unsetMapEditor
 */

void TeacherMapHandler::unsetMapEditor()
{
	if (m_mapEditor) {
		LOG_CTRACE("client") << "Teacher map editor unloaded" << m_mapEditor;
		delete m_mapEditor;
	}

	setMapEditor(nullptr);
}


/**
 * @brief TeacherMapHandler::mapList
 * @return
 */

TeacherMapList *TeacherMapHandler::mapList() const
{
	return m_mapList;
}




/**
 * @brief TeacherMapHandler::mapImport
 * @param file
 */

void TeacherMapHandler::mapImport(const QUrl &file)
{
	LOG_CDEBUG("client") << "Import map:" << file;

	if (file.isEmpty() || !file.isLocalFile()) {
		LOG_CERROR("client") << "Invalid URL:" << file;
		return m_client->messageError(tr("A fájl nem importálható"));
	}

	const auto &b = Utils::fileContent(file.toLocalFile());

	if (!b)
		return m_client->messageError(tr("A fájl nem importálható"));


	_mapImportContent(Utils::fileBaseName(file.toLocalFile()), *b);
}





/**
 * @brief TeacherMapHandler::mapDownload
 * @param map
 */

void TeacherMapHandler::mapDownload(TeacherMap *map)
{
	if (!map)
		return;

	download(map, HttpConnection::ApiTeacher, QStringLiteral("map/%1/content").arg(map->uuid()));
}



/**
 * @brief TeacherMapHandler::mapEdit
 * @param map
 */

void TeacherMapHandler::mapEdit(TeacherMap *map)
{
	if (!map)
		return;

	if (m_mapEditor)
		return m_client->messageError(tr("Egy pálya már meg van nyitva szerkesztésre!"), tr("Belső hiba"));


	TeacherMapEditor *editor = new TeacherMapEditor(this);

	editor->setDraftVersion(qMax(map->draftVersion(),0));
	editor->setDisplayName(map->name());
	editor->setUuid(map->uuid());

	setMapEditor(editor);

	HttpReply *r = m_client->httpConnection()->send(HttpConnection::ApiTeacher,
													QStringLiteral("map/%1/draft/%2").arg(editor->uuid()).arg(editor->draftVersion()))
			->fail(this, [this](const QString &err) {
		m_client->messageWarning(err, tr("Letöltési hiba"));
		unsetMapEditor();
	})
			->done(this, [this, map](const QByteArray &data) {
		if (map)
			map->setDownloadProgress(0);

		if (!m_mapEditor) {
			LOG_CWARNING("client") << "Invalid map editor";
			return;
		}

		const QByteArray &uncomp = qUncompress(data);

		if (uncomp.isEmpty()) {
			m_client->messageError(tr("Nem sikerült beolvasni a pálya tartalmát!"), tr("Belső hiba"));
			unsetMapEditor();
			return;
		}

		if (!m_mapEditor->loadFromBinaryData(uncomp)) {
			m_client->messageError(tr("Érvénytelen pálya!"), tr("Belső hiba"));
			unsetMapEditor();
			return;
		}

		loadEditorPage();
	});

	connect(r, &HttpReply::downloadProgress, map, &BaseMap::setDownloadProgress);

}





/**
 * @brief TeacherMapEditor::TeacherMapEditor
 * @param parent
 */

TeacherMapEditor::TeacherMapEditor(QObject *parent)
	: MapEditor(parent)
{
	connect(this, &MapEditor::saveRequest, this, &TeacherMapEditor::onSaveRequest);
	connect(this, &MapEditor::autoSaveRequest, this, &TeacherMapEditor::onSaveRequest);
}


/**
 * @brief TeacherMapEditor::~TeacherMapEditor
 */

TeacherMapEditor::~TeacherMapEditor()
{

}



/**
 * @brief TeacherMapEditor::onSaveRequest
 */

void TeacherMapEditor::onSaveRequest()
{
	if (!m_map || m_uuid.isEmpty())
		return;

	if (!modified())
		return;

	const QByteArray &data = qCompress(m_map->toBinaryData(true));

	m_client->httpConnection()->send(HttpConnection::ApiTeacher, QStringLiteral("map/%1/upload/%2").arg(m_uuid).arg(m_draftVersion), data)
			->fail(this, [this](const QString &){
		onSaved(false);
	})
			->done(this, [this](const QJsonObject &data){
		onSaved(true);
		if (data.contains(QStringLiteral("version")))
			setDraftVersion(data.value(QStringLiteral("version")).toInt());

		LOG_CTRACE("client") << "Map saved, new draftVersion:" << m_draftVersion;
	});
}



/**
 * @brief TeacherMapEditor::draftVersion
 * @return
 */

int TeacherMapEditor::draftVersion() const
{
	return m_draftVersion;
}

void TeacherMapEditor::setDraftVersion(int newDraftVersion)
{
	if (m_draftVersion == newDraftVersion)
		return;
	m_draftVersion = newDraftVersion;
	emit draftVersionChanged();
}



/**
 * @brief TeacherMapEditor::uuid
 * @return
 */

const QString &TeacherMapEditor::uuid() const
{
	return m_uuid;
}

void TeacherMapEditor::setUuid(const QString &newUuid)
{
	if (m_uuid == newUuid)
		return;
	m_uuid = newUuid;
	emit uuidChanged();
}


