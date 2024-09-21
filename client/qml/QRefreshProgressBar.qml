import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial

Qaterial.ProgressBar
{
	id: progressbar
	width: parent.width
	indeterminate: true
	color: Qaterial.Style.iconColor()
}
