import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli

Column {
	id: root

	signal musicVolumeModified()

	Timer {
		id: _delayTimer
		interval: 250
		triggeredOnStart: false
		running: false

		property int soundType: Sound.GameSound

		onTriggered: Client.playSound("qrc:/sound/sfx/question.mp3", soundType)
	}

	GridLayout {
		width: parent.width
		columns: 3
		columnSpacing: 5 * Qaterial.Style.pixelSizeRatio


		// Volume SFX

		Qaterial.IconLabel {
			id: _label1
			text: qsTr("Effektek")
			icon.color: _volSfx.value > 0 ? Qaterial.Style.primaryTextColor() : Qaterial.Style.disabledTextColor()
			icon.source: _volSfx.value > _volSfx.to * 0.66 ?
							 Qaterial.Icons.volumeHigh :
							 _volSfx.value > _volSfx.to * 0.33 ? Qaterial.Icons.volumeMedium :
																 _volSfx.value > 0 ? Qaterial.Icons.volumeLow :
																					 Qaterial.Icons.volumeOff
			Layout.fillWidth: false
			Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
		}

		Qaterial.Slider {
			id: _volSfx
			from: 0
			to: 100
			stepSize: 1
			value: Client.volumeSfx
			Layout.fillWidth: true
			Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

			onMoved: {
				Client.volumeSfx = _volSfx.value
				_delayTimer.soundType = Sound.GameSound
				_delayTimer.restart()
			}
		}


		Qaterial.LabelBody2 {
			text: Client.volumeSfx
			Layout.minimumWidth: Math.max(implicitWidth, 25 * Qaterial.Style.pixelSizeRatio)
			Layout.fillWidth: false
			Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
			horizontalAlignment: Text.AlignRight
		}



		// Volume Voiceover

		Qaterial.IconLabel {
			text: qsTr("Beszéd")
			icon.color: _volVoiceOver.value > 0 ? Qaterial.Style.primaryTextColor() : Qaterial.Style.disabledTextColor()
			icon.source: _volVoiceOver.value > _volVoiceOver.to * 0.66 ?
							 Qaterial.Icons.volumeHigh :
							 _volVoiceOver.value > _volVoiceOver.to * 0.33 ? Qaterial.Icons.volumeMedium :
																 _volVoiceOver.value > 0 ? Qaterial.Icons.volumeLow :
																					 Qaterial.Icons.volumeOff
			Layout.fillWidth: false
			Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
		}

		Qaterial.Slider {
			id: _volVoiceOver
			from: 0
			to: 100
			stepSize: 1
			value: Client.volumeVoiceOver
			Layout.fillWidth: true
			Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

			onMoved: {
				Client.volumeVoiceOver = _volVoiceOver.value
				_delayTimer.soundType = Sound.VoiceOver
				_delayTimer.restart()
			}
		}


		Qaterial.LabelBody2 {
			text: Client.volumeVoiceOver
			Layout.fillWidth: false
			Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
		}


		// Volume Music

		Qaterial.IconLabel {
			text: qsTr("Háttérzene")
			icon.color: _volMusic.value > 0 ? Qaterial.Style.primaryTextColor() : Qaterial.Style.disabledTextColor()
			icon.source: _volMusic.value > _volMusic.to * 0.66 ?
							 Qaterial.Icons.volumeHigh :
							 _volMusic.value > _volMusic.to * 0.33 ? Qaterial.Icons.volumeMedium :
																 _volMusic.value > 0 ? Qaterial.Icons.volumeLow :
																					 Qaterial.Icons.volumeOff
			Layout.fillWidth: false
			Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
		}

		Qaterial.Slider {
			id: _volMusic
			from: 0
			to: 100
			stepSize: 1
			value: Client.volumeMusic
			Layout.fillWidth: true
			Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

			onMoved: {
				Client.volumeMusic = _volMusic.value
				musicVolumeModified()
			}
		}


		Qaterial.LabelBody2 {
			text: Client.volumeMusic
			Layout.fillWidth: false
			Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
		}



	}

	Row {
		visible: Qt.platform.os == "ios" || Qt.platform.os == "android"

		topPadding: 5 * Qaterial.Style.pixelSizeRatio

		QFormSwitchButton
		{
			text: qsTr("Rezgés engedélyezése")
			anchors.verticalCenter: parent.verticalCenter
			checked: Client.vibrate
			onToggled: Client.vibrate = checked
		}

		Qaterial.Icon
		{
			icon: Client.vibrate ? Qaterial.Icons.vibrate : Qaterial.Icons.vibrateOff
			implicitWidth: _label1.icon.width
			implicitHeight: _label1.icon.height
			color: Client.vibrate ? Qaterial.Style.primaryTextColor() : Qaterial.Style.disabledTextColor()
			anchors.verticalCenter: parent.verticalCenter
		}

	}

}
