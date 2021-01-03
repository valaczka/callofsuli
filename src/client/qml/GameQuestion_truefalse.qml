import QtQuick 2.12
import QtQuick.Controls 2.12
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

Item {
	id: control

	property var questionData: null

	signal succeed()
	signal failed()


	QLabel {
		id: labelQuestion

		font.family: "Special Elite"
		font.pixelSize: CosStyle.pixelSize*1.2
		wrapMode: Text.Wrap
		anchors.top: parent.top
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.topMargin: 20

		horizontalAlignment: Text.AlignHCenter

		color: CosStyle.colorAccent

		text: questionData.question
	}

	Item {
		anchors.top: labelQuestion.bottom
		anchors.right: parent.right
		anchors.left: parent.left
		anchors.bottom: parent.bottom

		Row {
			anchors.centerIn: parent
			spacing: 10

			QButton {
				text: qsTr("Igaz")
				icon.source: CosStyle.iconOK
				themeColors: CosStyle.buttonThemeApply
				onClicked: {
					if (questionData.correct)
						control.succeed()
					else
						control.failed()
				}
			}

			QButton {
				text: qsTr("Hamis")
				icon.source: CosStyle.iconCancel
				themeColors: CosStyle.buttonThemeDelete
				onClicked: {
					if (questionData.correct)
						control.failed()
					else
						control.succeed()
				}
			}
		}
	}
}

