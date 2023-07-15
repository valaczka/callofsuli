import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial

Qaterial.ProgressBar
{
	id: progressbar
	width: parent.width
	indeterminate: true
	color: Qaterial.Style.iconColor()
}
