import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as JS

Qaterial.StackView
{
	id: mainStackView

	signal stackViewLoaded()

	initialItem: Item {
		CosImage {
			width: Math.min(mainStackView.width*0.7, 800)
			anchors.centerIn: parent
			glowRadius: 6

			image.onStatusChanged: if (image.status == Image.Ready) {
									   mainStackView.stackViewLoaded()
								   }
		}
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
		if (_index < 0 || _index >= depth) {
			console.warn(qsTr("Invalid index"), index)
			return
		}

		if (currentItem.onPageClose) {
			console.debug(qsTr("Lap bezárási funkció meghívása:"), currentItem)
			currentItem.onPageClose()
		}

		pop(get(_index))
	}



	function callStackPop() : bool {
		if (currentItem.StackView.status !== StackView.Active) {
			console.debug(qsTr("StackView.status != Active"), currentItem.StackView.status)
			return false
		}

		if (currentItem.stackPopFunction) {
			console.debug(qsTr("Lap pop funkció meghívása:"), currentItem)
			return currentItem.stackPopFunction()
		}

		return true
	}

}
