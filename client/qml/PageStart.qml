import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as JS

QPage {
	id: control

	//closeQuestion: StackView.index < 3 ? "Nem tudod, miért nem szeretnéd bezárni?" : ""
	//closeDisabled: (StackView.index == 4) ? "Nem lehet bezárni!" : ""

	header: Qaterial.ToolBar {
		backgroundColor: "transparent"
		elevation: 1
		RowLayout {
			anchors.fill: parent
			Layout.fillWidth: true
			Layout.preferredHeight: Qaterial.Style.toolbar.implicitHeight
			property alias title: _titleLabel.text

			Qaterial.AppBarButton
			{
				id: _backButton
				icon.source: Qaterial.Icons.arrowLeft
				onClicked: Client.stackPop()
				//onPrimary: true
			} // AppBarButton

			Qaterial.LabelHeadline6
			{
				id: _titleLabel

				Layout.fillWidth: true
				Layout.leftMargin: !_backButton.visible ? 20 : undefined
				onPrimary: true

				text: "Default text"
				elide: Qaterial.Label.ElideRight
			} // Label

			Qaterial.AppBarButton
			{
				icon.source: Qaterial.Icons.palette
				onPrimary: true
				onClicked: Client.stackPushPage("PageStart.qml", {})
			} // AppBarButton
		} // RowLayout
	}

	Qaterial.RaisedButton {
		text: "Start Game"
		icon.source: Qaterial.Icons.gamepad
		onClicked: Client.loadGame()
		foregroundColor: Qaterial.Colors.black
	}

	StackView.onActivated: Client.loadGame()
}
