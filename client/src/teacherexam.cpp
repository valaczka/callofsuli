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

void TeacherExam::createPdf(const QJsonArray &list, const PdfConfig &pdfConfig)
{
	LOG_CDEBUG("client") << "Generate paper exam pdf";

	m_worker.execInThread([this, list, pdfConfig]{
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

		QImage image = QImage::fromData(Utils::fileContentRead(QStringLiteral(":/internal/exam/bg.png")));
		document.addResource(QTextDocument::ImageResource, QUrl(QStringLiteral("imgdata://bg.png")), QVariant(image));

		QString html;

		html.append(QStringLiteral("<html><body>"));

		int count = 0;

		for (const QJsonValue &v : list) {
			const QJsonObject &o = v.toObject();
			QString username = o.value(QStringLiteral("username")).toString();
			const int &id = o.value(QStringLiteral("id")).toInt();
			const QJsonArray &qList = o.value(QStringLiteral("data")).toArray();

			if (m_teacherGroup) {
				if (User *u = OlmLoader::find<User>(m_teacherGroup->memberList(), "username", username); u)
					username = u->fullName();
			}

			html += QStringLiteral("<table width=\"100%\"");
			if (count>0)
				html += QStringLiteral(" style=\"page-break-before: always;\"");

			html += QStringLiteral("><tr><td><img width=30 src=\"imgdata://bg.png\"></td><td>");

			html += pdfTitle(pdfConfig, username, id, &document);
			html += pdfSheet(count==0, layout.paintRectPoints().width()-80, &document);
			html += pdfQuestion(qList);

			html += QStringLiteral("</td><td><img width=30 src=\"imgdata://bg.png\"></td></tr></table>");

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

void TeacherExam::createPdf(const QJsonArray &list, const QVariantMap &pdfConfig)
{
	PdfConfig c;

	if (pdfConfig.contains(QStringLiteral("examId")))
		c.examId = pdfConfig.value(QStringLiteral("examId")).toInt();

	if (pdfConfig.contains(QStringLiteral("title")))
		c.title = pdfConfig.value(QStringLiteral("title")).toString();

	if (pdfConfig.contains(QStringLiteral("subject")))
		c.subject = pdfConfig.value(QStringLiteral("subject")).toString();
	else if (m_teacherGroup)
		c.subject = m_teacherGroup->fullName();

	if (pdfConfig.contains(QStringLiteral("fontSize")))
		c.fontSize = pdfConfig.value(QStringLiteral("fontSize")).toInt();

	createPdf(list, c);
}



/**
 * @brief TeacherExam::scanImageDir
 * @param path
 */

void TeacherExam::scanImageDir(const QString &path)
{
	if (m_scanState == ScanRunning) {
		LOG_CERROR("client") << "Scanning in progress";
		return;
	}

	m_scanData->clear();
	m_scanTempDir.reset();

	QDirIterator it(path, {
						QStringLiteral("*.png"),
						QStringLiteral("*.PNG"),
						QStringLiteral("*.jpg"),
						QStringLiteral("*.JPG"),
						QStringLiteral("*.jpeg"),
						QStringLiteral("*.JPEG"),
					}, QDir::Files);


	while (it.hasNext()) {
		ExamScanData *s = new ExamScanData;
		s->setPath(it.next());
		m_scanData->append(s);
	}

	setScanState(ScanRunning);

	scanImages();
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
		//const int &factor = qFloor(obj.value(QStringLiteral("xpFactor")).toDouble());

		if (obj.value(QStringLiteral("separator")).toBool()) {
			html += QStringLiteral("<p align=center style=\"margin-top: 20px; margin-bottom: 20px;\">===============</p>");
		}

		html += QStringLiteral("<p style=\"margin-top: 6px;\" "
							   "align=justify><span style=\"font-weight: 600;\">");

		if (!nonNumberedModules.contains(module))
			html += QString::number(num++).append(QStringLiteral(". "));

		html += question;
		html += QStringLiteral("</span>");
		//html += QObject::tr("[%1p]").arg(factor);

		if (module == QStringLiteral("simplechoice") || module == QStringLiteral("multichoice")) {
			const QJsonArray &options = obj.value(QStringLiteral("options")).toArray();
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
					html += QStringLiteral(" <b>____(")+QString::number(num++)+QStringLiteral(".)</b>");
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
					QJsonArray r = getResult(o.value(QStringLiteral("data")).toArray(), s->result());
					LOG_CDEBUG("client") << "Update server answer" << s->contentId() << s->username() << r;
					s->setServerAnswer(r);
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



QJsonArray TeacherExam::getResult(const QJsonArray &qList, const QJsonObject &answer) const
{
	int num = 1;

	/*static const QStringList nonNumberedModules = {
		QStringLiteral("pair"),
		QStringLiteral("fillout"),
		QStringLiteral("order"),
	};*/

	QJsonArray ret;

	for (const QJsonValue &v : qList) {
		const QJsonObject &obj = v.toObject();
		const QString &module = obj.value(QStringLiteral("module")).toString();
		const QString &question = obj.value(QStringLiteral("question")).toString();
		//const int &factor = qFloor(obj.value(QStringLiteral("xpFactor")).toDouble());

		if (module == QStringLiteral("simplechoice")) {
			ret.append(letterToInt(answer, num++));
		} else if (module == QStringLiteral("multichoice")) {
			ret.append(letterToArray(answer, num++));
		} else if (module == QStringLiteral("order")) {
			const QJsonArray &list = obj.value(QStringLiteral("list")).toArray();

			QJsonArray l;

			for (int i=0; i<list.size(); ++i)
				l.append(letterToInt(answer, num++));

			ret.append(l);
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

			ret.append(l);
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

	return ret;
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
