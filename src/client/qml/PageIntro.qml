import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


Page {
	id: pageIntro

	property Intro intro: null

	property bool hasWindowCloseRequest: false

	signal pagePopulated()



	background: Rectangle {
		color: "black"
	}


	Item {
		id: loadingItem
		anchors.fill: parent

		BusyIndicator {
			anchors.centerIn: parent
			width: 100
			height: 100
			running: true
		}
	}

	QButton {
		anchors.right: parent.right
		anchors.verticalCenter: parent.verticalCenter

		text: qsTr("end")

		onClicked: {
			mainStack.back()
		}
	}

	StackView.onRemoved: destroy()

	StackView.onActivated: {
		pagePopulated()
	}


	function windowClose() {
		return true
	}


	function stackBack() {
		if (mainStack.depth > pageIntro.StackView.index+1) {
			if (!mainStack.get(pageIntro.StackView.index+1).stackBack()) {
				if (mainStack.depth > pageIntro.StackView.index+1) {
					mainStack.pop(pageIntro)
				}
			}
			return true
		}

		return false
	}
}
