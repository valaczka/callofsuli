import QtQuick 2.12
import QtQuick.Controls 2.12
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as JS

Qaterial.StackView
{
	id: mainStackView

	initialItem: CosImage {
		maxWidth: Math.min(mainWindow.width*0.7, 800)
		glowRadius: 6
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

		if (_index < 0)
			pop()
		else
			pop(get(_index))
	}

}
