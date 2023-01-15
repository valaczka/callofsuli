import QtQuick 2.12
import QtQuick.Controls 2.12
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial


Column {
	id: control

	property alias title: _title.text
	property bool modified: false

	width: Math.min(parent.width, 700)
	anchors.horizontalCenter: parent.horizontalCenter

	Qaterial.LabelHeadline5
	{
		id: _title
		width: parent.width
		wrapMode: Label.Wrap
		topPadding: 15
		bottomPadding: 20
		leftPadding: Qaterial.Style.horizontalPadding
		rightPadding: Qaterial.Style.horizontalPadding
		visible: text.length
	}

}
