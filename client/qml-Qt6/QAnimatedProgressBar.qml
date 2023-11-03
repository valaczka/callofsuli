import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial

Column {
	id: control

	property string icon: ""
	property alias value: _bar.value
	property alias to: _bar.to
	property alias from: _bar.from
	property color color: Qaterial.Style.iconColor()
	property string textFormat: "%1"
	property alias rightText: _rightLabel.text
	property int duration: 650

	spacing: 0

	Qaterial.IconLabel {
		id: _valueLabel
		anchors.left: parent.left
		font: Qaterial.Style.textTheme.headline5
		text: control.textFormat.arg(Math.floor(_bar.value).toLocaleString())
		color: control.color
		icon.source: control.icon
		visible: control.icon != "" || control.textFormat != ""
	}

	Row {
		id: _row

		spacing: 15

		Qaterial.ProgressBar {
			id: _bar
			width: control.width-(_rightLabel.visible ? _row.spacing+_rightLabel.width : 0)

			from: 0
			to: Math.max(value, 1)
			value: 0

			color: control.color

			anchors.verticalCenter: parent.verticalCenter

			Behavior on value {
				NumberAnimation { duration: control.duration; easing.type: Easing.OutQuart }
			}
		}

		Qaterial.LabelCaption {
			id: _rightLabel
			anchors.verticalCenter: parent.verticalCenter
			color: control.color
			visible: text != ""
		}
	}


}
/*ProfileDetailsProgress {
	id: barXP
	width: parent.width*0.75
	anchors.horizontalCenter: parent.horizontalCenter
	textFormat: "%1 XP"
	color: CosStyle.colorOKLighter

	visible: (cosClient.userRoles & Client.RoleStudent)

	property int rank: -1

	onRankChanged: {
		var n = cosClient.nextRank(rank)
		if (!Object.keys(n).length) {
			textTo = ""
			to = 0
			from = 0
		} else {
			to = n.next.xp
			from = n.current.xp
			textTo = n.next.rankname+(n.next.ranklevel > 0 ? " (lvl "+n.next.ranklevel+")" : "")
		}
	}
}*/
