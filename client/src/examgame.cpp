#include "examgame.h"
#include "Logger.h"
#include "application.h"


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
			ModuleInterface *iface = Application::instance()->objectiveModules().value(objective->module());

			if (!iface || (!iface->types().testFlag(ModuleInterface::PaperAuto) &&
						   !iface->types().testFlag(ModuleInterface::PaperManual)))
				continue;

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

	static QStringList easyModules;

	if (easyModules.isEmpty()) {
		LOG_CTRACE("game") << "Generate easy modules list";

		for (const auto &[key, iface] : Application::instance()->objectiveModules().asKeyValueRange()) {
			if (!iface || !iface->types().testFlag(ModuleInterface::PaperAuto))
				continue;
			else
				easyModules.append(key);
		}

	}

	QJsonArray retList;


	for (auto it=list.constBegin(); it != list.constEnd(); ++it) {
		if (easyModules.contains(it->module()))
			easyList.append(*it);
		else
			complexList.append(*it);
	}


	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(easyList.begin(), easyList.end(), g);
	std::shuffle(complexList.begin(), complexList.end(), g);

	QHash<QString, QJsonObject> commonData;

	for (const Question &q : easyList) {
		const QVariantMap &data = q.generate();

		if (const QVariantMap &d = q.commonData(); !d.isEmpty() && !commonData.contains(q.uuid())) {
			QJsonObject json = QJsonObject::fromVariantMap(d);
			json[QStringLiteral("uuid")] = q.uuid();
			json[QStringLiteral("module")] = q.module();
			json[QStringLiteral("common")] = true;
			commonData.insert(q.uuid(), json);
		}

		QJsonObject json = QJsonObject::fromVariantMap(data);
		json[QStringLiteral("uuid")] = q.uuid();
		json[QStringLiteral("module")] = q.module();
		retList.append(json);
	}

	bool isFirst = true;

	for (const Question &q : complexList) {
		const QVariantMap &data = q.generate();

		if (const QVariantMap &d = q.commonData(); !d.isEmpty() && !commonData.contains(q.uuid())) {
			QJsonObject json = QJsonObject::fromVariantMap(d);
			json[QStringLiteral("uuid")] = q.uuid();
			json[QStringLiteral("module")] = q.module();
			json[QStringLiteral("common")] = true;
			commonData.insert(q.uuid(), json);
		}

		QJsonObject json = QJsonObject::fromVariantMap(data);
		json[QStringLiteral("uuid")] = q.uuid();
		json[QStringLiteral("module")] = q.module();
		if (isFirst) {
			json[QStringLiteral("separator")] = true;
			isFirst = false;
		}
		retList.append(json);
	}

	for (const QJsonObject &v : commonData)
		retList.append(v);

	return retList;
}


/**
 * @brief ExamGame::clearQuestions
 * @param missionLevel
 */

void ExamGame::clearQuestions(GameMapMissionLevel *missionLevel)
{
	LOG_CDEBUG("game") << "Clear questions";

	if (!missionLevel) {
		LOG_CWARNING("game") << "Missing game map, don't cleared any question";
		return;
	}

	foreach (GameMapChapter *chapter, missionLevel->chapters()) {
		foreach (GameMapObjective *objective, chapter->objectives()) {
			objective->generatedQuestions().clear();
			objective->commonData().clear();
		}
	}
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

