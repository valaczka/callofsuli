import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import CallOfSuli 1.0
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial

Qaterial.Card {
	id: control

	property MapPlayMissionLevel missionLevel: null
	property bool readOnly: false

	outlined: true

	Binding {
		target: control
		property: "backgroundColor"
		value: "transparent"
		when: missionLevel.lockDepth > 0 || readOnly

	}

	borderColor: readOnly || missionLevel.lockDepth > 0 ? "transparent" :
											  missionLevel.solved === 0 && missionLevel.lockDepth === 0
											  ? Qaterial.Colors.green500 :
												enabled ? Qaterial.Style.dividersColor() : Qaterial.Style.disabledDividersColor()


	elevation: missionLevel.lockDepth > 0 ? 0 : Qaterial.Style.card.activeElevation

	signal clicked(MapPlayMissionLevel item)

	scale: area.pressed ? 0.9 : 1.0

	Behavior on scale {
		NumberAnimation { duration: 125 }
	}

	readonly property int _verticalPadding: 6//Qaterial.Style.dense ? 6 : 8

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
			spacing: control._verticalPadding

			MedalImage {
				deathmatch: missionLevel.deathmatch
				image: missionLevel.solved ? missionLevel.medalImage : ""
				level: missionLevel.solved ? missionLevel.level : -1

				visible: missionLevel.lockDepth === 0

				Layout.topMargin: control._verticalPadding
				Layout.fillWidth: true
				Layout.fillHeight: true
			}

			Qaterial.Icon {
				icon: Qaterial.Icons.lock

				enabled: false

				visible: missionLevel.lockDepth > 0

				Layout.fillWidth: true
				Layout.fillHeight: true
				Layout.topMargin: 2*control._verticalPadding
				Layout.bottomMargin: 2*control._verticalPadding
			}

			Qaterial.Label
			{
				text: (missionLevel.deathmatch ?
						   qsTr("Level %1 SD").arg(missionLevel.level) :
						   qsTr("Level %1").arg(missionLevel.level))
					  +qsTr("<br><b>%1 XP</b>").arg(missionLevel.xp)

				font: Qaterial.Style.textTheme.body2Upper

				lineHeight: 0.9
				textFormat: Text.StyledText

				color: control.textColor

				horizontalAlignment: Text.AlignHCenter
				verticalAlignment: Text.AlignVCenter

				Layout.leftMargin: Qaterial.Style.card.horizontalPadding
				Layout.rightMargin: Qaterial.Style.card.horizontalPadding
				Layout.fillWidth: true
				//Layout.topMargin: control._verticalPadding
				Layout.bottomMargin: control._verticalPadding
				Layout.preferredHeight: font.pixelSize*2.3
			}

		}

		MouseArea {
			id: area
			anchors.fill: parent
			acceptedButtons: Qt.LeftButton

			onClicked: control.clicked(missionLevel)
		}
	}


}
