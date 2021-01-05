import QtQuick 2.12
import QtQuick.Controls 2.12
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

Item {
	id: control

	implicitHeight: labelQuestion.implicitHeight+row.implicitHeight+25
	implicitWidth: 800

	property var questionData: null
	property bool interactive: true

	signal succeed()
	signal failed()


	QLabel {
		id: labelQuestion

		font.family: "Special Elite"
		font.pixelSize: CosStyle.pixelSize*1.5
		wrapMode: Text.Wrap
		anchors.top: parent.top
		anchors.left: parent.left
		anchors.right: parent.right
		topPadding: 50
		bottomPadding: 50

		horizontalAlignment: Text.AlignHCenter

		color: CosStyle.colorAccent

		text: questionData.question
	}

	Item {
		anchors.top: labelQuestion.bottom
		anchors.right: parent.right
		anchors.left: parent.left
		anchors.bottom: parent.bottom
		anchors.bottomMargin: 20

		Row {
			id: row
			anchors.centerIn: parent
			spacing: 30

			QButton {
				id: btnTrue
				text: qsTr("Igaz")
				icon.source: CosStyle.iconOK
				onClicked: if (interactive) { answer(questionData.correct) }
			}

			QButton {
				id: btnFalse
				text: qsTr("Hamis")
				icon.source: CosStyle.iconCancel
				onClicked: if (interactive) { answer(!questionData.correct) }
			}
		}
	}


	function answer(correct) {
		interactive = false

		if (correct)
			succeed()
		else
			failed()

		if (questionData.correct) {
			btnTrue.themeColors = CosStyle.buttonThemeApply
			btnFalse.enabled = false
			btnTrue.forceActiveFocus()
		} else {
			btnTrue.enabled = false
			btnFalse.themeColors = CosStyle.buttonThemeApply
			btnFalse.forceActiveFocus()
		}
	}
}

