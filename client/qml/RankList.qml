import QtQuick 2.12
import QtQuick.Controls 2.12
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0

Item
{
	id: control

	implicitWidth: 200
	implicitHeight: 200

	Qaterial.LabelHeadline4
	{
		anchors.centerIn: parent
		text: "Rank List"
		color: "#FFF44336"
	}
}
