import QtQuick
import QtQuick.Controls

Item {
	id: _group

	property var loaders: []
	property Item parentItem: parent

	readonly property int size: loaders.length
	property bool showPlaceholders: true

	signal allLoadersLoaded()

	function add(loader) {
		loaders.push(loader)
	}

	function remove(loader) {
		let idx = loaders.indexOf(loader)
		if (idx != -1) {
			loaders.splice(idx, 1)
			if (loaders.length == 0) {
				showPlaceholders = false
				allLoadersLoaded()
			}
		}
	}
}

