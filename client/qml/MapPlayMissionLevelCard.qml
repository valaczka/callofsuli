import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.15
import CallOfSuli 1.0
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial

Qaterial.Card {
	id: control

	property QtObject missionLevel: null

	outlined: true

	elevation: Qaterial.Style.card.activeElevation

	signal clicked(QtObject item)

	scale: area.pressed ? 0.9 : 1.0

	Behavior on scale {
		NumberAnimation { duration: 125 }
	}

	readonly property color textColor: missionLevel.solved > 0 ?
										   (missionLevel.level === 3 ?
												Qaterial.Colors.yellow500 :
												missionLevel.level === 2 ?
													Qaterial.Colors.orange500 :
													Qaterial.Colors.green500) :
										   missionLevel.lockDepth === 0 ? Qaterial.Style.colorTheme.primaryText :
																		  Qaterial.Style.colorTheme.disabledText

	contentItem: Item {
		width: parent.width
		height: parent.height

		ColumnLayout {
			id: lay
			width: parent.width
			height: parent.height
			spacing: Qaterial.Style.card.verticalPadding

			Qaterial.LabelBody2
			{
				text: missionLevel.deathmatch ?
						  qsTr("Level %1\nSudden death").arg(missionLevel.level) :
						  qsTr("Level %1").arg(missionLevel.level)

				font.capitalization: Font.AllUppercase
				font.family: Qaterial.Style.textTheme.body2.family
				font.pixelSize: Qaterial.Style.textTheme.body2.pixelSize
				font.weight: Font.DemiBold
				lineHeight: 0.9

				color: control.textColor

				horizontalAlignment: Text.AlignHCenter
				verticalAlignment: Text.AlignVCenter

				Layout.leftMargin: Qaterial.Style.card.horizontalPadding
				Layout.rightMargin: Qaterial.Style.card.horizontalPadding
				Layout.fillWidth: true
				Layout.topMargin: Qaterial.Style.card.verticalPadding
				//Layout.bottomMargin: Qaterial.Style.card.verticalPadding
				Layout.preferredHeight: font.pixelSize*2.2
			}


			MedalImage {
				deathmatch: missionLevel.deathmatch
				image: missionLevel.solved ? missionLevel.medalImage : ""
				level: missionLevel.solved ? missionLevel.level : -1

				visible: missionLevel.lockDepth === 0

				Layout.fillWidth: true
				Layout.fillHeight: true
			}

			Qaterial.Icon {
				icon: Qaterial.Icons.lock

				enabled: false

				visible: missionLevel.lockDepth > 0

				Layout.fillWidth: true
				Layout.fillHeight: true
				Layout.topMargin: Qaterial.Style.card.verticalPadding
				Layout.bottomMargin: Qaterial.Style.card.verticalPadding
			}


			Qaterial.LabelBody2
			{
				text: qsTr("%1 XP").arg(missionLevel.xp)
				color: control.textColor
				horizontalAlignment: Text.AlignHCenter
				verticalAlignment: Text.AlignVCenter
				Layout.leftMargin: Qaterial.Style.card.horizontalPadding
				Layout.rightMargin: Qaterial.Style.card.horizontalPadding
				Layout.bottomMargin: Qaterial.Style.card.verticalPadding
				//Layout.topMargin: Qaterial.Style.card.verticalPadding
				Layout.fillWidth: true
				Layout.preferredHeight: font.pixelSize*2
			}
		}

		MouseArea {
			id: area
			anchors.fill: parent
			acceptedButtons: Qt.LeftButton

			onClicked: if (missionLevel.lockDepth === 0) control.clicked(missionLevel)
		}
	}


}
