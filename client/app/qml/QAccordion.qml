import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

Item {
	id: control

	implicitHeight: 400
	implicitWidth: 400


	property alias flickable: flick
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
}

