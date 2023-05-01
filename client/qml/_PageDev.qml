import QtQuick 2.15
import QtQuick.Controls 2.15
import CallOfSuli 1.0
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as JS


Page {
	id: control

	Image {
		anchors.fill: parent
		fillMode: Image.PreserveAspectCrop
		source: "qrc:/internal/img/villa.png"
	}


	QScrollable {
		anchors.fill: parent

		Qaterial.LabelHeadline1 {
			text: "hello"
		}

		TextEdit {
			id: _cmp
			width: parent.width
			color: "white"
			readOnly: true
			wrapMode: Text.Wrap

			font.pixelSize: Qaterial.Style.textTheme.body1.pixelSize
			font.family: Qaterial.Style.textTheme.body1.family
			font.weight: Qaterial.Style.textTheme.body1.weight

			Component.onCompleted: Client.testDocument(textDocument, Qaterial.Style.textTheme.body1.pixelSize)
		}
	}

}
