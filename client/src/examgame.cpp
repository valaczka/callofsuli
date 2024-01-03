#include "examgame.h"
#include "Logger.h"
#include <QRandomGenerator>


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

