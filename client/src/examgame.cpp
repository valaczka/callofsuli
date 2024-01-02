#include "examgame.h"
#include "BitMatrix.h"
#include "Logger.h"
#include "Matrix.h"
#include "MultiFormatWriter.h"
#include "qbuffer.h"
#include "QPageSize"
#include "application.h"
#include "teachergroup.h"
#include "utils_.h"
#include <QPdfWriter>
#include <QRandomGenerator>
#include "stb_image_write.h"


const QString ExamGame::m_optionLetters = QStringLiteral("ABCDEF?????????????????????????????????????????????");


/**
 * @brief ExamGame::ExamGame
 * @param mode
 * @param missionlevel
 * @param client
 */

ExamGame::ExamGame(const Exam::Mode &mode, Client *client)
	: AbstractGame(GameMap::Exam, client)
	, m_mode(mode)
{
	LOG_CTRACE("game") << "Exam game constructed" << this;
}


/**
 * @brief ExamGame::~ExamGame
 */

ExamGame::~ExamGame()
{
	LOG_CTRACE("game") << "Exam game destroyed" << this;
}


/**
 * @brief ExamGame::createQuestionsFromMissionLevel
 * @param missionLevel
 * @return
 */

QVector<Question> ExamGame::createQuestions(GameMapMissionLevel *missionLevel)
{
	LOG_CDEBUG("game") << "Create questions";

	QVector<Question> list;

	if (!missionLevel) {
		LOG_CWARNING("game") << "Missing game map, don't created any question";
		return list;
	}

	foreach (GameMapChapter *chapter, missionLevel->chapters()) {
		foreach (GameMapObjective *objective, chapter->objectives()) {
			int n = (objective->storageId() > 0 ? objective->storageCount() : 1);

			for (int i=0; i<n; ++i)
				list.append(Question(objective));

		}
	}

	LOG_CDEBUG("game") << "Created " << list.size() << " questions";

	return list;
}



/**
 * @brief ExamGame::generatePaperQuestions
 * @param missionLevel
 * @return
 */

QJsonArray ExamGame::generatePaperQuestions(GameMapMissionLevel *missionLevel)
{
	LOG_CDEBUG("game") << "Generate paper questions";

	if (!missionLevel) {
		LOG_CWARNING("game") << "Missing game map, don't created any question";
		return {};
	}

	QVector<Question> list = createQuestions(missionLevel);

	QVector<Question> easyList;
	QVector<Question> complexList;

	static const QStringList easyModules = {
		QStringLiteral("truefalse"),
		QStringLiteral("simplechoice"),
		QStringLiteral("multichoice"),
		QStringLiteral("pair"),
		QStringLiteral("order"),
		QStringLiteral("fillout"),
	};


	QJsonArray retList;


	for (auto it=list.constBegin(); it != list.constEnd(); ++it) {
		if (easyModules.contains(it->module()))
			easyList.append(*it);
		else
			complexList.append(*it);
	}

	while (!easyList.isEmpty()) {
		const Question &q = easyList.takeAt(QRandomGenerator::global()->bounded(easyList.size()));
		const QVariantMap &data = q.generate();

		QJsonObject json = QJsonObject::fromVariantMap(data);
		json[QStringLiteral("uuid")] = q.uuid();
		json[QStringLiteral("module")] = q.module();
		retList.append(json);
	}

	bool isFirst = true;

	while (!complexList.isEmpty()) {
		const Question &q = complexList.takeAt(QRandomGenerator::global()->bounded(complexList.size()));
		const QVariantMap &data = q.generate();

		QJsonObject json = QJsonObject::fromVariantMap(data);
		json[QStringLiteral("uuid")] = q.uuid();
		json[QStringLiteral("module")] = q.module();
		if (isFirst) {
			json[QStringLiteral("separator")] = true;
			isFirst = false;
		}
		retList.append(json);
	}

	return retList;
}




/**
 * @brief ExamGame::generatePdf
 * @param list
 */

void ExamGame::generatePdf(const QJsonArray &list, const PdfConfig &pdfConfig, TeacherGroup *group)
{
	LOG_CDEBUG("game") << "Generate paper exam pdf";

	m_worker.execInThread([this, list, pdfConfig, group]{
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

		LOG_CERROR("game") << layout.fullRectPoints();

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

		/*QImage imageR = QImage::fromData(Utils::fileContentRead(QStringLiteral(":/internal/exam/bgRight.png")));
		document.addResource(QTextDocument::ImageResource, QUrl(QStringLiteral("imgdata://bgRight.png")), QVariant(imageR));*/

		QString html;

		html.append(QStringLiteral("<html><body>"));

		int count = 0;

		for (const QJsonValue &v : list) {
			const QJsonObject &o = v.toObject();
			QString username = o.value(QStringLiteral("username")).toString();
			const int &id = o.value(QStringLiteral("id")).toInt();
			const QJsonArray &qList = o.value(QStringLiteral("data")).toArray();

			if (group) {
				if (User *u = OlmLoader::find<User>(group->memberList(), "username", username); u)
					username = u->fullName();
			}

			html += QStringLiteral("<table width=\"100%\"");
			if (count>0)
				html += QStringLiteral(" style=\"page-break-before: always;\"");

			html += QStringLiteral("><tr><td><img src=\"imgdata://bg.png\"></td><td>");

			html += pdfTitle(pdfConfig, username, id, count==0, &document);
			html += pdfSheet(count==0, layout.paintRectPoints().width()-80, &document);
			html += pdfQuestion(qList);

			html += QStringLiteral("</td><td><img src=\"imgdata://bg.png\"></td></tr></table>");


			// Check page

			++count;

			QString tmp = html + QStringLiteral("</body></html>");
			document.setHtml(tmp);

			int reqPages = pdfConfig.pagePerUser*count - document.pageCount();

			if (reqPages > 0)
				html += QStringLiteral("<p style=\"page-break-before: always;\">&nbsp;</p>").repeated(reqPages);
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
 * @brief ExamGame::scanImageDir
 * @param path
 */

void ExamGame::scanImageDir(const QString &path)
{
	if (!m_scanFiles.isEmpty()) {
		LOG_CERROR("game") << "Scanning in progress";
		return;
	}

	QDirIterator it(path, {
						QStringLiteral("*.png"),
						QStringLiteral("*.PNG"),
						QStringLiteral("*.jpg"),
						QStringLiteral("*.JPG"),
						QStringLiteral("*.jpeg"),
						QStringLiteral("*.JPEG"),
					}, QDir::Files);

	while (it.hasNext()) {
		ScanFile s;
		s.path = it.next();
		m_scanFiles.append(s);
	}

	scanImages();
}

/**
 * @brief ExamGame::generateQuestions
 */

ExamGame::PaperContent ExamGame::generateQuestions(const QVector<Question> &list)
{
	LOG_CERROR("client") << "Deprecated function:" << __PRETTY_FUNCTION__;

	PaperContent content;

	QVector<Question> easyList;
	QVector<Question> complexList;

	static const QStringList easyModules = {
		QStringLiteral("truefalse"),
		QStringLiteral("simplechoice"),
		QStringLiteral("multichoice"),
		QStringLiteral("pair"),
	};


	for (auto it=list.constBegin(); it != list.constEnd(); ++it) {
		if (easyModules.contains(it->module()))
			easyList.append(*it);
		else
			complexList.append(*it);
	}


	bool enumerateStarted = false;
	bool requireRestart = true;

	while (!easyList.isEmpty()) {
		const Question &q = easyList.takeAt(QRandomGenerator::global()->bounded(easyList.size()));

		const QVariantMap &data = q.generate();

		QStringList options;
		QVariantList correct;

		if (q.module() == QStringLiteral("simplechoice")) {
			QString q;
			if (!enumerateStarted) {
				q = requireRestart ? QStringLiteral("\\begin{enumerate}[start=1]\n") : QStringLiteral("\\begin{enumerate}\n");
				enumerateStarted = true;
				requireRestart = false;
			}

			q += QStringLiteral("\\item ")+data.value(QStringLiteral("question")).toString()+QStringLiteral("\n");
			content.questions += q;
			content.answers += q;

			options = data.value(QStringLiteral("options")).toStringList();
			correct << data.value(QStringLiteral("answer")).toInt();

		} else if (q.module() == QStringLiteral("multichoice")) {
			QString q;
			if (!enumerateStarted) {
				q = requireRestart ? QStringLiteral("\\begin{enumerate}[start=1]\n") : QStringLiteral("\\begin{enumerate}\n");
				enumerateStarted = true;
				requireRestart = false;
			}

			q += QStringLiteral("\\item ")+data.value(QStringLiteral("question")).toString()+QStringLiteral("\n");
			content.questions += q;
			content.answers += q;

			options = data.value(QStringLiteral("options")).toStringList();
			correct = data.value(QStringLiteral("answer")).toList();

		} else if (q.module() == QStringLiteral("pair")) {
			QString q;
			if (enumerateStarted) {
				q = QStringLiteral("\\end{enumerate}\n\n");
				enumerateStarted = false;
			}
			q += QStringLiteral("\\textbf{")+data.value(QStringLiteral("question")).toString()+QStringLiteral("}\n");
			content.questions += q;
			content.answers += q;

			options = data.value(QStringLiteral("options")).toStringList();
		}

		static const QString optStart = QStringLiteral("\\begin{inlinelist}\n");
		content.questions += optStart;
		content.answers += optStart;

		for (int i=0; i<options.size(); ++i) {
			content.questions += QStringLiteral(" \\item ") + options.at(i) + QStringLiteral("\n");

			if (correct.contains(i))
				content.answers += QStringLiteral(" \\item \\textbf{") + options.at(i) + QStringLiteral("}");
			else
				content.answers += QStringLiteral(" \\item ") + options.at(i);

			content.answers += QStringLiteral("\n");

		}

		static const QString optEnd = QStringLiteral("\\end{inlinelist}\n\n");
		content.questions += optEnd;
		content.answers += optEnd;


		if (q.module() == QStringLiteral("pair")) {
			QString q;
			if (requireRestart) {
				q = QStringLiteral("\n\\begin{enumerate}[labelindent=3em, topsep=0cm, start=1]\n");
				requireRestart = false;
			} else
				q = QStringLiteral("\n\\begin{enumerate}[labelindent=3em, topsep=0cm]\n");

			content.questions += q;
			content.answers += q;


			const QStringList &qList = data.value(QStringLiteral("list")).toStringList();
			const QStringList &aList = data.value(QStringLiteral("answer")).toMap().value(QStringLiteral("list")).toStringList();

			for (int i=0; i<qList.size(); ++i) {
				content.questions += QStringLiteral("\\item ")+qList.at(i)+QStringLiteral(" -- \\rule{3em}{0.5pt}\n");
				content.answers += QStringLiteral("\\item ")+qList.at(i)+QStringLiteral(" -- \\textbf{");

				if (aList.size() && i < aList.size()) {
					const QString &ans = aList.at(i);
					int idx = options.indexOf(ans);

					static const QString &optValues = QStringLiteral("ABCDEFGHIJKLMNOPQRSTUVWXYZ");

					if (idx > -1)
						content.answers += QStringLiteral("(")+((idx < optValues.size()) ? optValues.at(idx) : QStringLiteral("?"))+QStringLiteral(") ");

					content.answers += ans + QStringLiteral("}\n");
				}

			}

			static const QString &q2 = QStringLiteral("\\end{enumerate}\n\n");
			content.questions += q2;
			content.answers += q2;
			enumerateStarted = false;

		}
	}

	if (enumerateStarted) {
		static const QString &q = QStringLiteral("\\end{enumerate}\n\n");
		content.questions += q;
		content.answers += q;
	}


	if (!content.questions.isEmpty() && !complexList.isEmpty()) {
		content.questions += QStringLiteral("\n\\rule{\\linewidth}{0.5pt}\n\n");
	}


	content.questions += QStringLiteral("\\begin{spacing}{2.0}\n");

	while (!complexList.isEmpty()) {
		const Question &q = complexList.takeAt(QRandomGenerator::global()->bounded(complexList.size()));

		const QVariantMap &data = q.generate();

		if (!content.questions.isEmpty()) {
			content.questions += QStringLiteral("\n");
		}

		content.questions += data.value(QStringLiteral("question")).toString()
							 + QStringLiteral(" -- \\rule{10em}{0.5pt}\n\n");

	}

	content.questions += QStringLiteral("\\end{spacing}\n");


	return content;
}


/**
 * @brief ExamGame::mode
 * @return
 */

const Exam::Mode &ExamGame::mode() const
{
	return m_mode;
}

void ExamGame::setMode(const Exam::Mode &newMode)
{
	if (m_mode == newMode)
		return;
	m_mode = newMode;
	emit modeChanged();
}


/**
 * @brief ExamGame::loadPage
 * @return
 */

QQuickItem *ExamGame::loadPage()
{
	if (m_mode == Exam::ExamPaper)
		return nullptr;
	else {
		LOG_CERROR("game") << "Missing implementation:" << __PRETTY_FUNCTION__;
		return nullptr;
	}
}


/**
 * @brief ExamGame::connectGameQuestion
 */

void ExamGame::connectGameQuestion()
{
	if (m_mode == Exam::ExamPaper)
		return;
	else {
		LOG_CERROR("game") << "Missing implementation:" << __PRETTY_FUNCTION__;
		return;
	}
}



/**
 * @brief ExamGame::gameFinishEvent
 * @return
 */

bool ExamGame::gameFinishEvent()
{
	if (m_mode == Exam::ExamPaper)
		return true;
	else {
		LOG_CERROR("game") << "Missing implementation:" << __PRETTY_FUNCTION__;
		return false;
	}
}


/**
 * @brief ExamGame::pdfTitle
 * @param title
 * @param username
 * @param examId
 * @param contentId
 * @param document
 * @return
 */

QString ExamGame::pdfTitle(const PdfConfig &pdfConfig, const QString &username, const int &contentId,
						   const bool &isFirstPage, QTextDocument *document)
{
	Q_ASSERT(document);

	const QString id = QStringLiteral("Call of Suli Exam %1/%2/%3/%4")
					   .arg(Application::versionMajor()).arg(Application::versionMinor())
					   .arg(pdfConfig.examId).arg(contentId);

	//QImage barcode = QZXing::encodeData(id, QZXing::EncoderFormat_QR_CODE, QSize(120,120));

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

	QString html = QStringLiteral("<table width=\"100%\" style=\"margin-left: 0px; margin-right: 30px;\">");

	/*QString html = isFirstPage ? QStringLiteral("<table width=\"100%\" style=\"margin-left: 35px; margin-right: 30px;\">")
							   : QStringLiteral("<table width=\"100%\" style=\"margin-left: 35px; margin-right: 30px; "
												"page-break-before: always;\">");*/

	html += QStringLiteral("<tr><td valign=middle><img height=60 src=\"%1\"></td>"
						   "<td width=\"100%\" valign=middle style=\"padding-left: 10px;\">"
						   "<p><span style=\"font-size: large;\"><b>%2</b></span><br/>%3<br/><small>%4</small></p>"
						   "</td></tr></table>\n\n")
			.arg(imgName, username, pdfConfig.title, pdfConfig.subject)
			;

	return html;
}



/**
 * @brief ExamGame::pdfSheet
 * @param document
 * @return
 */

QString ExamGame::pdfSheet(const bool &addResource, const int &width, QTextDocument *document)
{
	Q_ASSERT(document);

	static const QString imgName = QStringLiteral("imgdata://sheet.svg");

	if (addResource) {
		QImage image = QImage::fromData(Utils::fileContentRead(QStringLiteral(":/internal/exam/sheet50.png")));
		document->addResource(QTextDocument::ImageResource, QUrl(imgName), QVariant(image));
	}

	return QStringLiteral("<center><img src=\"%1\" width=\"%2\"></center>").arg(imgName).arg(width);
}



/**
 * @brief ExamGame::pdfQuestion
 * @param list
 * @return
 */

QString ExamGame::pdfQuestion(const QJsonArray &list)
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
 * @brief ExamGame::scanImages
 */

void ExamGame::scanImages()
{
	LOG_CTRACE("game") << "Scan images";

	m_worker.execInThread([this](){
		QMutexLocker locker(&m_mutex);

		for (auto it=m_scanFiles.begin(); it != m_scanFiles.end(); ++it) {
			const auto &data = Utils::fileContent(it->path);

			if (!data) {
				it->state = ScanFileError;
				continue;
			}

			QImage img = QImage::fromData(data.value());

			if (img.isNull()) {
				LOG_CERROR("game") << "Invalid image:" << qPrintable(it->path);
				it->state = ScanFileError;
				continue;
			}

			ExamSBarcodeDecoder *decoder = new ExamSBarcodeDecoder;
			decoder->setPath(it->path);

			decoder->setResolution(img.width(), img.height());
			connect(decoder, &SBarcodeDecoder::isDecodingChanged, this, [this, decoder](bool d) {
				if (!d) {
					LOG_CTRACE("game") << "Decoding finished:" << decoder->path();
					scanUpdateQR(decoder->path(), decoder->captured());
					decoder->deleteLater();
				}
			});
			decoder->process(img, ZXing::BarcodeFormat::QRCode);
		}
	});
}



/**
 * @brief ExamGame::scanHasPendingQR
 * @return
 */

bool ExamGame::scanHasPendingQR()
{
	QMutexLocker locker(&m_mutex);

	for (const ScanFile &s : m_scanFiles) {
		if (s.state == ScanFileLoaded)
			return true;
	}

	return false;
}


/**
 * @brief ExamGame::scanUpdateQR
 * @param path
 * @param state
 */

void ExamGame::scanUpdateQR(const QString &path, const QString &qr)
{
	m_worker.execInThread([this, path, qr](){
		QMutexLocker locker(&m_mutex);

		for (auto it=m_scanFiles.begin(); it != m_scanFiles.end(); ++it) {
			if (it->path != path)
				continue;

			static const QString prefix = QStringLiteral("Call of Suli Exam ");

			if (!qr.startsWith(prefix)) {
				LOG_CWARNING("game") << "Invalid QR code:" << qr << "file:" << path;
				it->state = ScanFileInvalid;
				continue;
			}

			QString code = qr;
			code.remove(0, prefix.size());
			QStringList fields = code.split('/');

			if (fields.size() != 4) {
				LOG_CWARNING("game") << "Invalid QR code:" << qr << "file:" << path;
				it->state = ScanFileInvalid;
				continue;
			}

			const int vMajor = fields.at(0).toInt();
			const int vMinor = fields.at(1).toInt();

			if (Utils::versionCode(vMajor, vMinor) != Utils::versionCode()) {
				LOG_CWARNING("game") << "Wrong version:" << qr << "file:" << path;
				it->state = ScanFileInvalid;
				continue;
			}

			it->state = ScanFileReadQR;
			it->examId = fields.at(2).toInt();
			it->contentId = fields.at(3).toInt();

			LOG_CTRACE("game") << "Loaded QR image:" << path;
		}

		if (scanHasPendingQR())
			return;

		emit scanQRfinished();

		scanPreapareOMR();
	});
}


/**
 * @brief ExamGame::scanPreapareOMR
 */

void ExamGame::scanPreapareOMR()
{
	m_worker.execInThread([this](){
		QMutexLocker locker(&m_mutex);

		m_scanTempDir.reset(new QTemporaryDir);
		m_scanTempDir->setAutoRemove(true);

		LOG_CDEBUG("game") << "Prepare OMR to dir:" << qPrintable(m_scanTempDir->path());

		const QString dirInput = m_scanTempDir->filePath(QStringLiteral("input"));

		if (!QFile::exists(dirInput)) {
			QDir dir;
			if (!dir.mkpath(dirInput)) {
				LOG_CERROR("game") << "Directory create error:" << qPrintable(dirInput);
				return;
			}
		}

		bool omr = false;

		for (auto it=m_scanFiles.begin(); it != m_scanFiles.end(); ++it) {
			if (it->state == ScanFileReadQR) {
				QFileInfo f(it->path);
				QString newName = dirInput+QStringLiteral("/id_%1_%2.%3").arg(it->examId).arg(it->contentId).arg(f.suffix());
				if (QFile::copy(it->path, newName)) {
					LOG_CTRACE("game") << "Copy:" << qPrintable(it->path) << "->" << qPrintable(newName);
					it->state = ScanFileReadingOMR;
					omr = true;
				} else {
					LOG_CERROR("game") << "Copy error:" << qPrintable(it->path) << "->" << qPrintable(newName);
					it->state = ScanFileError;
				}
			}
		}

		if (!omr) {
			LOG_CWARNING("game") << "Scannable image not found";
			m_scanTempDir.reset();
			return;
		}

		runOMR();
	});
}



/**
 * @brief ExamGame::runOMR
 */

void ExamGame::runOMR()
{
	m_worker.execInThread([this](){
		if (m_omrProcess) {
			LOG_CERROR("game") << "OMR in progress";
			return;
		}

		if (!m_scanTempDir) {
			LOG_CERROR("game") << "Missing image directory";
			return;
		}

		if (!QFile::copy(QStringLiteral(":/internal/exam/template.json"), m_scanTempDir->filePath(QStringLiteral("input/template.json")))) {
			LOG_CERROR("game") << "Template copy error";
			return;
		}

		if (!QFile::copy(QStringLiteral(":/internal/exam/marker.png"), m_scanTempDir->filePath(QStringLiteral("input/marker.png")))) {
			LOG_CERROR("game") << "Marker copy error";
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
			LOG_CERROR("game") << "OMR main.py not found";
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

		connect(m_omrProcess.get(), &QProcess::finished, this, &ExamGame::onOmrFinished);

		LOG_CINFO("game") << "Start OMR";

		m_omrProcess->start();
	});
}





/**
 * @brief ExamGame::onOmrFinished
 * @param exitCode
 * @param exitStatus
 */
void ExamGame::onOmrFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
	if (m_omrProcess) {
		auto p = m_omrProcess.release();
		p->deleteLater();
	}

	if (!m_scanTempDir) {
		LOG_CERROR("game") << "Missing image directory";
		return;
	}

	if (exitStatus != QProcess::NormalExit || exitCode != 0) {
		LOG_CERROR("game") << "OMR error";
		return;
	}

	LOG_CINFO("game") << "OMR finished successful";

	emit scanOMRfinished();

	const QString &fResult = m_scanTempDir->filePath(QStringLiteral("output/Results/Results.csv"));
	const QString &fError = m_scanTempDir->filePath(QStringLiteral("output/Manual/ErrorFiles.csv"));

	const QByteArray &csvResult = Utils::fileContentRead(fResult);
	const QByteArray &errorResult = Utils::fileContentRead(fError);

	m_scanFiles.clear();

	LOG_CINFO("game") << csvResult.constData();
	LOG_CWARNING("game") << errorResult.constData();


}



