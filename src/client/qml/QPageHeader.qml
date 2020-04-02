import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.14
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

Rectangle {
	id: control
	anchors.top: parent.top
	anchors.left: parent.left
	anchors.right: parent.right
	color: CosStyle.colorPrimaryDark

	Rectangle {
		anchors.bottom: parent.bottom
		width: parent.width
		height: 1
		color: CosStyle.colorAccent
	}
}

