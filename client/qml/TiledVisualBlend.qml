import QtQuick
import QtQuick.Controls
import Qt5Compat.GraphicalEffects
import CallOfSuli
import QtQuick.Shapes


Item {
	id: root

	property TiledSceneImpl scene: parent && (parent instanceof TiledSceneImpl) ? parent : null
	property bool modeUpper: false

	property real sourceZ: z
	readonly property Item current: _lastBlend
	readonly property real currentZ: _lastZ

	implicitHeight: 200
	implicitWidth: 200

	property var blendTargets: []
	property Item _lastBlend: null
	property real _lastZ: z

	onSourceZChanged: reloadBlends()
	onSceneChanged: reloadBlends()

	Item {
		id: _blank
		anchors.fill: parent
		visible: false
	}


	Component {
		id: _cmpBlend

		Blend {
			id: _blendItem

			width: root.width
			height: root.height

			visible: false

			required property Item foregroundItem

			foregroundSource: ShaderEffectSource {
				visible: true
				width: root.width
				height: root.height
				sourceItem: _blendItem.foregroundItem
				sourceRect: Qt.rect(root.x, root.y, root.width, root.height)
				live: true
			}

			mode: "normal"
		}
	}


	property var _items: []

	function reloadBlends() {
		for (let i=0; i<_items.length; ++i) {
			_items[i].destroy()
		}

		_items = []
		blendTargets = []

		if (!scene)
			return


		let tg = scene.visualItems()

		if (!tg)
			return

		let lz = _lastZ

		for (let i=0; i<tg.length; ++i) {
			let item = tg[i]
			if (!modeUpper && item.z <= root.sourceZ) {
				blendTargets.push(item)
				lz = item.z
			} else if (modeUpper && item.z > root.sourceZ) {
				blendTargets.push(item)
				lz = item.z
			}
		}

		_lastZ = lz

		if (blendTargets == null || blendTargets == undefined || blendTargets.length == 0) {
			_lastBlend = null
			return
		}

		for (let i=0; i<blendTargets.length; ++i) {
			let o = _cmpBlend.createObject(root, {
									   source: i==0 ? _blank : _items[i-1],
									   foregroundItem: blendTargets[i]
								   })
			if (!o) {
				console.error("Object creation error")
				break
			}

			_items.push(o)
		}


		if (_items.length > 0)
			_lastBlend = _items[_items.length-1]
		else
			_lastBlend = null
	}


}
