import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial


Column {
	id: control

	property alias title: _title.text
	property bool modified: false

	width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
	anchors.horizontalCenter: parent.horizontalCenter

	Qaterial.LabelHeadline5
	{
		id: _title
		width: parent.width
		wrapMode: Label.Wrap
		topPadding: 8
		bottomPadding: 20
		visible: text.length
	}

}
