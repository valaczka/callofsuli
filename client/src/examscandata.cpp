/*
 * ---- Call of Suli ----
 *
 * %{Cpp:License:FileName}
 *
 * Created on: %{CurrentDate:yyyy. MM. dd.}
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * %{Cpp:License:ClassName}
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



#include "examscandata.h"



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

int ExamScanData::maxPoint() const
{
    return m_maxPoint;
}

void ExamScanData::setMaxPoint(int newMaxPoint)
{
    if (m_maxPoint == newMaxPoint)
        return;
    m_maxPoint = newMaxPoint;
    emit maxPointChanged();
}

int ExamScanData::point() const
{
    return m_point;
}

void ExamScanData::setPoint(int newPoint)
{
    if (m_point == newPoint)
        return;
    m_point = newPoint;
    emit pointChanged();
}

int ExamScanData::gradeId() const
{
    return m_gradeId;
}

void ExamScanData::setGradeId(int newGradeId)
{
    if (m_gradeId == newGradeId)
        return;
    m_gradeId = newGradeId;
    emit gradeIdChanged();
}
