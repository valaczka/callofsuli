import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial

Qaterial.Pane
{
	id: control

	width: 32
	height: 32
	padding: 0
	x: (handler.width - width) / 2
	y: (dragProgress > 100) ? 100 : dragProgress

	property bool readyToRefresh: false

	/*Material.elevation: 2
	background: Rectangle {
		id: backgroud_rect
		color: Material.background
		radius: width / 2
		layer.enabled: control.enabled && control.Material.elevation > 0
		layer.effect: ElevationEffect {
			elevation: control.Material.elevation
		}
	}*/

	Qaterial.RoundColorIcon {
		id: arrow
		anchors.fill: parent
		source: Qaterial.Icons.refresh
		iconSize: control.width*0.8
		roundSize: control.width
		transformOrigin: Item.Center
		fill: true
		onPrimary: true
		//Behavior on rotation { NumberAnimation { duration: 200 } }
		opacity: (dragProgress < 50) ? 0.5*(dragProgress/50) : 1

		color: readyToRefresh ? Qaterial.Colors.green400 : Qaterial.Style.iconColor()

		rotation:
		{
			 if (readyToRefresh)
				 return rotation;
			 if (dragProgress >= 50 && dragProgress <= 100)
				return ((dragProgress - 50) * 200) / 50;
			 return 0;
		}
	}
  /*  Shape
	{
		id: shape
		anchors.fill: parent
		asynchronous: true
		antialiasing: true
		smooth: true
		opacity: (dragProgress < 50) ? 0.5 : 1

		ShapePath
		{
			id: shape_path
			strokeWidth: 3
			strokeColor: control.Material.accent
			startX: 24
			startY: 16
			fillColor: "transparent"

			PathAngleArc
			{
				centerX: 16
				centerY: 16
				radiusX: 8
				radiusY: 8
				startAngle: -20
				sweepAngle: (dragProgress < 50) ? (dragProgress * 280) / 50 : 280
			}
		}

		rotation:
		{
			 if (dragProgress >= 100)
				 return rotation;
			 if (dragProgress >= 50 && dragProgress <= 100)
				return ((dragProgress - 50) * 200) / 50;
			 return 0;
		}
	} */

}
