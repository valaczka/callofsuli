import QtQuick
import QtQuick.Controls
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial


QPage {
	id: root

	TeacherExam {
		id: _teacherExam
	}

	QButton {
		text: "TEST"
		onClicked: _teacherExam.test()
	}
}


