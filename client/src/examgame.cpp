#include "examgame.h"
#include "BitMatrix.h"
#include "Logger.h"
#include "Matrix.h"
#include "MultiFormatWriter.h"
#include "stb_image_write.h"
#include "qbuffer.h"
#include "QPageSize"
#include "application.h"
#include "utils_.h"
#include <QPdfWriter>
#include <QRandomGenerator>

#ifndef ENABLE_ENCODER_GENERIC
#	define ENABLE_ENCODER_GENERIC
#endif

#include <QZXing.h>



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


	while (!complexList.isEmpty()) {
		const Question &q = complexList.takeAt(QRandomGenerator::global()->bounded(complexList.size()));
		const QVariantMap &data = q.generate();

		QJsonObject json = QJsonObject::fromVariantMap(data);
		json[QStringLiteral("uuid")] = q.uuid();
		json[QStringLiteral("module")] = q.module();
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

	QTextDocument document;

	QFont font(QStringLiteral("Rajdhani"), pdfConfig.fontSize);

	document.setPageSize(QPageSize::sizePoints(QPageSize::A4));
	document.setDefaultFont(font);

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

		html += pdfTitle(pdfConfig.title, username, pdfConfig.examId, id, count==0, &document);
		html += pdfSheet(count==0, &document);

		html += "<p align=justify>"+QJsonDocument(qList).toJson()+"</p>";


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


	QByteArray content;
	QBuffer buffer(&content);
	buffer.open(QIODevice::WriteOnly);

	QPdfWriter pdf(&buffer);
	pdf.setCreator(QStringLiteral("Call of Suli - v")+Application::version());
	pdf.setTitle(pdfConfig.title);

	QPageLayout layout = pdf.pageLayout();
	layout.setUnits(QPageLayout::Millimeter);
	layout.setPageSize(QPageSize::A4);
	layout.setMargins(QMarginsF(10, 10, 10, 10));
	//layout.setMinimumMargins(QMarginsF(5, 5, 5, 5));
	//layout.setMode(QPageLayout::FullPageMode);
	pdf.setPageLayout(layout);


	document.print(&pdf);

	buffer.close();

	QFile f("/tmp/out.pdf");
	f.open(QIODevice::WriteOnly);
	f.write(content);
	f.close();

	/*QString txt;

	txt.append(QStringLiteral("<html><body>\n"));

	txt.append(QStringLiteral("<h1>"))
			.append(m_title)
			.append(QStringLiteral("</h1>"));

	txt.append(QStringLiteral("<h4>Jelenlegi jogviszony (piarista): <i>%1 év %2 nap</i><br/>")
			   .arg(m_calculation.value(QStringLiteral("jobYears"), 0).toInt())
			   .arg(m_calculation.value(QStringLiteral("jobDays"), 0).toInt())
			   );

	txt.append(QStringLiteral("Gyakorlati idő: <i>%1 év %2 nap</i><br/>")
			   .arg(m_calculation.value(QStringLiteral("practiceYears"), 0).toInt())
			   .arg(m_calculation.value(QStringLiteral("practiceDays"), 0).toInt())
			   );

	txt.append(QStringLiteral("Jubileumi jutalom: <i>%1 év %2 nap</i></h4>")
			   .arg(m_calculation.value(QStringLiteral("prestigeYears"), 0).toInt())
			   .arg(m_calculation.value(QStringLiteral("prestigeDays"), 0).toInt())
			   );

	if (const int nextY = m_calculation.value(QStringLiteral("nextPrestigeYears"), 0).toInt(); nextY > 0) {
		txt.append(QStringLiteral("<h4>Következő jubileumi jutalom időpontja: <i>%1</i> (%2 év)</h4>")
				   .arg(QLocale().toString(m_calculation.value(QStringLiteral("nextPrestige")).toDate(), QStringLiteral("yyyy. MMMM d.")))
				   .arg(m_calculation.value(QStringLiteral("nextPrestigeYears"), 0).toInt())
				   );
	}

	txt.append(QStringLiteral("<h3>&nbsp;</h3>"));

	const QVariantList &list = m_model->storage();

	for (const auto &v : list) {
		const QVariantMap &map = v.toMap();

		txt.append(QStringLiteral("<h3>"));
		txt.append(map.value(QStringLiteral("name")).toString())
				.append(QStringLiteral(" ("))
				.append(QLocale().toString(map.value(QStringLiteral("start")).toDate(), QStringLiteral("yyyy. MMMM d.")))
				.append(QStringLiteral(" – "));

		if (map.contains(QStringLiteral("end")))
			txt.append(QLocale().toString(map.value(QStringLiteral("end")).toDate(), QStringLiteral("yyyy. MMMM d.")));

		txt.append(QStringLiteral(")</h3>"));

		txt.append(QStringLiteral("<p>Foglalkoztatási jogviszony: <b>%1</b>, ").arg(map.value(QStringLiteral("type")).toString()))
				.append(QStringLiteral("munkaidő: <b>%1 óra</b>, ").arg(map.value(QStringLiteral("hour")).toInt()))
				.append(QStringLiteral("heti munkaóra: <b>%1 óra</b><br/>").arg(map.value(QStringLiteral("value")).toInt()));

		txt.append(QStringLiteral("Munkáltató vagy megbízó:</p><p style=\"margin-left: 25px;\"><small>"));
		txt.append(map.value(QStringLiteral("master")).toString().replace(QStringLiteral("\n"), QStringLiteral("<br/>")));
		txt.append(QStringLiteral("</small></p>"));

		txt.append(QStringLiteral("<p>Számított jelenlegi jogviszony (piarista): <b>%1 év %2 nap</b><br/>")
				   .arg(map.value(QStringLiteral("jobYears"), 0).toInt())
				   .arg(map.value(QStringLiteral("jobDays"), 0).toInt())
				   );

		txt.append(QStringLiteral("Számított gyakorlati idő: <b>%1 év %2 nap</b><br/>")
				   .arg(map.value(QStringLiteral("practiceYears"), 0).toInt())
				   .arg(map.value(QStringLiteral("practiceDays"), 0).toInt())
				   );

		txt.append(QStringLiteral("Számított jubileumi jutalom: <b>%1 év %2 nap</b></p>")
				   .arg(map.value(QStringLiteral("prestigeYears"), 0).toInt())
				   .arg(map.value(QStringLiteral("prestigeDays"), 0).toInt())
				   );

	}

	txt.append(QStringLiteral("<p style=\"margin-top: 20px;\"><i>A munkáltató a mai napon a fenti, szakmai gyakorlati időre vonatkozó jogviszonyokat és köznevelési jubileumi jutalomra jogosító időket tartja nyilván.</i></p>"
							  "<p style=\"margin-top: 20px;\">Kelt:</p>"
							  "<p style=\"margin-top: 20px; margin-bottom: 80px;\">Aláírás (a munkáltató képviseletében):</p><p>&nbsp;</p>"));

	txt.append(QStringLiteral("<table width=\"100%\"><tr><td style=\"border-top: 1px solid #cccccc; font-size: 2pt;\">&nbsp;</td></tr></table>"));

	txt.append(QStringLiteral("<table width=\"100%\"><tr><td valign=middle><img height=20 src=\"imgdata://piar.png\"></td>"
							  "<td width=\"100%\" valign=middle style=\"padding-left: 10px;\"><p style=\"font-size: 5pt;\">Gyarkolati idő kalkulátor v%2.%3<br/>"
							  "Készült: %1</p>"
							  "</td></tr></table>\n\n")
			   .arg(QLocale().toString(QDateTime::currentDateTime(), QStringLiteral("yyyy. MMMM d. HH:mm:ss")))
			   .arg(Application::versionMajor())
			   .arg(Application::versionMinor())
			   );

	txt.append(QStringLiteral("</body></html>"));*/


}





/**
 * @brief ExamGame::generateQuestions
 */

ExamGame::PaperContent ExamGame::generateQuestions(const QVector<Question> &list)
{
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

QString ExamGame::pdfTitle(const QString &title, const QString &username,
						   const int &examId, const int &contentId,
						   const bool &isFirstPage, QTextDocument *document)
{
	Q_ASSERT(document);

	const QString id = QStringLiteral("%1/%2").arg(examId).arg(contentId);

	ZXing::BarcodeFormat format = ZXing::BarcodeFormat::DataMatrix;
	ZXing::MultiFormatWriter writer = ZXing::MultiFormatWriter(format).setMargin(0);

	ZXing::Matrix<uint8_t> bitmap = ZXing::ToMatrix<uint8_t>(writer.encode(id.toStdWString(), 200, 200));

	QImage image;

	stbi_write_png_to_func([](void *context, void *data, int size){
		QImage *img = (QImage*) context;
		*img = QImage::fromData((const unsigned char *) data, size);
	}, &image, bitmap.width(), bitmap.height(), 1, bitmap.data(), 0);

	const QString imgName = QStringLiteral("imgdata://id%1.png").arg(contentId);

	document->addResource(QTextDocument::ImageResource, QUrl(imgName), QVariant(image));

	QString html = isFirstPage ? QStringLiteral("<table width=\"100%\">")
							   : QStringLiteral("<table width=\"100%\" style=\"page-break-before: always;\">");

	html += QStringLiteral("<tr><td valign=middle><img height=20 src=\"%1\"></td>"
						   "<td width=\"100%\" valign=middle style=\"padding-left: 10px;\">"
						   "<p><span style=\"font-size: large;\"><b>%2</b></span><br/><small>%3</small></p>"
						   "</td></tr></table>\n\n")
			.arg(imgName, username, title)
			;

	return html;
}



/**
 * @brief ExamGame::pdfSheet
 * @param document
 * @return
 */

QString ExamGame::pdfSheet(const bool &addResource, QTextDocument *document)
{
	Q_ASSERT(document);

	static const QString imgName = QStringLiteral("imgdata://sheet.svg");

	if (addResource) {
		QImage image = QImage::fromData(Utils::fileContentRead(QStringLiteral("/tmp/4794.png")));
		document->addResource(QTextDocument::ImageResource, QUrl(imgName), QVariant(image));
	}

	return QStringLiteral("<center><img src=\"%1\" height=500></center>").arg(imgName);
}


