import QtQuick 2.15
import QtQuick.Controls 2.15

Loader {
	id: _loader

	property QFetchLoaderGroup group: null

	default property Component _sourceComponent: null
	sourceComponent: _sourceComponent

	asynchronous: group && group.showPlaceholders

	onGroupChanged: {
		if (group && group.parentItem)
			group.parentItem.Component.destruction.connect(_loader._stopLoading)
	}

	Component.onCompleted: if (group) group.add(_loader)

	onLoaded: if (group) group.remove(_loader)
	Component.onDestruction: if (group) group.remove(_loader)
	ListView.onPooled: if (group) group.remove(_loader)

	function _stopLoading() {
		if (_loader)
			_loader.active = false
	}
}

