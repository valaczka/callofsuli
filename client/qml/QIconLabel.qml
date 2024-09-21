import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli

Item {
	id: root

	property double topPadding: 0
	property double leftPadding: 0
	property double rightPadding: 0
	property double bottomPadding: 0

	implicitWidth: _iconLabel.implicitWidth+leftPadding+rightPadding
	implicitHeight: _iconLabel.implicitHeight+topPadding+bottomPadding

	property alias iconLabelItem: _iconLabel

	property alias text: _iconLabel.text
	property alias color: _iconLabel.color
	property alias font: _iconLabel.font
	property alias elide: _iconLabel.elide
	property alias wrapMode: _iconLabel.wrapMode
	property alias maximumLineCount: _iconLabel.maximumLineCount

	property alias horizontalAlignment: _iconLabel.horizontalAlignment
	property alias verticalAlignment: _iconLabel.verticalAlignment
	property alias display: _iconLabel.display
	property alias spacing: _iconLabel.spacing
	property alias mirrored: _iconLabel.mirrored

	property alias icon: _iconLabel.icon

	property alias iconItem: _iconLabel.iconItem
	property alias labelItem: _iconLabel.labelItem

	Qaterial.IconLabel
	{
		id: _iconLabel
		width: parent.width-root.leftPadding-root.rightPadding
		x: root.leftPadding
		height: parent.height-root.topPadding-root.bottomPadding
		y: root.topPadding
	}
}
