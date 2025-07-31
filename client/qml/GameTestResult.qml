import QtQuick
import QtQuick.Controls
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as JS


Item {
	id: root

	property alias textDocument: _textEdit.textDocument

	implicitWidth: 200
	implicitHeight: 200

	width: Math.min(parent.width, 1200)
	height: _textEdit.height+85

	BorderImage {
		width: parent.width/scale
		height: parent.height/scale
		source: "qrc:/internal/img/paper_bw.png"
		border {
			left: 253
			top: 129
			right: 1261-1175
			bottom: 851-737
		}
		scale: Qaterial.Style.dense ? 0.3 : 0.15
		transformOrigin: Item.TopLeft
	}

	TextEdit {
		id: _textEdit

		x: Qaterial.Style.dense ? 50 : 25
		width: parent.width-2*x
		y: Qaterial.Style.dense ? 35 : 20

		color: Qaterial.Colors.black
		readOnly: true
		selectByKeyboard: false
		selectByMouse: false
		wrapMode: Text.Wrap

		font.pixelSize: Qaterial.Style.textTheme.body1.pixelSize
		font.family: Qaterial.Style.textTheme.body1.family
		font.weight: Qaterial.Style.textTheme.body1.weight
	}
}
