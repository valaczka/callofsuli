import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.14
import "Style"
import "JScript.js" as JS


GridLayout {
	id: item

	property real requiredWidth: 800

	property bool watchModification: false
	property bool modified: false
	property bool acceptable: true

	property int panelPaddingLeft: CosStyle.panelPaddingLeft
	property int panelPaddingRight: CosStyle.panelPaddingRight

	signal accepted()

	width: parent.width-panelPaddingLeft-panelPaddingRight
	x: panelPaddingLeft

	columns: item.width < item.requiredWidth ? 1 : 2
	columnSpacing: 5
	rowSpacing: columns > 1 ? 5 : 0

	function accept() {
		item.accepted()
	}
}

