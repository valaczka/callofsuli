import QtQuick
import QtQuick.Particles

Item {
	id: control

	implicitWidth: 800
	implicitHeight: 800

	readonly property real hFactor: Math.max(0.8, Math.min(height/800, 1.))

	ParticleSystem { id: sys }

	ImageParticle {
		anchors.fill: parent
		id: particles
		system: sys
		entryEffect: ImageParticle.None
		source: "qrc:/rpg/common/rain.svg"
		rotation: 15
	}

	Emitter {
		system: sys
		emitRate: 120 * Math.max(0.8, width/800)
		lifeSpan: 10000
		velocity: AngleDirection {angle: 110; magnitude: 800 ; magnitudeVariation: 20; angleVariation: 1}
		acceleration: PointDirection { y: 20 }
		size: 50 * hFactor
		sizeVariation: 40 * hFactor
		width: Math.max(800, parent.width*1.3, parent.height*1.3)
		height: 100
		y: -50
	}
}
