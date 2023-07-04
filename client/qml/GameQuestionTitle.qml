import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import CallOfSuli 1.0
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial

RowLayout {
	id: control

	property alias titleLabel: label
	property alias title: label.text
	property bool buttons: true
	property alias buttonOkEnabled: btnOk.enabled
	property alias buttonOkVisible: btnOk.visible

	readonly property GameQuestionImpl gameQuestion: (parent instanceof GameQuestionComponentImpl) ? parent.question : null

	signal buttonOkClicked()

	GameQuestionButtonPostpone {
		id: btnPostpone
		Layout.alignment: Qt.AlignVCenter
		Layout.leftMargin: 20

		visible: buttons && gameQuestion && gameQuestion.postponeEnabled
		onClicked: if (gameQuestion) gameQuestion.onPostpone()

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


	GameQuestionButtonOk {
		id: btnOk
		Layout.alignment: Qt.AlignVCenter
		Layout.rightMargin: 20
		visible: buttons
		onClicked: control.buttonOkClicked()
	}

}
