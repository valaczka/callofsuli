import QtQuick 2.12
import QtQuick.Controls 2.12
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as JS

Qaterial.StackView
{
	id: mainStackView

	signal stackViewLoaded()

	initialItem: Item {
		Column {
			anchors.centerIn: parent
			spacing: 5

			Text {
				text: "Call of Suli"
				anchors.horizontalCenter: parent.horizontalCenter
				color: Qaterial.Style.iconColorDark
				font.pixelSize: 96
			}

			Text {
				id: loadingLabel
				anchors.horizontalCenter: parent.horizontalCenter
				color: Qaterial.Style.iconColorDark
				font.pixelSize: 18

				readonly property string _txt: qsTr("Betöltés")

				Timer {
					interval: 200
					triggeredOnStart: true
					running: true

					property int num: 1

					onTriggered:  {
						if (num >= 3)
							num = 1
						else
							num++

						loadingLabel.text = loadingLabel._txt+String(".").repeat(num)
					}
				}
			}
		}

		Component.onCompleted: mainStackView.stackViewLoaded()
	}

	Transition {
		id: transitionEnter

		PropertyAnimation {
			property: "opacity"
			from: 0.0
			to: 1.0
		}
	}

	Transition {
		id: transitionExit

		PropertyAnimation {
			property: "opacity"
			from: 1.0
			to: 0.0
		}
	}

	pushEnter: transitionEnter
	pushExit: transitionExit
	popEnter: transitionEnter
	popExit: transitionExit



	function createPage(_qml : string, _prop : jsobject) : Item {
		return mainStackView.push(_qml, _prop)
	}


		function popPage(_index : int) {
		if (currentItem.onPageClose) {
			console.info(qsTr("Lap bezárási funkció meghívása:"), currentItem)
			currentItem.onPageClose()
		}

			if (_index >= depth)
			return

			if (_index < 0)
			pop()
			else
			pop(get(_index))
		}



			function callStackPop() : bool {
				if (currentItem.stackPopFunction) {
					console.info(qsTr("Lap pop funkció meghívása:"), currentItem)
					return currentItem.stackPopFunction()
				}

					return true
				}

				}
