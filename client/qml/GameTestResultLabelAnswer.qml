import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial

Qaterial.Label {

	property bool success: true

	font.family: "Special Elite"
	font.pixelSize: Qaterial.Style.textTheme.body1.pixelSize
	color: success ? Qaterial.Colors.indigo700 : Qaterial.Colors.red500
	//lineHeight: 1.2
	font.underline: !success

	height: 0.8*implicitHeight
}
