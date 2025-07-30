import QtQuick
import QtQuick.Controls
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as JS

Qaterial.ListDialog
{
	id: _dialog

	property RpgGameImpl game: null

	property string iconSource: ""
	property string text: ""
	property color iconColor: Qaterial.Style.accentColor
	property color textColor: Qaterial.Style.primaryTextColor()
	property int iconSize: 32 * Qaterial.Style.pixelSizeRatio
	property bool showFailed: false

	title: qsTr("Quests")

	dialogImplicitWidth: 450 * Qaterial.Style.pixelSizeRatio
	autoFocusButtons: true

	model: game ? game.quests : null

	delegate: _cmp

	standardButtons: DialogButtonBox.Ok

	listViewHeader: _dialog.iconSource != "" || _dialog.text != "" ? _cmpHeader : null

	Component {
		id: _cmpHeader
		RpgQuestHeader {
			iconSource: _dialog.iconSource
			text: _dialog.text
			iconColor: _dialog.iconColor
			textColor: _dialog.textColor
			iconSize: _dialog.iconSize

			width: ListView.view.width
		}
	}

	Component {
		id: _cmp
		RpgQuestDelegate {

			showFailed: _dialog.showFailed

			width: ListView.view.width
		}
	}
}
