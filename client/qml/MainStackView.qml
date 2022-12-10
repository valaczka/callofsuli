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


	function createPage(_qml : string, _prop : jsobject) : int {
		var comp = Qt.createComponent(_qml)

		if (comp.status === Component.Ready) {
			var incubator = comp.incubateObject(mainWindow, _prop)
			if (incubator.status !== Component.Ready) {
				incubator.onStatusChanged = function(status) {
					if (status === Component.Ready) {
						mainStackView.push(incubator.object)
					} else if (status === Component.Error) {
						console.warning("Component create error: ", _qml, incubator.errorString())
					}
				}
			} else {
				console.debug("Object ", incubator.object, "incubating...")
			}

			return depth

		} else if (comp.status === Component.Error) {
			console.warn("Error loading component: ", comp.errorString())
		}

		return -1
	}


	function popPage(_index : int) : int {
		if (_index >= depth)
			return -1;

		if (depth <= 2) {
			Client.closeWindow()
			return depth;
		}

		if (_index < 0)
			pop()
		else
			pop(get(_index))

		return depth
	}


	function getCloseWindowQuestion() : string {
		return qsTr("Biztosan kilépsz a programból?")
	}
}
