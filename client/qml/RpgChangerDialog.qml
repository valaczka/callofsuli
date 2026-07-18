import QtQuick
import QtQuick.Controls
import Qt5Compat.GraphicalEffects
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial


RpgChangerImpl {
	id: root

	anchors.fill: parent

	implicitWidth: panel.implicitWidth
	implicitHeight: panel.implicitHeight


	RpgDialog {
		id: panel
		anchors.fill: parent

		active: root.active

		onCloseRequest: root.close()

		Row {
			height: parent.height

			readonly property real itemWidth: (parent.width-(4*spacing)-(2*_sep.width))/3

			spacing: 5

			RpgChangerComponent {
				id: _cmpWeapon
				width: parent.itemWidth
				height: parent.height
				changer: root
				type: "weapon"
			}

			Qaterial.VerticalLineSeparator {
				id: _sep
				height: parent.height * 0.9
				anchors.verticalCenter: parent.verticalCenter
			}

			RpgChangerComponent {
				id: _cmpDefender
				width: parent.itemWidth
				height: parent.height
				changer: root
				type: "defender"
			}

			Qaterial.VerticalLineSeparator {
				height: parent.height * 0.9
				anchors.verticalCenter: parent.verticalCenter
			}

			RpgChangerComponent {
				id: _cmpUtility
				width: parent.itemWidth
				height: parent.height
				changer: root
				type: "utility"
			}

		}
	}

	onActiveChanged: {
		if (active) {
			_cmpWeapon.reset()
			_cmpDefender.reset()
			_cmpUtility.reset()
		}
	}


}
