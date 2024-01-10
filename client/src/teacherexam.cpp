/*
 * ---- Call of Suli ----
 *
 * teacherexam.cpp
 *
 * Created on: 2024. 01. 03.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * TeacherExam
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

#include "teacherexam.h"
#include "Logger.h"
#include "BitMatrix.h"
#include "Matrix.h"
#include "MultiFormatWriter.h"
#include "application.h"
#include "qbuffer.h"
#include "utils_.h"
#include "QPageSize"
#include <QPdfWriter>
#include "stb_image_write.h"
#include "csv.hpp"
#include "examgame.h"



/**
 * @brief TeacherExam::m_optionLetters
 */

const QString TeacherExam::m_optionLetters = QStringLiteral("ABCDEF?????????????????????????????????????????????");



/**
 * @brief TeacherExam::TeacherExam
 * @param parent
 */

TeacherExam::TeacherExam(QObject *parent)
	: QObject{parent}
	, m_scanData(new ExamScanDataList)
	, m_examUserList(new ExamUserList)
{
	LOG_CTRACE("client") << "TeacherExam created" << this;

	connect(this, &TeacherExam::updateFromServerRequest, this, &TeacherExam::updateResultFromServer);
}


/**
 * @brief TeacherExam::~TeacherExam
 */

TeacherExam::~TeacherExam()
{
	LOG_CTRACE("client") << "TeacherExam destroyed" << this;
}



/**
 * @brief TeacherExam::createPdf
 * @param list
 */

void TeacherExam::createPdf(const QList<ExamUser *> &list, const PdfConfig &pdfConfig)
{
	LOG_CDEBUG("client") << "Generate paper exam pdf";

	m_worker.execInThread([this, list, pdfConfig]() {
		QByteArray content;
		QBuffer buffer(&content);
		buffer.open(QIODevice::WriteOnly);

		QPdfWriter pdf(&buffer);
		QPageLayout layout = pdf.pageLayout();
		layout.setUnits(QPageLayout::Millimeter);
		layout.setPageSize(QPageSize::A4);
		layout.setMargins(QMarginsF(0, 10, 0, 10));
		//layout.setMinimumMargins(QMarginsF(0, 0, 0, 0));
		//layout.setMode(QPageLayout::FullPageMode);

		LOG_CERROR("client") << layout.fullRectPoints();

		pdf.setCreator(QStringLiteral("Call of Suli - v")+Application::version());
		pdf.setTitle(pdfConfig.title);
		pdf.setPageLayout(layout);

		QTextDocument document;

		QFont font(QStringLiteral("Rajdhani"), pdfConfig.fontSize);

		document.setPageSize(QPageSize::sizePoints(QPageSize::A4));
		document.setDefaultFont(font);

		document.setDefaultStyleSheet(QStringLiteral("p { margin-bottom: 0px; margin-top: 0px; }"));

		QImage image = QImage::fromData(Utils::fileContentRead(QStringLiteral(":/internal/exam/bgL.png")));
		document.addResource(QTextDocument::ImageResource, QUrl(QStringLiteral("imgdata://bgL.png")), QVariant(image));

		QImage imageR = QImage::fromData(Utils::fileContentRead(QStringLiteral(":/internal/exam/bgR.png")));
		document.addResource(QTextDocument::ImageResource, QUrl(QStringLiteral("imgdata://bgR.png")), QVariant(imageR));

		QString html;

		html.append(QStringLiteral("<html><body>"));

		int count = 0;

		QList<ExamUser *> realList;

		if (list.isEmpty()) {
			realList.reserve(m_examUserList->size());
			for (ExamUser *u : *m_examUserList)
				realList.append(u);
		} else {
			realList = list;
		}

		for (const ExamUser *u : realList) {
			if (u->examData().isEmpty())
				continue;

			const QString &username = u->fullName();
			const int &id = u->contentId();
			const QJsonArray &qList = u->examData();

			html += QStringLiteral("<table width=\"100%\"");
			if (count>0)
				html += QStringLiteral(" style=\"page-break-before: always;\"");

			html += QStringLiteral("><tr><td><img width=30 src=\"imgdata://bgL.png\"></td><td>");

			html += pdfTitle(pdfConfig, username, id, &document);
			html += pdfSheet(count==0, layout.paintRectPoints().width()-80, &document);
			html += pdfQuestion(qList);

			html += QStringLiteral("</td><td><img width=30 src=\"imgdata://bgR.png\"></td></tr></table>");

			++count;

			/*
			// Check page

			++count;

			QString tmp = html + QStringLiteral("</body></html>");
			document.setHtml(tmp);

			int reqPages = pdfConfig.pagePerUser*count - document.pageCount();

			if (reqPages > 0)
				html += QStringLiteral("<p style=\"page-break-before: always;\">&nbsp;</p>").repeated(reqPages);
				*/
		}

		html.append(QStringLiteral("</body></html>"));

		document.setHtml(html);
		document.print(&pdf);

		buffer.close();

		QFile f("/tmp/out.pdf");
		f.open(QIODevice::WriteOnly);
		f.write(content);
		f.close();

		emit pdfFileGenerated("/tmp/out.pdf");
	});

}


/**
 * @brief TeacherExam::createPdf
 * @param list
 * @param pdfConfig
 * @param group
 */

void TeacherExam::createPdf(const QList<ExamUser*> &list, const QVariantMap &pdfConfig)
{
	PdfConfig c;

	if (m_exam) {
		c.examId = m_exam->examId();
		c.title = m_exam->description();
	}

	if (m_teacherGroup) {
		c.subject = m_teacherGroup->fullName();
	}

	if (pdfConfig.contains(QStringLiteral("fontSize")))
		c.fontSize = pdfConfig.value(QStringLiteral("fontSize")).toInt();

	createPdf(list, c);
}



/**
 * @brief TeacherExam::scanImageDir
 * @param path
 */

void TeacherExam::scanImageDir(const QUrl &path)
{
	if (m_scanState == ScanRunning) {
		LOG_CERROR("client") << "Scanning in progress";
		return;
	}

	m_scanData->clear();
	m_scanTempDir.reset();

	QDirIterator it(path.toLocalFile(), {
						QStringLiteral("*.png"),
						QStringLiteral("*.PNG"),
						QStringLiteral("*.jpg"),
						QStringLiteral("*.JPG"),
						QStringLiteral("*.jpeg"),
						QStringLiteral("*.JPEG"),
					}, QDir::Files);


	while (it.hasNext()) {
		ExamScanData *s = new ExamScanData;
		connect(s, &ExamScanData::uploadChanged, this, &TeacherExam::uploadableCountChanged);
		connect(s, &ExamScanData::stateChanged, this, &TeacherExam::uploadableCountChanged);
		connect(s, &ExamScanData::serverAnswerChanged, this, &TeacherExam::uploadableCountChanged);
		s->setPath(it.next());
		m_scanData->append(s);
	}

	if (m_scanData->empty())
		return;

	setScanState(ScanRunning);

	scanImages();
}


/**
 * @brief TeacherExam::remove
 * @param scan
 */

void TeacherExam::remove(ExamScanData *scan)
{
	if (!scan)
		return;

	m_worker.execInThread([this, scan](){
		QMutexLocker locker(&m_mutex);

		m_scanData->remove(scan);
		emit uploadableCountChanged();
	});
}


/**
 * @brief TeacherExam::removeSelected
 */

void TeacherExam::removeSelected()
{
	LOG_CTRACE("client") << "Remove selected scans";

	m_worker.execInThread([this](){
		QMutexLocker locker(&m_mutex);

		QVector<ExamScanData*> list;

		list.reserve(m_scanData->size());

		for (auto &s : *m_scanData) {
			if (s->selected())
				list.append(s);
		}

		list.squeeze();

		foreach (auto s, list)
			remove(s);
	});
}



/**
 * @brief TeacherExam::uploadResult
 */

void TeacherExam::uploadResult()
{
	LOG_CDEBUG("client") << "Upload exam results";

	m_worker.execInThread([this](){
		QMutexLocker locker(&m_mutex);

		QVector<QPointer<ExamScanData>> list;

		list.reserve(m_scanData->size());

		for (auto &s : *m_scanData) {
			if (s->state() == ExamScanData::ScanFileFinished && !s->serverAnswer().isEmpty() && s->upload())
				list.append(s);
		}

		list.squeeze();

		for (const auto &s : list) {
			if (s) {
				Application::instance()->client()->send(HttpConnection::ApiTeacher,
														QStringLiteral("exam/answer/%1").arg(s->contentId()), {
															{ QStringLiteral("answer"), s->serverAnswer() },
															{ QStringLiteral("correction"), s->correction() },
														})
						->done(this, [s, this](const QJsonObject &ret){
					if (s && ret.value(QStringLiteral("status")).toString() == QStringLiteral("ok")) {
						remove(s);
					}
				});
			}
		}

		Application::instance()->messageInfo(tr("Eredmények feltöltve"));
	});
}



/**
 * @brief TeacherExam::getMissionLevelList
 * @return
 */

QVariantList TeacherExam::getMissionLevelList()
{
	QVariantList list;

	loadGameMap();

	if (!m_gameMap)
		return {};

	for (GameMapMission *m : m_gameMap->missions()) {
		for (GameMapMissionLevel *ml : m->levels()) {
			list.append(QVariantMap{
							{ QStringLiteral("name"), m->name() },
							{ QStringLiteral("uuid"), m->uuid() },
							{ QStringLiteral("level"), ml->level() },
							{ QStringLiteral("missionLevel"), QVariant::fromValue(ml) }
						});
		}
	}

	return list;
}





/**
 * @brief TeacherExam::loadContentFromJson
 * @param object
 */

void TeacherExam::loadContentFromJson(const QJsonObject &object)
{
	LOG_CTRACE("client") << "Load exam content from json" << object;

	const QJsonArray &list = object.value(QStringLiteral("list")).toArray();

	for (ExamUser *u : *m_examUserList) {
		const QString &username = u->username();

		auto it = std::find_if(list.constBegin(), list.constEnd(), [username](const QJsonValue &v) {
			return (v.toObject().value(QStringLiteral("username")).toString() == username);
		});

		if (it != list.constEnd()) {
			const QJsonObject &o = it->toObject();
			u->setExamData(o.value(QStringLiteral("data")).toArray());
			u->setContentId(o.value(QStringLiteral("id")).toInt());
		} else {
			u->setExamData({});
			u->setContentId(0);
		}
	}
}



/**
 * @brief TeacherExam::generateExamContent
 */

void TeacherExam::generateExamContent(const QList<ExamUser*> &list)
{
	LOG_CDEBUG("client") << "Generate exam content";

	GameMapMissionLevel *ml = m_gameMap ? m_gameMap->missionLevel(m_missionUuid, m_level) : nullptr;

	if (!m_exam) {
		Application::instance()->messageError(tr("Nincs dolgozat beállítva"), tr("Belső hiba"));
		return;
	}

	if (!ml) {
		Application::instance()->messageWarning(tr("Nincs küldetés kiválasztva"), tr("Dolgozatok generálása"));
		return;
	}

	QJsonArray data;

	for (const ExamUser *u : list) {
		QJsonObject userdata;
		userdata[QStringLiteral("username")] = u->username();
		userdata[QStringLiteral("q")] = ExamGame::generatePaperQuestions(ml);

		data.append(userdata);
	}

	QJsonObject o;
	o[QStringLiteral("list")] = data;

	Application::instance()->client()->send(HttpConnection::ApiTeacher, QStringLiteral("exam/%1/create").arg(m_exam->examId()),
											o)
			->done(this, [this](const QJsonObject&) {
		reloadExamContent();
	});
}




/**
 * @brief TeacherExam::pickUsers
 * @param list
 */

void TeacherExam::pickUsers(QStringList userList, int count)
{
	// Get virtual exams data
	/*Application::instance()->client()->send(HttpConnection::ApiTeacher, QStringLiteral("exam/%1/create").arg(m_exam->examId()),
											o)
			->done(this, [this](const QJsonObject&) {
		if (m_exam && m_exam->mode() == Exam::ExamVirtual) {
			finish();
		} else {
			reloadExamContent();
		}
	});*/

	QList<ExamUser*> list = pickUsersRandom(count, userList, {});			// TODO: from server

	emit virtualListPicked(list);

	QJsonArray data;

	for (const ExamUser *u : *m_examUserList) {
		if (!userList.contains(u->username()))
			continue;

		QJsonObject userdata;
		QJsonObject qData;

		qData[QStringLiteral("picked")] = list.contains(u);

		userdata[QStringLiteral("username")] = u->username();
		userdata[QStringLiteral("q")] = QJsonArray{qData};

		data.append(userdata);
	}

	QJsonObject o;
	o[QStringLiteral("list")] = data;

	Application::instance()->client()->send(HttpConnection::ApiTeacher, QStringLiteral("exam/%1/create").arg(m_exam->examId()),
											o)
			->done(this, [this](const QJsonObject&) {
		finish();
		reloadExamContent();
	});
}



/**
 * @brief TeacherExam::reloadExamContent
 */

void TeacherExam::reloadExamContent()
{
	if (!m_exam)
		return;

	Application::instance()->client()->send(HttpConnection::ApiTeacher, QStringLiteral("exam/%1/content").arg(m_exam->examId()))
			->done(this, &TeacherExam::loadContentFromJson)
			;
}


/**
 * @brief TeacherExam::reload
 */

void TeacherExam::reload()
{
	if (!m_exam) {
		Application::instance()->messageError(tr("Nincs dolgozat kiválasztva"), tr("Belső hiba"));
		return;
	}

	LOG_CTRACE("client") << "Reload exam" << m_exam->examId();

	Application::instance()->client()->send(HttpConnection::ApiTeacher, QStringLiteral("exam/%1").arg(m_exam->examId()))
			->done(this, [this](const QJsonObject &obj){
		if (m_exam)
			m_exam->loadFromJson(obj, true);
	})
			->fail(this, [](const QString &err) {
		Application::instance()->client()->messageWarning(err, tr("Dolgozat letöltése"));
	});
}


/**
 * @brief TeacherExam::activate
 */

void TeacherExam::activate()
{
	if (!m_exam) {
		Application::instance()->messageError(tr("Nincs dolgozat kiválasztva"), tr("Belső hiba"));
		return;
	}

	LOG_CDEBUG("client") << "Activate exam" << m_exam->examId();

	Application::instance()->client()->send(HttpConnection::ApiTeacher, QStringLiteral("exam/%1/activate").arg(m_exam->examId()))
			->done(this, [this](const QJsonObject &){
		Application::instance()->client()->snack(tr("Dolgozat aktiválva"));
		emit examListReloadRequest();
		emit examActivated();
		reload();
	});

}


/**
 * @brief TeacherExam::inactivate
 */

void TeacherExam::inactivate()
{
	if (!m_exam) {
		Application::instance()->messageError(tr("Nincs dolgozat kiválasztva"), tr("Belső hiba"));
		return;
	}

	LOG_CDEBUG("client") << "Inactivate exam" << m_exam->examId();

	Application::instance()->client()->send(HttpConnection::ApiTeacher, QStringLiteral("exam/%1/inactivate").arg(m_exam->examId()))
			->done(this, [this](const QJsonObject &){
		Application::instance()->client()->snack(tr("Dolgozat inaktiválva"));
		emit examListReloadRequest();
		reload();
	});
}


/**
 * @brief TeacherExam::finish
 */

void TeacherExam::finish()
{
	if (!m_exam) {
		Application::instance()->messageError(tr("Nincs dolgozat kiválasztva"), tr("Belső hiba"));
		return;
	}

	LOG_CDEBUG("client") << "Finish exam" << m_exam->examId();

	Application::instance()->client()->send(HttpConnection::ApiTeacher, QStringLiteral("exam/%1/finish").arg(m_exam->examId()))
			->done(this, [this](const QJsonObject &){
		Application::instance()->client()->snack(tr("Dolgozat befejezve"));
		emit examListReloadRequest();
		reload();
	});
}









/**
 * @brief TeacherExam::pdfTitle
 * @param title
 * @param username
 * @param examId
 * @param contentId
 * @param document
 * @return
 */

QString TeacherExam::pdfTitle(const PdfConfig &pdfConfig, const QString &username, const int &contentId, QTextDocument *document)
{
	Q_ASSERT(document);

	const QString id = QStringLiteral("Call of Suli Exam %1/%2/%3/%4")
					   .arg(Application::versionMajor()).arg(Application::versionMinor())
					   .arg(pdfConfig.examId).arg(contentId);

	ZXing::BarcodeFormat format = ZXing::BarcodeFormat::QRCode;
	ZXing::MultiFormatWriter writer = ZXing::MultiFormatWriter(format).setMargin(0);

	ZXing::Matrix<uint8_t> bitmap = ZXing::ToMatrix<uint8_t>(writer.encode(id.toStdWString(), 120, 120));

	QImage image;

	stbi_write_png_to_func([](void *context, void *data, int size){
		QImage *img = (QImage*) context;
		*img = QImage::fromData((const unsigned char *) data, size);
	}, &image, bitmap.width(), bitmap.height(), 1, bitmap.data(), 0);


	const QString imgName = QStringLiteral("imgdata://id%1.png").arg(contentId);
	document->addResource(QTextDocument::ImageResource, QUrl(imgName), QVariant(image));

	return QStringLiteral("<table width=\"100%\" style=\"margin-left: 0px; margin-right: 30px;\">"
						  "<tr><td valign=middle><img height=60 src=\"%1\"></td>"
						  "<td width=\"100%\" valign=middle style=\"padding-left: 10px;\">"
						  "<p><span style=\"font-size: large;\"><b>%2</b></span><br/>%3<br/><small>%4</small></p>"
						  "</td></tr></table>\n\n")
			.arg(imgName, username, pdfConfig.title, pdfConfig.subject)
			;
}



/**
 * @brief TeacherExam::pdfSheet
 * @param document
 * @return
 */

QString TeacherExam::pdfSheet(const bool &addResource, const int &width, QTextDocument *document)
{
	Q_ASSERT(document);

	static const QString imgName = QStringLiteral("imgdata://sheet.svg");

	if (addResource) {
		QImage image = QImage::fromData(Utils::fileContentRead(QStringLiteral(":/internal/exam/sheet50.png")));
		document->addResource(QTextDocument::ImageResource, QUrl(imgName), QVariant(image));
	}

	return QStringLiteral("<table width=\"100%\"><tr>"
						  "<td align=center valign=middle style=\"padding-top: 5px; padding-bottom: 10px; margin-bottom: 10px; "
						  "border-bottom: 1px solid #cccccc;\">"
						  "<img src=\"%1\" width=\"%2\"></td></tr></table>").arg(imgName).arg(width);
}



/**
 * @brief TeacherExam::pdfQuestion
 * @param list
 * @return
 */

QString TeacherExam::pdfQuestion(const QJsonArray &list)
{
	QString html;

	int num = 1;

	static const QStringList nonNumberedModules = {
		QStringLiteral("pair"),
		QStringLiteral("fillout"),
		QStringLiteral("order"),
	};

	for (const QJsonValue &v : list) {
		const QJsonObject &obj = v.toObject();
		const QString &module = obj.value(QStringLiteral("module")).toString();
		const QString &question = obj.value(QStringLiteral("question")).toString();
		const int &point = obj.value(QStringLiteral("examPoint")).toInt();

		if (obj.value(QStringLiteral("separator")).toBool()) {
			html += QStringLiteral("<p align=center style=\"margin-top: 20px; margin-bottom: 20px;\">===============</p>");
		}

		html += QStringLiteral("<p style=\"margin-top: 6px;\" align=justify>");

		if (!nonNumberedModules.contains(module)) {
			html += QStringLiteral("<span style=\"font-weight: 600;\">");
			html += QString::number(num++).append(QStringLiteral("."));
			html += QStringLiteral("</span>");
		}

		LOG_CINFO("client") << "Q" << module << question;

		html += QStringLiteral("<i>");
		html += QObject::tr(" [%1p]").arg(point);
		html += QStringLiteral("</i>");

		html += QStringLiteral(" <span style=\"font-weight: 600;\">");
		html += question;
		html += QStringLiteral("</span>");

		if (module == QStringLiteral("simplechoice") ||
				module == QStringLiteral("multichoice") ||
				module == QStringLiteral("truefalse")) {
			const QJsonArray &options = module == QStringLiteral("truefalse")
										? QJsonArray{ tr("hamis"), tr("igaz") }
										: obj.value(QStringLiteral("options")).toArray();
			for (int i=0; i<options.size(); ++i) {
				html += QStringLiteral("&nbsp;&nbsp;&nbsp;<b>(")+m_optionLetters.at(i)
						+QStringLiteral(")</b> ")+options.at(i).toString();
				if (i<options.size()-1)
					html += QStringLiteral(",");
			}
		} else if (module == QStringLiteral("order")) {
			const QJsonArray &list = obj.value(QStringLiteral("list")).toArray();
			for (int i=0; i<list.size(); ++i) {
				html += QStringLiteral("&nbsp;&nbsp;&nbsp;<b>(")+m_optionLetters.at(i)
						+QStringLiteral(")</b> ")+list.at(i).toObject().value(QStringLiteral("text")).toString();
				if (i<list.size()-1)
					html += QStringLiteral(",");
			}

			html += QStringLiteral("</p>");
			html += QStringLiteral("<p style=\"margin-left: 30px;\" align=justify>");

			for (int i=0; i<list.size(); ++i) {
				if (i>0)
					html += QStringLiteral("&nbsp;&nbsp;&nbsp;");

				html += QStringLiteral("<b>")+QString::number(num++)+QStringLiteral(".</b> ")
						+QStringLiteral(" ___");

				if (i==0)
					html += QStringLiteral("(")+obj.value(QStringLiteral("placeholderMin")).toString()+QStringLiteral(")");
				if (i==list.size()-1)
					html += QStringLiteral("(")+obj.value(QStringLiteral("placeholderMax")).toString()+QStringLiteral(")");
			}
		} else if (module == QStringLiteral("pair")) {
			html += QStringLiteral("</p>");
			html += QStringLiteral("<p style=\"margin-left: 30px;\" align=justify>");

			const QJsonArray &list = obj.value(QStringLiteral("list")).toArray();
			for (int i=0; i<list.size(); ++i) {
				if (i>0)
					html += QStringLiteral("&nbsp;&nbsp;&nbsp;");

				html += QStringLiteral("<b>")+QString::number(num++)+QStringLiteral(".</b> ")
						+list.at(i).toString()
						+QStringLiteral(": ___");

				if (i<list.size()-1)
					html += QStringLiteral(",");
			}

			html += QStringLiteral("</p>");
			html += QStringLiteral("<p style=\"margin-left: 30px;\" align=justify>");

			const QJsonArray &options = obj.value(QStringLiteral("options")).toArray();
			for (int i=0; i<options.size(); ++i) {
				if (i>0)
					html += QStringLiteral("&nbsp;&nbsp;&nbsp;");

				html += QStringLiteral("<b>(")+m_optionLetters.at(i)
						+QStringLiteral(")</b> ")+options.at(i).toString();

				if (i<options.size()-1)
					html += QStringLiteral(",");
			}

		} else if (module == QStringLiteral("fillout")) {
			const QJsonArray &list = obj.value(QStringLiteral("list")).toArray();
			for (const QJsonValue &v : list) {
				const QJsonObject &data = v.toObject();

				if (data.contains(QStringLiteral("w")))
					html += QStringLiteral(" ")+data.value(QStringLiteral("w")).toString();
				else
					html += QStringLiteral(" <b>(")+QString::number(num++)+QStringLiteral(".)____</b>");
			}

			html += QStringLiteral("</p>");
			html += QStringLiteral("<p style=\"margin-left: 30px;\" align=justify>");

			const QJsonArray &options = obj.value(QStringLiteral("options")).toArray();
			for (int i=0; i<options.size(); ++i) {
				if (i>0)
					html += QStringLiteral("&nbsp;&nbsp;&nbsp;");

				html += QStringLiteral("<b>(")+m_optionLetters.at(i)
						+QStringLiteral(")</b> ")+options.at(i).toString();

				if (i<options.size()-1)
					html += QStringLiteral(",");
			}

		} else {
			//html += QJsonDocument(obj).toJson();
			html += QStringLiteral(" ___________");
		}

		html += QStringLiteral("</p>");
	}

	return html;
}



/**
 * @brief TeacherExam::loadUserList
 */

void TeacherExam::loadUserList()
{
	if (!m_teacherGroup) {
		m_examUserList->clear();
		return;
	}

	QList<ExamUser*> list;

	for (User *u : *m_teacherGroup->memberList()) {
		const QJsonObject &o = u->toJsonObject();
		ExamUser *eUser = new ExamUser;
		eUser->loadFromJson(o, true);
		list.append(eUser);
	}

	m_examUserList->append(list);
}





/**
 * @brief TeacherExam::loadGameMap
 */

void TeacherExam::loadGameMap()
{
	if (!m_exam || !m_mapHandler) {
		LOG_CWARNING("client") << "Missing exam/mapHandler";
		setGameMap({});
		return;
	}

	if (m_exam->mapUuid().isEmpty()) {
		Application::instance()->messageInfo(tr("Nincs beállítva pálya!"), tr("Dolgozat készítése"));
		setGameMap({});
		return;
	}

	auto mapList = m_mapHandler->mapList();

	TeacherMap *map = OlmLoader::find<TeacherMap>(mapList, "uuid", m_exam->mapUuid());

	if (!map) {
		Application::instance()->messageInfo(tr("Érvénytelen pályaazonosító!"), tr("Dolgozat készítése"));
		setGameMap({});
	}

	if (!map->downloaded()) {
		Application::instance()->messageInfo(tr("A dolgozat elkészítéséhez a kiválasztott pályát előbb le kell tölteni. A letöltés elindult."), tr("Pálya letöltése"));
		m_mapHandler->mapDownload(map);
		setGameMap({});
		return;
	}

	std::unique_ptr<GameMap> mapData;

	if (const auto &data = m_mapHandler->read(map); data) {
		mapData.reset(GameMap::fromBinaryData(data.value()));
	}

	if (!mapData) {
		Application::instance()->messageInfo(tr("Érvénytelen pálya!"), tr("Dolgozat készítése"));
		setGameMap({});
		return;
	}

	setGameMap(std::move(mapData));
}


/**
 * @brief TeacherExam::pickUsersRandom
 * @param userList
 * @param data
 */

QList<ExamUser*> TeacherExam::pickUsersRandom(const int &count, QStringList userList, const QJsonObject &data)
{
	QList<ExamUser*> list;

	while (!userList.isEmpty() && list.size() < count) {
		const QString &u = userList.takeAt(QRandomGenerator::global()->bounded(userList.size()));
		ExamUser *user = OlmLoader::find<ExamUser>(m_examUserList.get(), "username", u);
		if (!user)
			continue;
		list.append(user);
	}

	return list;
}



/**
 * @brief TeacherExam::scanImages
 */

void TeacherExam::scanImages()
{
	LOG_CTRACE("client") << "Scan images";

	m_worker.execInThread([this](){
		QMutexLocker locker(&m_mutex);

		for (auto it : *m_scanData) {
			const auto &data = Utils::fileContent(it->path());

			if (!data) {
				it->setState(ExamScanData::ScanFileError);
				continue;
			}

			QImage img = QImage::fromData(data.value());

			if (img.isNull()) {
				LOG_CWARNING("client") << "Invalid image:" << qPrintable(it->path());
				it->setState(ExamScanData::ScanFileError);
				continue;
			}

			ExamSBarcodeDecoder *decoder = new ExamSBarcodeDecoder;
			decoder->setPath(it->path());

			decoder->setResolution(img.width(), img.height());
			connect(decoder, &SBarcodeDecoder::isDecodingChanged, this, [this, decoder](bool d) {
				if (!d) {
					LOG_CTRACE("client") << "Decoding finished:" << decoder->path();
					processQRdata(decoder->path(), decoder->captured());
					decoder->deleteLater();
				}
			});
			decoder->process(img, ZXing::BarcodeFormat::QRCode);
		}
	});
}



/**
 * @brief TeacherExam::scanHasPendingQR
 * @return
 */

bool TeacherExam::scanHasPendingQR()
{
	QMutexLocker locker(&m_mutex);

	for (const auto &s : *m_scanData) {
		if (s->state() == ExamScanData::ScanFileLoaded)
			return true;
	}

	return false;
}


/**
 * @brief TeacherExam::processQRdata
 * @param path
 * @param state
 */

void TeacherExam::processQRdata(const QString &path, const QString &qr)
{
	m_worker.execInThread([this, path, qr](){
		QMutexLocker locker(&m_mutex);

		for (auto it : *m_scanData) {
			if (it->path() != path)
				continue;

			static const QString prefix = QStringLiteral("Call of Suli Exam ");

			if (!qr.startsWith(prefix)) {
				LOG_CWARNING("client") << "Invalid QR code:" << qr << "file:" << path;
				it->setState(ExamScanData::ScanFileInvalid);
				continue;
			}

			QString code = qr;
			code.remove(0, prefix.size());
			QStringList fields = code.split('/');

			if (fields.size() != 4) {
				LOG_CWARNING("client") << "Invalid QR code:" << qr << "file:" << path;
				it->setState(ExamScanData::ScanFileInvalid);
				continue;
			}

			const int vMajor = fields.at(0).toInt();
			const int vMinor = fields.at(1).toInt();

			if (Utils::versionCode(vMajor, vMinor) != Utils::versionCode()) {
				LOG_CWARNING("client") << "Wrong version:" << qr << "file:" << path;
				it->setState(ExamScanData::ScanFileInvalid);
				continue;
			}

			const int exam = fields.at(2).toInt();
			const int content = fields.at(3).toInt();

			if (!m_acceptedExamIdList.isEmpty() && !m_acceptedExamIdList.contains(exam)) {
				LOG_CWARNING("client") << "Exam not accepted:" << qr << "file:" << path;
				it->setState(ExamScanData::ScanFileInvalid);
				continue;
			}

			// Check duplicates

			bool has = false;

			for (const auto &d : *m_scanData) {
				if (d != it &&
						d->examId() == exam &&
						d->contentId() == content &&
						d->state() == ExamScanData::ScanFileReadQR) {
					has = true;
					break;
				}
			}

			if (has) {
				LOG_CWARNING("client") << "Sheet already exists:" << path;
				it->setState(ExamScanData::ScanFileInvalid);
				continue;
			}

			it->setState(ExamScanData::ScanFileReadQR);

			it->setExamId(exam);
			it->setContentId(content);

			LOG_CTRACE("client") << "Loaded QR image:" << path;
		}

		if (scanHasPendingQR())
			return;

		emit scanQRfinished();

		scanPreapareOMR();
	});
}


/**
 * @brief TeacherExam::scanPreapareOMR
 */

void TeacherExam::scanPreapareOMR()
{
	m_worker.execInThread([this](){
		QMutexLocker locker(&m_mutex);

		m_scanTempDir.reset(new QTemporaryDir);
		m_scanTempDir->setAutoRemove(true);

		LOG_CDEBUG("client") << "Prepare OMR to dir:" << qPrintable(m_scanTempDir->path());

		const QString dirInput = m_scanTempDir->filePath(QStringLiteral("input"));

		if (!QFile::exists(dirInput)) {
			QDir dir;
			if (!dir.mkpath(dirInput)) {
				setScanState(ScanErrorFileSystem);
				LOG_CERROR("client") << "Directory create error:" << qPrintable(dirInput);
				return;
			}
		}

		bool omr = false;

		for (auto it : *m_scanData) {
			if (it->state() == ExamScanData::ScanFileReadQR) {
				QFileInfo f(it->path());
				QString newName = dirInput+QStringLiteral("/sheet_%1_%2.%3").arg(it->examId()).arg(it->contentId()).arg(f.suffix());
				if (QFile::copy(it->path(), newName)) {
					LOG_CTRACE("client") << "Copy:" << qPrintable(it->path()) << "->" << qPrintable(newName);
					it->setState(ExamScanData::ScanFileReadingOMR);
					omr = true;
				} else {
					LOG_CERROR("client") << "Copy error:" << qPrintable(it->path()) << "->" << qPrintable(newName);
					it->setState(ExamScanData::ScanFileError);
				}
			}
		}

		if (!omr) {
			LOG_CWARNING("client") << "Scannable image not found";
			m_scannedIdList = {};
			setScanState(ScanFinished);
			m_scanTempDir.reset();
			return;
		}

		runOMR();
	});
}




/**
 * @brief TeacherExam::runOMR
 */

void TeacherExam::runOMR()
{
	m_worker.execInThread([this](){
		if (m_omrProcess) {
			setScanState(ScanErrorOmrInProgress);
			LOG_CERROR("client") << "OMR in progress";
			return;
		}

		if (!m_scanTempDir) {
			setScanState(ScanErrorFileSystem);
			LOG_CERROR("client") << "Missing image directory";
			return;
		}

		if (!QFile::copy(QStringLiteral(":/internal/exam/template.json"), m_scanTempDir->filePath(QStringLiteral("input/template.json")))) {
			setScanState(ScanErrorFileSystem);
			LOG_CERROR("client") << "Template copy error";
			return;
		}

		if (!QFile::copy(QStringLiteral(":/internal/exam/marker.png"), m_scanTempDir->filePath(QStringLiteral("input/marker.png")))) {
			setScanState(ScanErrorFileSystem);
			LOG_CERROR("client") << "Marker copy error";
			return;
		}

		QStringList searchList;

		QString binDir = QCoreApplication::applicationDirPath();

		searchList.append(binDir);
		searchList.append(binDir+"/share");

#ifndef QT_NO_DEBUG
		searchList.append(binDir+"/../callofsuli/share");
		searchList.append(binDir+"/../../callofsuli/share");
		searchList.append(binDir+"/../../../callofsuli/share");
#endif

		searchList.removeDuplicates();

		QString program;

		foreach (QString dir, searchList)
		{
			QString p = dir.append(QStringLiteral("/OMRChecker/main.py"));
			if (QFile::exists(p)) {
				program = p;
				break;
			}
		}

		if (program.isEmpty()) {
			setScanState(ScanErrorOmrNotFound);
			LOG_CERROR("client") << "OMR main.py not found";
			return;
		}


		QStringList arguments;
		arguments << program;
		arguments << QStringLiteral("-i");
		arguments << m_scanTempDir->filePath(QStringLiteral("input"));
		arguments << QStringLiteral("-o");
		arguments << m_scanTempDir->filePath(QStringLiteral("output"));

		m_omrProcess = std::make_unique<QProcess>();
		m_omrProcess->setProgram("/home/valaczka/usr/python/bin/python3");
		m_omrProcess->setArguments(arguments);

		connect(m_omrProcess.get(), &QProcess::finished, this, &TeacherExam::onOmrFinished);

		LOG_CINFO("client") << "Start OMR";

		m_omrProcess->start();
	});
}





/**
 * @brief TeacherExam::onOmrFinished
 * @param exitCode
 * @param exitStatus
 */
void TeacherExam::onOmrFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
	if (m_omrProcess) {
		auto p = m_omrProcess.release();
		p->deleteLater();
	}

	if (!m_scanTempDir) {
		setScanState(ScanErrorFileSystem);
		LOG_CERROR("client") << "Missing image directory";
		return;
	}

	if (exitStatus != QProcess::NormalExit || exitCode != 0) {
		setScanState(ScanErrorOmr);
		LOG_CERROR("client") << "OMR error";
		return;
	}

	LOG_CINFO("client") << "OMR finished successful";

	csv::CSVFormat format;
	format.delimiter(',').quote('"').header_row(0);

	csv::CSVReader reader(m_scanTempDir->filePath(QStringLiteral("output/Results/Results.csv")).toStdString(), format);

	auto columns = reader.get_col_names();

	for (auto it=columns.begin(); it != columns.end(); ) {
		static const QRegularExpression exp(R"(^q\d+$)");
		const QString &header = QString::fromStdString(*it);
		if (exp.match(header).hasMatch() || header == QStringLiteral("output_path"))
			++it;
		else
			it = columns.erase(it);
	}


	QJsonArray answers;

	for (const csv::CSVRow &row : reader) {
		static const QRegularExpression exp(R"(^sheet_(\d+)_(\d+))");

		QJsonObject data = QJsonDocument::fromJson(row.to_json(columns).c_str()).object();
		const auto &match = exp.match(QString::fromStdString(row["file_id"].get()));
		if (match.hasMatch()) {
			data[QStringLiteral("exam")] = match.captured(1).toInt();
			data[QStringLiteral("content")] = match.captured(2).toInt();
		}

		answers.append(data);
	}

	emit scanOMRfinished();

	processOMRdata(answers);
}





/**
 * @brief TeacherExam::processOMRdata
 * @param data
 */

void TeacherExam::processOMRdata(const QJsonArray &data)
{
	LOG_CTRACE("client") << "Process OMR data" << data;

	m_worker.execInThread([this, data]() {
		QMutexLocker locker(&m_mutex);

		QJsonArray idList;

		for (const QJsonValue &v : data) {
			const QJsonObject &obj = v.toObject();
			const int examId = obj.value(QStringLiteral("exam")).toInt(-1);
			const int contentId = obj.value(QStringLiteral("content")).toInt(-1);

			for (auto &s : *m_scanData) {
				if (s->examId() == examId && s->contentId() == contentId && s->state() == ExamScanData::ScanFileReadingOMR) {
					s->setResult(obj);
					s->setState(ExamScanData::ScanFileFinished);
					s->setOutputPath(obj.value(QStringLiteral("output_path")).toString());
					idList.append(contentId);
				}
			}
		}

		for (auto &s : *m_scanData) {
			if (s->state() != ExamScanData::ScanFileInvalid &&
					s->state() != ExamScanData::ScanFileError &&
					s->state() != ExamScanData::ScanFileFinished) {
				s->setState(ExamScanData::ScanFileInvalid);
			}
		}

		m_scannedIdList = idList;
		setScanState(ScanFinished);

		emit updateFromServerRequest();
	});
}



/**
 * @brief TeacherExam::generateAnswerResult
 * @param list
 * @param content
 */

void TeacherExam::generateAnswerResult(const QJsonObject &content)
{
	LOG_CTRACE("client") << "Generate answer result";

	m_worker.execInThread([this, content]() {
		QMutexLocker locker(&m_mutex);

		QVector<ExamScanData*> usedList;

		const QJsonArray &cData = content.value(QStringLiteral("list")).toArray();

		for (const QJsonValue &v : cData) {
			const QJsonObject &o = v.toObject();
			const int examId = o.value(QStringLiteral("examId")).toInt();
			const int contentId = o.value(QStringLiteral("id")).toInt();

			for (auto &s : *m_scanData) {
				if (s->examId() == examId && s->contentId() == contentId && s->state() == ExamScanData::ScanFileFinished) {
					QString username = o.value(QStringLiteral("username")).toString();
					if (m_teacherGroup) {
						if (User *u = OlmLoader::find<User>(m_teacherGroup->memberList(), "username", username); u)
							username = u->fullName();
					}

					s->setUsername(username);
					QJsonArray r, corr;
					getResult(o.value(QStringLiteral("data")).toArray(), s->result(), &r, &corr);
					LOG_CDEBUG("client") << "Update server answer" << s->contentId() << s->username() << r;
					s->setServerAnswer(r);
					s->setCorrection(corr);
					usedList.append(s);
				}
			}
		}


		for (auto &s : *m_scanData) {
			if (s->state() == ExamScanData::ScanFileFinished && !usedList.contains(s))
				s->setState(ExamScanData::ScanFileInvalid);
		}
	});
}


/**
 * @brief TeacherExam::getResult
 * @param qList
 * @param answer
 * @return
 */



void TeacherExam::getResult(const QJsonArray &qList, const QJsonObject &answer, QJsonArray *result, QJsonArray *correction) const
{
	int num = 1;

	/*static const QStringList nonNumberedModules = {
		QStringLiteral("pair"),
		QStringLiteral("fillout"),
		QStringLiteral("order"),
	};*/

	QJsonArray ret;
	QJsonArray corr;

	for (const QJsonValue &v : qList) {
		const QJsonObject &obj = v.toObject();
		const QString &module = obj.value(QStringLiteral("module")).toString();
		const QString &question = obj.value(QStringLiteral("question")).toString();
		const QJsonValue &correctAnswer = obj.value(QStringLiteral("answer"));
		const int &point = obj.value(QStringLiteral("examPoint")).toInt();

		if (module == QStringLiteral("truefalse")) {
			const auto &aList = letterToOptions(answer, num++);
			if (aList.size() != 1 || aList.at(0) == -1) {
				ret.append(QJsonValue{});
				corr.append(0);
			} else {
				const int &a = aList.at(0);
				const bool success = (correctAnswer.toInt(-1) == a);

				ret.append(QJsonObject{
							   { QStringLiteral("index"), a },
							   { QStringLiteral("success"), success }
						   });
				corr.append(success ? point : 0);
			}

		} else if (module == QStringLiteral("simplechoice")) {
			const auto &aList = letterToOptions(answer, num++);
			if (aList.size() != 1 || aList.at(0) == -1) {
				ret.append(QJsonValue{});
				corr.append(0);
			} else {
				const int &a = aList.at(0);
				const bool success = (correctAnswer.toInt(-1) == a);

				ret.append(QJsonObject{
							   { QStringLiteral("index"), a },
							   { QStringLiteral("success"), success }
						   });
				corr.append(success ? point : 0);
			}

		} else if (module == QStringLiteral("multichoice")) {
			const auto &aList = letterToOptions(answer, num++);
			QJsonArray r;
			for (const int &i : aList) {
				if (i != -1)
					r.append(i);
			}

			bool success = false;

			if (r.size() != correctAnswer.toArray().size()) {
				corr.append(0);
			} else {
				for (const QJsonValue &v : correctAnswer.toArray()) {
					if (auto it = std::find_if(r.begin(), r.end(), [v](const QJsonValue &q){
											   return q.toInt(-1) == v.toInt(-2);
				}); it != r.end()) {
						r.erase(it);
					}
				}

				success = r.isEmpty();
				corr.append(success ? point : 0);
			}

			ret.append(QJsonObject{
						   { QStringLiteral("list"), r },
						   { QStringLiteral("success"), success }
					   });

		} else if (module == QStringLiteral("order")) {
			const QJsonArray &list = obj.value(QStringLiteral("list")).toArray();

			QJsonArray retList;
			bool success = true;

			for (int i=0; i<list.size(); ++i) {
				const auto &v = letterToInt(answer, num++);

				if (v.isNull()) {
					retList.append(QJsonObject{
									   { QStringLiteral("answer"), QJsonValue::Null },
									   { QStringLiteral("success"), false }
								   });
					success = false;
				} else {
					const QJsonArray &aList = correctAnswer.toArray();
					const int vv = i<aList.size() ? aList.at(i).toInt(-1) : -1;
					if (const int &vInt = v.toInt(-2); vInt == vv) {
						retList.append(QJsonObject{
										   { QStringLiteral("answer"), vInt },
										   { QStringLiteral("success"), true }
									   });
					} else {
						retList.append(QJsonObject{
										   { QStringLiteral("answer"), vInt },
										   { QStringLiteral("success"), false }
									   });
						success = false;
					}
				}
			}

			ret.append(QJsonObject{
						   { QStringLiteral("list"), retList },
						   { QStringLiteral("success"), success }
					   });

			corr.append(success ? point : 0);

		} else if (module == QStringLiteral("pair")) {
			const QJsonArray &options = obj.value(QStringLiteral("options")).toArray();
			const QJsonArray &list = obj.value(QStringLiteral("list")).toArray();
			QJsonArray l;

			for (int i=0; i<list.size(); ++i) {
				const auto &v = letterToInt(answer, num++);

				if (v.isNull())
					l.append(QJsonValue::Null);
				else {
					const int idx = v.toInt(-1);
					if (idx<0 || idx>=options.size())
						l.append(QJsonValue::Null);
					else
						l.append(options.at(idx));
				}
			}

			QJsonArray retList;
			bool success = true;
			const QJsonArray &aList = correctAnswer.toObject().value(QStringLiteral("list")).toArray();
			for (int i=0; i<aList.size(); ++i) {

				if (i >= l.size() || l.at(i).isNull()) {
					retList.append(QJsonObject{
									   { QStringLiteral("answer"), QJsonValue::Null },
									   { QStringLiteral("success"), false }
								   });
					success = false;
				} else {
					const bool s = (l.at(i).toString() == aList.at(i).toString());
					retList.append(QJsonObject{
									   { QStringLiteral("answer"), l.at(i).toString() },
									   { QStringLiteral("success"), s }
								   });
					if (!s)
						success = false;
				}
			}

			ret.append(QJsonObject{
						   { QStringLiteral("list"), retList },
						   { QStringLiteral("success"), success }
					   });

			corr.append(success ? point : 0);
		} else if (module == QStringLiteral("fillout")) {
			const QJsonArray &options = obj.value(QStringLiteral("options")).toArray();
			const QJsonArray &list = obj.value(QStringLiteral("list")).toArray();

			QJsonObject l;

			for (const QJsonValue &v : list) {
				const QJsonObject &data = v.toObject();

				const QString &q = data.value(QStringLiteral("q")).toString();

				if (q.isEmpty())
					continue;

				const auto &i = letterToInt(answer, num++);

				if (i.isNull())
					l.insert(q, QJsonValue::Null);
				else {
					const int idx = i.toInt(-1);
					if (idx<0 || idx>=options.size())
						l.insert(q, QJsonValue::Null);
					else
						l.insert(q, options.at(idx));
				}
			}

			ret.append(l);

		} else {
			ret.append(QJsonValue::Null);
		}
	}

	if (result)
		*result = ret;

	if (correction)
		*correction = corr;
}



/**
 * @brief TeacherExam::updateResultFromServer
 */

void TeacherExam::updateResultFromServer()
{
	LOG_CTRACE("client") << "Update result from server";

	if (m_scannedIdList.isEmpty()) {
		LOG_CWARNING("client") << "All of OMR results are invalid";
		return;
	}

	Application::instance()->client()->send(HttpConnection::ApiTeacher, "exam/content", {
												{ QStringLiteral("list"), m_scannedIdList }
											})
			->done(this, &TeacherExam::generateAnswerResult);
}



/**
 * @brief TeacherExam::examUserList
 * @return
 */

ExamUserList*TeacherExam::examUserList() const
{
	return m_examUserList.get();
}

int TeacherExam::level() const
{
	return m_level;
}

void TeacherExam::setLevel(int newLevel)
{
	if (m_level == newLevel)
		return;
	m_level = newLevel;
	emit levelChanged();
}

QString TeacherExam::missionUuid() const
{
	return m_missionUuid;
}

void TeacherExam::setMissionUuid(const QString &newMissionUuid)
{
	if (m_missionUuid == newMissionUuid)
		return;
	m_missionUuid = newMissionUuid;
	emit missionUuidChanged();
}


/**
 * @brief TeacherExam::gameMap
 * @return
 */

GameMap*TeacherExam::gameMap() const
{
	return m_gameMap.get();
}

void TeacherExam::setGameMap(std::unique_ptr<GameMap> newGameMap)
{
	if (m_gameMap == newGameMap)
		return;
	m_gameMap = std::move(newGameMap);
	emit gameMapChanged();
	setMissionUuid(QStringLiteral(""));
	setLevel(-1);
}

Exam *TeacherExam::exam() const
{
	return m_exam;
}

void TeacherExam::setExam(Exam *newExam)
{
	if (m_exam == newExam)
		return;
	m_exam = newExam;
	emit examChanged();
}

TeacherMapHandler *TeacherExam::mapHandler() const
{
	return m_mapHandler;
}

void TeacherExam::setMapHandler(TeacherMapHandler *newMapHandler)
{
	if (m_mapHandler == newMapHandler)
		return;
	m_mapHandler = newMapHandler;
	emit mapHandlerChanged();
}



/**
 * @brief TeacherExam::uploadableCount
 * @return
 */

int TeacherExam::uploadableCount() const
{
	int n = 0;

	for (auto s : *m_scanData) {
		if (s->state() == ExamScanData::ScanFileFinished && !s->serverAnswer().isEmpty() && s->upload())
			++n;
	}

	return n;
}



/**
 * @brief TeacherExam::teacherGroup
 * @return
 */

TeacherGroup *TeacherExam::teacherGroup() const
{
	return m_teacherGroup;
}

void TeacherExam::setTeacherGroup(TeacherGroup *newTeacherGroup)
{
	if (m_teacherGroup == newTeacherGroup)
		return;
	m_teacherGroup = newTeacherGroup;
	emit teacherGroupChanged();

	loadUserList();
}


/**
 * @brief TeacherExam::acceptedExamIdList
 * @return
 */

QVariantList TeacherExam::acceptedExamIdList() const
{
	return m_acceptedExamIdList;
}

void TeacherExam::setAcceptedExamIdList(const QVariantList &newAcceptedExamIdList)
{
	if (m_acceptedExamIdList == newAcceptedExamIdList)
		return;
	m_acceptedExamIdList = newAcceptedExamIdList;
	emit acceptedExamIdListChanged();
}


/**
 * @brief TeacherExam::scanState
 * @return
 */

TeacherExam::ScanState TeacherExam::scanState() const
{
	return m_scanState;
}

void TeacherExam::setScanState(const ScanState &newScanState)
{
	if (m_scanState == newScanState)
		return;
	m_scanState = newScanState;
	emit scanStateChanged();
}


/**
 * @brief TeacherExam::scanData
 * @return
 */

ExamScanDataList* TeacherExam::scanData() const
{
	return m_scanData.get();
}







/**
 * @brief ExamScanData::ExamScanData
 * @param parent
 */

ExamScanData::ExamScanData(QObject *parent)
	: SelectableObject(parent)
{

}

QString ExamScanData::path() const
{
	return m_path;
}

void ExamScanData::setPath(const QString &newPath)
{
	if (m_path == newPath)
		return;
	m_path = newPath;
	emit pathChanged();
}

ExamScanData::ScanFileState ExamScanData::state() const
{
	return m_state;
}

void ExamScanData::setState(const ScanFileState &newState)
{
	if (m_state == newState)
		return;
	m_state = newState;
	emit stateChanged();
}

int ExamScanData::examId() const
{
	return m_examId;
}

void ExamScanData::setExamId(int newExamId)
{
	if (m_examId == newExamId)
		return;
	m_examId = newExamId;
	emit examIdChanged();
}

int ExamScanData::contentId() const
{
	return m_contentId;
}

void ExamScanData::setContentId(int newContentId)
{
	if (m_contentId == newContentId)
		return;
	m_contentId = newContentId;
	emit contentIdChanged();
}

QJsonObject ExamScanData::result() const
{
	return m_result;
}

void ExamScanData::setResult(const QJsonObject &newResult)
{
	if (m_result == newResult)
		return;
	m_result = newResult;
	emit resultChanged();
}

QString ExamScanData::outputPath() const
{
	return m_outputPath;
}

void ExamScanData::setOutputPath(const QString &newOutputPath)
{
	if (m_outputPath == newOutputPath)
		return;
	m_outputPath = newOutputPath;
	emit outputPathChanged();
}

QJsonArray ExamScanData::serverAnswer() const
{
	return m_serverAnswer;
}

void ExamScanData::setServerAnswer(const QJsonArray &newServerAnswer)
{
	if (m_serverAnswer == newServerAnswer)
		return;
	m_serverAnswer = newServerAnswer;
	emit serverAnswerChanged();
}

QString ExamScanData::username() const
{
	return m_username;
}

void ExamScanData::setUsername(const QString &newUsername)
{
	if (m_username == newUsername)
		return;
	m_username = newUsername;
	emit usernameChanged();
}

bool ExamScanData::upload() const
{
	return m_upload;
}

void ExamScanData::setUpload(bool newUpload)
{
	if (m_upload == newUpload)
		return;
	m_upload = newUpload;
	emit uploadChanged();
}

QJsonArray ExamScanData::correction() const
{
	return m_correction;
}

void ExamScanData::setCorrection(const QJsonArray &newCorrection)
{
	if (m_correction == newCorrection)
		return;
	m_correction = newCorrection;
	emit correctionChanged();
}



/**
 * @brief ExamUser::ExamUser
 * @param parent
 */

ExamUser::ExamUser(QObject *parent)
	: User(parent)
{

}


/**
 * @brief ExamUser::examData
 * @return
 */

QJsonArray ExamUser::examData() const
{
	return m_examData;
}

void ExamUser::setExamData(const QJsonArray &newExamData)
{
	if (m_examData == newExamData)
		return;
	m_examData = newExamData;
	emit examDataChanged();
}

int ExamUser::contentId() const
{
	return m_contentId;
}

void ExamUser::setContentId(int newContentId)
{
	if (m_contentId == newContentId)
		return;
	m_contentId = newContentId;
	emit contentIdChanged();
}
