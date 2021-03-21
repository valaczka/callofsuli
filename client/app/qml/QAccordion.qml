import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

Rectangle {
	id: control

	implicitHeight: 400
	implicitWidth: 400


	property bool reparented: false
	property Item reparentedParent: null

	color: reparented ? JS.setColorAlpha("black", 0.4) : "transparent"

	default property alias contents: col.data

	anchors.fill: parent

	Flickable {
		id: flick

		anchors.fill: parent

		contentWidth: col.width
		contentHeight: col.height

		clip: true

		flickableDirection: Flickable.VerticalFlick

		boundsBehavior: Flickable.StopAtBounds

		Column {
			id: col
			width: control.width
		}

		ScrollIndicator.vertical: ScrollIndicator { }
	}


	states: State {
		when: reparented
		ParentChange { target: control; parent: reparentedParent }
	}
}

