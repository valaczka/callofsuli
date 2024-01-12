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

