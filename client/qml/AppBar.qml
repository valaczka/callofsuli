import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial

Qaterial.ToolBar {
	id: control

	property alias title: titleLabel.text
	property alias subtitle: subtitleLabel.text
	property alias backButtonVisible: backButton.visible

	property alias rightComponent: rightLoader.sourceComponent
	property alias rightComponentSource: rightLoader.source

	topPadding: Client.safeMarginTop

	RowLayout {
		anchors.fill: parent
		Layout.fillWidth: true
		Layout.preferredHeight: Qaterial.Style.toolbar.implicitHeight

		Qaterial.AppBarButton
		{
			id: backButton
			icon.source: Qaterial.Icons.arrowLeft
			onClicked: Client.stackPop()
		}

		Column {
			Layout.fillWidth: true
			Layout.leftMargin: !backButton.visible ? 20 : undefined

			spacing: 0

			Qaterial.LabelHeadline6
			{
				id: titleLabel
				width: parent.width

				elide: Qaterial.Label.ElideRight
				bottomPadding: 0
				topPadding: 0
				lineHeight: 0.9
				visible: text.length
			}

			Qaterial.LabelCaption
			{
				id: subtitleLabel
				width: parent.width

				elide: Qaterial.Label.ElideRight

				bottomPadding: 0
				topPadding: 0
				lineHeight: 0.9
				visible: text.length
			}

		}

		Loader {
			id: rightLoader

		}
	}
}
