import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import CallOfSuli 1.0
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial

RowLayout {
	id: control

	property alias title: label.text
	property bool buttons: true

	GameQuestionButton {
		id: btnPostpone
		Layout.alignment: Qt.AlignVCenter
		Layout.leftMargin: 20
		//icon.source: CosStyle.iconPostpone
		text: qsTr("Később")
		//themeColors: CosStyle.buttonThemeOrange
		//onClicked: postponed()
		visible: buttons
	}

	Label {
		id: label

		font.family: "Special Elite"
		font.pixelSize: Qaterial.Style.textTheme.headline6.pixelSize
		wrapMode: Text.Wrap
		topPadding: 25
		bottomPadding: 25
		leftPadding: 20
		rightPadding: 20

		Layout.alignment: Qt.AlignCenter
		Layout.fillWidth: true

		horizontalAlignment: Text.AlignHCenter

		color: Qaterial.Colors.amber200

		textFormat: Text.RichText
	}


	GameQuestionButton {
		id: btnOk

		buttonType: GameQuestionButton.Correct

		icon.source: Qaterial.Icons.checkBold
		text: qsTr("Kész")

		Layout.alignment: Qt.AlignVCenter
		Layout.rightMargin: 20

		visible: buttons
	}

}
