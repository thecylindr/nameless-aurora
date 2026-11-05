import QtQuick
import QtTest
import "../../qml/components" as Effects
import "../../qml/common" as Common
import "../../qml" as Aurora

TestCase {
    id: testCase
    name: "AnimationEffects"
    when: windowShown
    visible: true
    width: 800
    height: 700

    QtObject {
        id: settingsStore
        property bool motionEnabled: true
        property string atmosphereScene: "meteors"
        property real atmosphereIntensity: 0.85
        property real objectDensity: 1
        property real objectScale: 1
        property real decorationOpacity: 1
        property real sphereDeformation: 1
        property string sphereStyle: "ribbons"
        property bool beatEffects: true
        property real reactionStrength: 1.15
        property real effectSpeed: 1
        property string colorScheme: "aurora"
    }
    Aurora.Theme {
        id: theme
    }
    QtObject {
        id: player
        property bool hasTrack: false
        property bool playing: true
        property real audioEnergy: 0.2
        property real bassEnergy: 0.2
        property real beatPulse: 0.1
        property double beatSerial: 0
        signal audioReactivityChanged()
    }

    Item {
        id: viewport
        width: 600
        height: 600
        clip: true
    }

    Component {
        id: surfaceComponent
        Effects.EffectSurface {
            width: 200
            height: 200
            lively: true
            property real elapsed: 0
            onTick: seconds => elapsed += seconds
        }
    }
    Component {
        id: objectsComponent
        Effects.AtmosphereObjects {
            width: 520
            height: 520
            phase: 0
        }
    }
    Component {
        id: progressComponent
        Common.AuroraProgress {
            uiTheme: theme
            width: 180
            indeterminate: true
        }
    }
    Component {
        id: orbComponent
        Effects.AtmosphereOrb {
            width: 1040
            height: 1040
            interactive: false
        }
    }

    function test_softwareSphereStopsAtZeroIntensity() {
        if (GraphicsInfo.api !== GraphicsInfo.Software && GraphicsInfo.api !== GraphicsInfo.Unknown) {
            skip("GPU shader resources are embedded in the application, outside qmltestrunner.");
            return;
        }
        settingsStore.atmosphereScene = "orb";
        const orb = createTemporaryObject(orbComponent, viewport);
        tryCompare(orb, "presented", true);
        orb.intensity = 0;
        compare(orb.motionRunning, false);
        compare(findChild(orb, "auroraOrbShader"), null);
    }

    function init() {
        settingsStore.motionEnabled = true;
        settingsStore.atmosphereScene = "meteors";
        settingsStore.objectDensity = 1;
        settingsStore.atmosphereIntensity = 0.85;
        viewport.visible = true;
        viewport.opacity = 1;
        viewport.y = 0;
    }

    function test_motionSwitchStopsTicks() {
        const surface = createTemporaryObject(surfaceComponent, viewport);
        tryCompare(surface, "motionRunning", true);
        tryVerify(() => surface.elapsed > 0);
        settingsStore.motionEnabled = false;
        tryCompare(surface, "motionRunning", false);
        const elapsed = surface.elapsed;
        wait(180);
        compare(surface.elapsed, elapsed);
        settingsStore.motionEnabled = true;
        tryVerify(() => surface.elapsed > elapsed);
    }

    function test_clippedEffectStopsAndResumes() {
        const surface = createTemporaryObject(surfaceComponent, viewport);
        tryCompare(surface, "motionRunning", true);
        surface.y = viewport.height + 20;
        tryCompare(surface, "presented", false, 1000);
        const elapsed = surface.elapsed;
        wait(180);
        compare(surface.elapsed, elapsed);
        surface.y = 0;
        tryCompare(surface, "motionRunning", true, 1000);
        tryVerify(() => surface.elapsed > elapsed);
    }

    function test_hiddenAncestorStopsTicks() {
        const surface = createTemporaryObject(surfaceComponent, viewport);
        tryCompare(surface, "motionRunning", true);
        viewport.visible = false;
        tryCompare(surface, "motionRunning", false);
        const elapsed = surface.elapsed;
        wait(180);
        compare(surface.elapsed, elapsed);
    }

    function test_transparentAncestorStopsTicks() {
        const surface = createTemporaryObject(surfaceComponent, viewport);
        tryCompare(surface, "motionRunning", true);
        viewport.opacity = 0;
        tryCompare(surface, "presented", false, 1000);
        const elapsed = surface.elapsed;
        wait(180);
        compare(surface.elapsed, elapsed);
    }

    function test_hiddenWindowStopsTicks() {
        const surface = createTemporaryObject(surfaceComponent, viewport);
        tryCompare(surface, "motionRunning", true);
        const window = surface.Window.window;
        const visibility = window.visibility;
        try {
            window.visibility = Window.Hidden;
            tryCompare(surface, "motionRunning", false);
            const elapsed = surface.elapsed;
            wait(180);
            compare(surface.elapsed, elapsed);
        } finally {
            window.visibility = visibility;
        }
        tryCompare(surface, "motionRunning", true);
    }

    function test_meteorsFollowTheirTail_data() {
        return [
            {
                tag: "small",
                width: 200,
                height: 200
            },
            {
                tag: "portrait",
                width: 390,
                height: 760
            },
            {
                tag: "landscape",
                width: 1280,
                height: 720
            },
            {
                tag: "large",
                width: 2048,
                height: 1100
            }
        ];
    }

    function test_meteorsFollowTheirTail(data) {
        const objects = createTemporaryObject(objectsComponent, viewport, {
            width: data.width,
            height: data.height
        });
        const repeater = findChild(objects, "meteorRepeater");
        verify(repeater.count >= 2);
        for (let index = 0; index < repeater.count; ++index) {
            const meteor = repeater.itemAt(index);
            objects.phase = 0;
            // Choose a point well inside this meteor's flight, away from its wrap.
            const offset = (0.35 * meteor.flightFraction - meteor.seed + 1) % 1;
            objects.phase = offset * meteor.cycleDuration;
            verify(meteor.visible);
            const x = meteor.x, y = meteor.y;
            objects.phase += 0.01;
            const dx = meteor.x - x, dy = meteor.y - y;
            verify(dx > 0 && dy > 0);
            fuzzyCompare(dy / dx, Math.tan(meteor.rotation * Math.PI / 180), 0.0001);
        }
    }

    function test_meteorWrapIsInvisible() {
        const objects = createTemporaryObject(objectsComponent, viewport);
        const meteor = findChild(objects, "meteorRepeater").itemAt(0);
        objects.phase = (1 - meteor.seed - 0.0001) * meteor.cycleDuration;
        verify(!meteor.visible);
        objects.phase += meteor.cycleDuration * 0.0002;
        verify(meteor.opacity < 0.01);
    }

    function test_densityAndDetailBoundObjectCount() {
        const objects = createTemporaryObject(objectsComponent, viewport);
        const repeater = findChild(objects, "meteorRepeater");
        const fullCount = repeater.count;
        objects.detail = 0.45;
        verify(repeater.count < fullCount);
        objects.width = 200;
        objects.height = 200;
        verify(repeater.count <= 5);
        settingsStore.objectDensity = 0;
        compare(repeater.count, 0);
        settingsStore.objectDensity = 1;
        verify(repeater.count > 0);
        settingsStore.atmosphereScene = "off";
        compare(repeater.count, 0);
    }

    function test_progressHasStaticIndicatorWithoutMotion() {
        const progress = createTemporaryObject(progressComponent, viewport);
        tryCompare(progress, "motionEnabled", true);
        wait(160);
        settingsStore.motionEnabled = false;
        compare(progress.motionEnabled, false);
        verify(progress.contentItem.children[1].visible);
        const indicator = progress.contentItem.children[1];
        fuzzyCompare(indicator.x, (indicator.parent.width - indicator.width) / 2, 0.001);
        const x = indicator.x;
        wait(180);
        compare(indicator.x, x);
    }
}
