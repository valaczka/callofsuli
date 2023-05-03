import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import SortFilterProxyModel 0.2
import "JScript.js" as JS

Row {
	id: root

	property User user: null

	property real iconSize: Qaterial.Style.mediumIcon
	property int maxIconCount: 10
	property int duration: 650
	property int value: user ? user.streak : 0
	property bool forceToday: false

	readonly property bool _hasToday: forceToday || (user && user.streakToday)
	readonly property int _iconCount: Math.max(3, value+(_hasToday ? 0 : 1))
	readonly property int _diff: Math.max(_iconCount-maxIconCount, 0)

	readonly property real implicitLabelWidth: _labelStreak.width+_labelStreakNum.width


	Behavior on value {
		NumberAnimation { duration: root.duration; easing.type: Easing.OutQuart }
	}

	Qaterial.LabelHeadline5 {
		id: _labelStreak
		anchors.verticalCenter: parent.verticalCenter
		font: Qaterial.Style.textTheme.headline5
		text: qsTr("Streak")
		rightPadding: 15
	}

	Repeater {
		model: root._iconCount-root._diff

		Qaterial.Icon {
			anchors.verticalCenter: parent.verticalCenter
			icon: Qaterial.Icons.fire
			color: ((root._diff+index) < root.value) ? Qaterial.Colors.orange500 : Qaterial.Style.disabledTextColor()
			width: root.iconSize
			height: root.iconSize
		}
	}

	Qaterial.LabelHeadline5 {
		id: _labelStreakNum
		anchors.verticalCenter: parent.verticalCenter
		font: Qaterial.Style.textTheme.headline5
		text: root.value
		leftPadding: 15
	}
}
