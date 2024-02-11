import QtQuick
import QtQuick.Controls
import CallOfSuli
import Qt5Compat.GraphicalEffects
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial

Item {
	id: root

	property ConquestLandData landData: null

	implicitWidth: _label.implicitWidth
	implicitHeight: _label.implicitHeight

	x: landData ? (landData.overX - width/2) : 0
	y: landData ? (landData.overY - height/2) : 0

	readonly property bool _pickable: landData && landData.game &&
									  landData.game.currentTurn.canPick.includes(landData.landId) &&
									  landData.game.currentTurn.player === landData.game.playerId

	readonly property bool _picked: landData && landData.game &&
									landData.game.currentTurn.pickedId === landData.landId

	clip: false

	Label {
		id: _label
		anchors.centerIn: parent

		font.family: Qaterial.Style.textTheme.headline4.family
		font.pixelSize: Qaterial.Style.textTheme.headline4.pixelSize
		font.weight: Font.DemiBold

		text: landData ? landData.xp : ""
		visible: (_pickable || _picked) && landData && landData.game && landData.game.config.currentStage === ConquestTurn.StageBattle
		color: Qaterial.Colors.cyan200
	}

	Glow {
		id: _glow
		anchors.fill: _label
		source: _label
		color: Qaterial.Colors.black
		radius: 2
		spread: 0.5
		samples: 5
		visible: _label.visible && _label.text != ""
	}


	SequentialAnimation {
		running: true
		loops: Animation.Infinite
		PropertyAnimation {
			targets: [_label, _glow]
			property: "scale"
			to: 1.2
			duration: 500
		}
		PropertyAnimation {
			targets: [_label, _glow]
			property: "scale"
			to: 1.0
			duration: 500
		}
	}
}
