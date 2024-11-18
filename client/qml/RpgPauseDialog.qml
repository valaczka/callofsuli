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

	title: qsTr("Paused")

	dialogImplicitWidth: 450 * Qaterial.Style.pixelSizeRatio
	autoFocusButtons: true

	signal exitRequest()

	model: [
		{
			text: qsTr("Resume"),
			icon: Qaterial.Icons.arrowLeftBold,
			color: Qaterial.Style.accentColor,
			mode: 0
		},
		{
			text: qsTr("Exit"),
			icon: Qaterial.Icons.close,
			color: Qaterial.Colors.red400,
			mode: 1
		}

	]

	delegate: _cmp

	standardButtons: DialogButtonBox.Cancel

	Connections {
		target: game

		function onPausedChanged() {
			if (!game.paused)
				_dialog.close()
		}
	}

	Component {
		id: _cmp

		Qaterial.ItemDelegate
		{
			id: _delegate

			text: modelData.text

			width: ListView.view.width

			icon.source: modelData.icon
			iconColor: modelData.color
			textColor: modelData.color

			onClicked: {
				if (modelData.mode == 1)
					_dialog.exitRequest()

				_dialog.close()
			}
		}
	}
}
