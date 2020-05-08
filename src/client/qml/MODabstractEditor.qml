import QtQuick 2.12
import QtQuick.Controls 2.14

Item {
	id: control

	signal save(var jsondata)

	property var jsonData: null
	property var storageData: null

}
