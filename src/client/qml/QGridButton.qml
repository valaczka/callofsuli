import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.14
import "Style"

QButton {
	id: btn

	property bool fillWidth: false
	property int topMargin: 10

	Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
	Layout.columnSpan: parent.columns
	Layout.fillWidth: fillWidth
	Layout.topMargin: topMargin

}
