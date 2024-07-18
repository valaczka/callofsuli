import QtQuick
import QtQuick.Controls
import Qt5Compat.GraphicalEffects
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli

Column {
	id: root

	required property TiledObjectImpl target

	readonly property bool _isEntity: target && target.hp !== undefined

	parent: target.scene

	visible: target && target.game && (!_isEntity || target.hp > 0)

	y: target ? target.y-20 : 0
	x: target ? target.x+(target.width-width)/2 : 0
	z: 99

	width: Math.min(70, Math.max(_label.implicitWidth, _progress.implicitWidth, 40))

	Item {
		id: _labelItem

		width: _label.width
		height: _label.height
		anchors.horizontalCenter: parent.horizontalCenter

		Qaterial.Label {
			id: _label
			font.family: Qaterial.Style.textTheme.body1.family
			font.pixelSize: 10
			font.weight: Font.Bold
			color: "white"
			//elide: implicitWidth > width ? Text.ElideRight : Text.ElideNone
			wrapMode: Text.Wrap
			text: target ? target.displayName : ""
			width: root.width
			horizontalAlignment: Text.AlignHCenter
		}

		Glow {
			anchors.fill: _label
			source: _label
			color: "black"
			radius: 1
			spread: 0.9
			samples: 5
		}
	}


	ProgressBar {
		id: _progress
		visible: _isEntity
		anchors.horizontalCenter: parent.horizontalCenter
		width: Math.min(40, parent.width)

		from: 0
		to: _isEntity ? target.maxHp : 0
		value: _isEntity ? target.hp : 0

		Material.accent: color

		readonly property color color: {
			if (!target || !target.game || !_isEntity)
				return Qaterial.Colors.gray

			let p = target.hp/target.maxHp

			/*if (target.game.followedItem != target)
				return Qaterial.Colors.blue500*/

			if (p > 0.5)
				return Qaterial.Colors.green500
			else if (p > 0.3)
				return Qaterial.Colors.amber500
			else
				return Qaterial.Colors.red500
		}

		Behavior on value {
			NumberAnimation { duration: 175; easing.type: Easing.InOutQuad }
		}
	}

}



