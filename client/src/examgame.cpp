#include "examgame.h"
#include "Logger.h"
#include <QRandomGenerator>



/**
 * @brief ExamGame::ExamGame
 * @param mode
 * @param missionlevel
 * @param client
 */

ExamGame::ExamGame(const Mode &mode, Client *client)
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

	static const QString fQuestionPrefix = QStringLiteral("%1. ");
	static const QString fQuestionSuffix = QLatin1String("");
	static const QString fQuestionSpecialPrefix = QStringLiteral("**");
	static const QString fQuestionSpecialSuffix = QStringLiteral("**  \n");
	static const QString fOptionPrefix = QStringLiteral("(%1) ");
	static const QString fOptionValues = QStringLiteral("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
	static const QString fOptionSeparator = QStringLiteral(", ");
	static const QString fOptionSuffix = QLatin1String("");
	static const QString fOptionListStart = QStringLiteral(" *");
	static const QString fOptionListEnd = QStringLiteral("*");
	static const QString fOptionCorrectPrefix = QStringLiteral("**");
	static const QString fOptionCorrectSuffix = QStringLiteral("**");

	for (auto it=list.constBegin(); it != list.constEnd(); ++it) {
		if (easyModules.contains(it->module()))
			easyList.append(*it);
		else
			complexList.append(*it);
	}


	int qNum = 1;


	while (!easyList.isEmpty()) {
		const Question &q = easyList.takeAt(QRandomGenerator::global()->bounded(easyList.size()));

		const QVariantMap &data = q.generate();

		if (!content.questions.isEmpty()) {
			content.questions += QStringLiteral("\n");
			content.answers += QStringLiteral("\n");
		}


		QStringList options;
		QVariantList correct;

		if (q.module() == QLatin1String("simplechoice")) {
			content.questions += fQuestionPrefix.arg(qNum) + data.value(QStringLiteral("question")).toString() + fQuestionSuffix;
			content.answers += fQuestionPrefix.arg(qNum++) + data.value(QStringLiteral("question")).toString() + fQuestionSuffix;

			options = data.value(QStringLiteral("options")).toStringList();
			correct << data.value(QStringLiteral("answer")).toInt();
		} else if (q.module() == QLatin1String("multichoice")) {
			content.questions += fQuestionPrefix.arg(qNum) + data.value(QStringLiteral("question")).toString() + fQuestionSuffix;
			content.answers += fQuestionPrefix.arg(qNum++) + data.value(QStringLiteral("question")).toString() + fQuestionSuffix;

			options = data.value(QStringLiteral("options")).toStringList();
			correct = data.value(QStringLiteral("answer")).toList();
		} else if (q.module() == QLatin1String("pair")) {
			content.questions += fQuestionSpecialPrefix + data.value(QStringLiteral("question")).toString() + fQuestionSpecialSuffix;
			content.answers += fQuestionSpecialPrefix + data.value(QStringLiteral("question")).toString() + fQuestionSpecialSuffix;

			options = data.value(QStringLiteral("options")).toStringList();
		}

		content.questions += fOptionListStart;
		content.answers += fOptionListStart;

		for (int i=0; i<options.size(); ++i) {

			if (correct.contains(i))
				content.answers += fOptionCorrectPrefix;

			content.questions += fOptionPrefix.arg((i < fOptionValues.size()) ? fOptionValues.at(i) : QStringLiteral("?")) + options.at(i) + fOptionSuffix;
			content.answers += fOptionPrefix.arg((i < fOptionValues.size()) ? fOptionValues.at(i) : QStringLiteral("?")) + options.at(i) + fOptionSuffix;

			if (correct.contains(i))
				content.answers += fOptionCorrectSuffix;

			if (i < options.size()-1) {
				content.questions += fOptionSeparator;
				content.answers += fOptionSeparator;
			}

		}

		content.questions += fOptionListEnd;
		content.answers += fOptionListEnd;

		content.questions += QStringLiteral("\n");
		content.answers += QStringLiteral("\n");

		if (q.module() == QLatin1String("pair")) {
			content.questions += QStringLiteral("\n");
			content.answers += QStringLiteral("\n");

			const QStringList &qList = data.value(QStringLiteral("list")).toStringList();
			const QStringList &aList = data.value(QStringLiteral("answer")).toMap().value(QStringLiteral("list")).toStringList();

			for (int i=0; i<qList.size(); ++i) {
				content.questions += fQuestionPrefix.arg(qNum) + qList.at(i) + fQuestionSuffix + QStringLiteral(" -- ________\n\n");
				content.answers += fQuestionPrefix.arg(qNum++) + qList.at(i) + fQuestionSuffix + QStringLiteral(" -- ") + fOptionCorrectPrefix;

				if (aList.size() && i < aList.size()) {
					const QString &ans = aList.at(i);
					int idx = options.indexOf(ans);

					if (idx != -1) {
						content.answers += fOptionPrefix.arg((idx < fOptionValues.size()) ? fOptionValues.at(idx) : QStringLiteral("?"));
					}

					content.answers += ans;
				}

				content.answers += fOptionCorrectSuffix + QStringLiteral("\n\n");
			}

			content.questions += QStringLiteral("\n---\n\n");
			content.answers += QStringLiteral("\n---\n\n");
		}
	}


	if (!content.questions.isEmpty() && !complexList.isEmpty()) {
		content.questions += QStringLiteral("\n---\n\n");
	}


	while (!complexList.isEmpty()) {
		const Question &q = complexList.takeAt(QRandomGenerator::global()->bounded(complexList.size()));

		const QVariantMap &data = q.generate();

		if (!content.questions.isEmpty()) {
			content.questions += QStringLiteral("\n");
		}

		content.questions += fQuestionPrefix.arg(qNum++) + data.value(QStringLiteral("question")).toString() + fQuestionSuffix
				+ QStringLiteral(" -- _________________________________________\n\n");

	}


	return content;
}





/**
 *	@brief ExamGame::mode
 * @return
 */

const ExamGame::Mode &ExamGame::mode() const
{
	return m_mode;
}

void ExamGame::setMode(const Mode &newMode)
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
	if (m_mode == ExamPaper)
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
	if (m_mode == ExamPaper)
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
	if (m_mode == ExamPaper)
		return true;
	else {
		LOG_CERROR("game") << "Missing implementation:" << __PRETTY_FUNCTION__;
		return false;
	}
}


