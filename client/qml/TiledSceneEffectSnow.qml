import QtQuick
import QtQuick.Particles

Item {
	id: control

	implicitWidth: 800
	implicitHeight: 800

	ParticleSystem { id: sys }

	ImageParticle {
		anchors.fill: parent
		id: particles
		system: sys
		source: "qrc:///particleresources/fuzzydot.png"
	}

	Emitter {
		system: sys
		emitRate: 130 * Math.max(0.8, width/800)
		lifeSpan: 5000
		velocity: AngleDirection {angle: 90; magnitude: 100 ; magnitudeVariation: 20; angleVariation: 40}
		acceleration: PointDirection { y: 20 }
		size: 4
		sizeVariation: 2
		anchors.fill: parent
	}
}
