import QtQuick
import QtTest
import "../../qml/components" as Effects

TestCase {
    id: testCase
    name: "Bonsai"
    when: windowShown
    visible: true
    width: 800
    height: 700

    QtObject {
        id: settingsStore
        property bool motionEnabled: true
        property real decorationOpacity: 1
        property real objectDensity: 0.5
    }

    Component {
        id: sceneComponent
        Effects.BonsaiScene {
            width: 600
            height: 600
            frameCover: true
            coverRect: Qt.rect(180, 200, 240, 240)
        }
    }

    SignalSpy { id: paintSpy; signalName: "painted" }

    function init() {
        settingsStore.motionEnabled = true;
        paintSpy.target = null;
        paintSpy.clear();
    }

    function test_rootsFollowBothCoverEdges_data() {
        return [
            { tag: "small-phone", size: 100 },
            { tag: "phone", size: 180 },
            { tag: "desktop", size: 340 },
            { tag: "large", size: 410 }
        ];
    }

    function test_rootsFollowBothCoverEdges(data) {
        const rect = Qt.rect((600 - data.size) / 2, 220, data.size, data.size);
        const scene = createTemporaryObject(sceneComponent, testCase, { coverRect: rect });
        const left = findChild(scene, "bonsaiLeftRoot");
        const right = findChild(scene, "bonsaiRightRoot");
        verify(left !== null && right !== null);
        fuzzyCompare(left.x + left.width / 2, rect.x, 0.001);
        fuzzyCompare(right.x + right.width / 2, rect.x + rect.width, 0.001);
        verify(left.height >= rect.height);
        verify(scene.treeY < rect.y);
        verify(scene.treeY + scene.treeHeight >= rect.y);
        scene.coverRect = Qt.rect(rect.x + 20, rect.y + 10, rect.width, rect.height);
        fuzzyCompare(left.x + left.width / 2, rect.x + 20, 0.001);
    }

    function test_bassKicksRootsThenTheySettle() {
        const scene = createTemporaryObject(sceneComponent, testCase);
        scene.advance(0.05);
        compare(scene.rootBend, 0);
        scene.bass = 1.3;
        scene.pulse = 1;
        let largest = 0;
        for (let frame = 0; frame < 20; ++frame) {
            scene.advance(1 / 60);
            largest = Math.max(largest, Math.abs(scene.rootBend));
        }
        verify(largest > 1, "A strong bass onset must produce visible root movement.");
        verify(Math.abs(scene.rootBend - scene.rootTip) > 0.02, "Root tips should follow with a delay.");
        scene.bass = 0;
        scene.pulse = 0;
        for (let frame = 0; frame < 300; ++frame)
            scene.advance(1 / 60);
        verify(Math.abs(scene.rootBend) < 0.01);
        verify(Math.abs(scene.rootTip) < 0.01);
    }

    function test_motionOffAndPauseStopTheRoots() {
        const scene = createTemporaryObject(sceneComponent, testCase, { bass: 1.4, pulse: 1 });
        scene.advance(0.06);
        verify(scene.rootBend > 0);
        settingsStore.motionEnabled = false;
        scene.advance(0.06);
        compare(scene.rootBend, 0);
        compare(scene.rootTip, 0);
        compare(scene.tremor, 0);
        settingsStore.motionEnabled = true;
        scene.advance(0.06);
        verify(scene.rootBend > 0);
        scene.animated = false;
        compare(scene.rootBend, 0);
        compare(scene.rootFollow, 0);
        compare(scene.rootTip, 0);
        compare(scene.tremor, 0);
    }

    function test_longFramesStayBounded() {
        const scene = createTemporaryObject(sceneComponent, testCase, { bass: 1.5, pulse: 1.5 });
        for (let frame = 0; frame < 20; ++frame)
            scene.advance(5);
        verify(isFinite(scene.rootBend));
        verify(Math.abs(scene.rootBend) <= 7.2);
        verify(isFinite(scene.rootTip));
    }

    function test_bassDoesNotRepaintTheCanopy() {
        const scene = createTemporaryObject(sceneComponent, testCase);
        paintSpy.target = findChild(scene, "bonsaiCrown");
        verify(paintSpy.valid);
        tryVerify(() => paintSpy.count > 0);
        wait(180);
        const paints = paintSpy.count;
        scene.bass = 1.4;
        scene.pulse = 1;
        for (let frame = 0; frame < 120; ++frame) {
            scene.phase += 1 / 60;
            scene.advance(1 / 60);
        }
        wait(180);
        compare(paintSpy.count, paints);
    }
}
