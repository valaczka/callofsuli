import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
import "JScript.js" as JS

QPage {
	id: root

	/*stackPopFunction: function() {
		var item = swipeView.currentItem

		if (item && item.stackPopFunction !== undefined) {
			return item.stackPopFunction()
		}

		if (swipeView.currentIndex > 0) {
			swipeView.decrementCurrentIndex()
			return false
		}

		return true
	}*/

	property TeacherExam teacherExam: null
	readonly property Exam _exam: teacherExam ? teacherExam.exam : null


	title: _exam ? (_exam.description != "" ? _exam.description : qsTr("Dolgozat #%1").arg(_exam.examId)) : ""
	subtitle: teacherExam && teacherExam.teacherGroup ? teacherExam.teacherGroup.fullName : ""

	appBar.backButtonVisible: true


	Qaterial.LabelHeadline1 {
		anchors.centerIn: parent
		text: "Hello"
	}



}
