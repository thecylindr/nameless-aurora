/****************************************************************************
** Meta object code from reading C++ file 'playbackcontroller.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.12.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/core/playbackcontroller.h"
#include <QtNetwork/QSslError>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'playbackcontroller.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.12.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN18PlaybackControllerE_t {};
} // unnamed namespace

template <> constexpr inline auto PlaybackController::qt_create_metaobjectdata<qt_meta_tag_ZN18PlaybackControllerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "PlaybackController",
        "currentTrackChanged",
        "",
        "playingChanged",
        "streamLoadingChanged",
        "positionChanged",
        "durationChanged",
        "volumeChanged",
        "lastErrorChanged",
        "modesChanged",
        "queueChanged",
        "audioLevelsChanged",
        "audioReactivityChanged",
        "seeked",
        "position",
        "seekableChanged",
        "playIndex",
        "index",
        "playTrackId",
        "id",
        "playQueueTrack",
        "playMix",
        "toggle",
        "play",
        "pause",
        "stop",
        "next",
        "previous",
        "seek",
        "toggleFavorite",
        "enqueue",
        "removeFromQueue",
        "equalizer",
        "AudioEqualizer*",
        "currentIndex",
        "currentTrackId",
        "currentTitle",
        "currentArtist",
        "currentFormat",
        "currentCoverPath",
        "artworkUrl",
        "QUrl",
        "atmosphereColor",
        "QColor",
        "atmosphereSecondary",
        "atmosphereShape",
        "favorite",
        "hasTrack",
        "streaming",
        "streamLoading",
        "streamKey",
        "seekable",
        "playing",
        "duration",
        "volume",
        "lastError",
        "shuffle",
        "repeatMode",
        "queue",
        "QVariantList",
        "audioLevels",
        "audioEnergy",
        "bassEnergy",
        "beatPulse",
        "beatSerial"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'currentTrackChanged'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'playingChanged'
        QtMocHelpers::SignalData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'streamLoadingChanged'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'positionChanged'
        QtMocHelpers::SignalData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'durationChanged'
        QtMocHelpers::SignalData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'volumeChanged'
        QtMocHelpers::SignalData<void()>(7, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'lastErrorChanged'
        QtMocHelpers::SignalData<void()>(8, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'modesChanged'
        QtMocHelpers::SignalData<void()>(9, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'queueChanged'
        QtMocHelpers::SignalData<void()>(10, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'audioLevelsChanged'
        QtMocHelpers::SignalData<void()>(11, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'audioReactivityChanged'
        QtMocHelpers::SignalData<void()>(12, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'seeked'
        QtMocHelpers::SignalData<void(qint64)>(13, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::LongLong, 14 },
        }}),
        // Signal 'seekableChanged'
        QtMocHelpers::SignalData<void()>(15, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'playIndex'
        QtMocHelpers::MethodData<void(int)>(16, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 17 },
        }}),
        // Method 'playTrackId'
        QtMocHelpers::MethodData<void(int)>(18, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 19 },
        }}),
        // Method 'playQueueTrack'
        QtMocHelpers::MethodData<void(int)>(20, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 19 },
        }}),
        // Method 'playMix'
        QtMocHelpers::MethodData<void()>(21, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'toggle'
        QtMocHelpers::MethodData<void()>(22, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'play'
        QtMocHelpers::MethodData<void()>(23, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'pause'
        QtMocHelpers::MethodData<void()>(24, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'stop'
        QtMocHelpers::MethodData<void()>(25, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'next'
        QtMocHelpers::MethodData<void()>(26, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'previous'
        QtMocHelpers::MethodData<void()>(27, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'seek'
        QtMocHelpers::MethodData<void(qint64)>(28, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::LongLong, 14 },
        }}),
        // Method 'toggleFavorite'
        QtMocHelpers::MethodData<void()>(29, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'enqueue'
        QtMocHelpers::MethodData<void(int)>(30, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 19 },
        }}),
        // Method 'removeFromQueue'
        QtMocHelpers::MethodData<void(int)>(31, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 19 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'equalizer'
        QtMocHelpers::PropertyData<AudioEqualizer*>(32, 0x80000000 | 33, QMC::DefaultPropertyFlags | QMC::EnumOrFlag | QMC::Constant),
        // property 'currentIndex'
        QtMocHelpers::PropertyData<int>(34, QMetaType::Int, QMC::DefaultPropertyFlags, 0),
        // property 'currentTrackId'
        QtMocHelpers::PropertyData<int>(35, QMetaType::Int, QMC::DefaultPropertyFlags, 0),
        // property 'currentTitle'
        QtMocHelpers::PropertyData<QString>(36, QMetaType::QString, QMC::DefaultPropertyFlags, 0),
        // property 'currentArtist'
        QtMocHelpers::PropertyData<QString>(37, QMetaType::QString, QMC::DefaultPropertyFlags, 0),
        // property 'currentFormat'
        QtMocHelpers::PropertyData<QString>(38, QMetaType::QString, QMC::DefaultPropertyFlags, 0),
        // property 'currentCoverPath'
        QtMocHelpers::PropertyData<QString>(39, QMetaType::QString, QMC::DefaultPropertyFlags, 0),
        // property 'artworkUrl'
        QtMocHelpers::PropertyData<QUrl>(40, 0x80000000 | 41, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 0),
        // property 'atmosphereColor'
        QtMocHelpers::PropertyData<QColor>(42, 0x80000000 | 43, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 0),
        // property 'atmosphereSecondary'
        QtMocHelpers::PropertyData<QColor>(44, 0x80000000 | 43, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 0),
        // property 'atmosphereShape'
        QtMocHelpers::PropertyData<double>(45, QMetaType::Double, QMC::DefaultPropertyFlags, 0),
        // property 'favorite'
        QtMocHelpers::PropertyData<bool>(46, QMetaType::Bool, QMC::DefaultPropertyFlags, 0),
        // property 'hasTrack'
        QtMocHelpers::PropertyData<bool>(47, QMetaType::Bool, QMC::DefaultPropertyFlags, 0),
        // property 'streaming'
        QtMocHelpers::PropertyData<bool>(48, QMetaType::Bool, QMC::DefaultPropertyFlags, 0),
        // property 'streamLoading'
        QtMocHelpers::PropertyData<bool>(49, QMetaType::Bool, QMC::DefaultPropertyFlags, 2),
        // property 'streamKey'
        QtMocHelpers::PropertyData<QString>(50, QMetaType::QString, QMC::DefaultPropertyFlags, 0),
        // property 'seekable'
        QtMocHelpers::PropertyData<bool>(51, QMetaType::Bool, QMC::DefaultPropertyFlags, 12),
        // property 'playing'
        QtMocHelpers::PropertyData<bool>(52, QMetaType::Bool, QMC::DefaultPropertyFlags, 1),
        // property 'position'
        QtMocHelpers::PropertyData<qint64>(14, QMetaType::LongLong, QMC::DefaultPropertyFlags, 3),
        // property 'duration'
        QtMocHelpers::PropertyData<qint64>(53, QMetaType::LongLong, QMC::DefaultPropertyFlags, 4),
        // property 'volume'
        QtMocHelpers::PropertyData<double>(54, QMetaType::Double, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 5),
        // property 'lastError'
        QtMocHelpers::PropertyData<QString>(55, QMetaType::QString, QMC::DefaultPropertyFlags, 6),
        // property 'shuffle'
        QtMocHelpers::PropertyData<bool>(56, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 7),
        // property 'repeatMode'
        QtMocHelpers::PropertyData<int>(57, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 7),
        // property 'queue'
        QtMocHelpers::PropertyData<QVariantList>(58, 0x80000000 | 59, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 8),
        // property 'audioLevels'
        QtMocHelpers::PropertyData<QVariantList>(60, 0x80000000 | 59, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 9),
        // property 'audioEnergy'
        QtMocHelpers::PropertyData<double>(61, QMetaType::Double, QMC::DefaultPropertyFlags, 10),
        // property 'bassEnergy'
        QtMocHelpers::PropertyData<double>(62, QMetaType::Double, QMC::DefaultPropertyFlags, 10),
        // property 'beatPulse'
        QtMocHelpers::PropertyData<double>(63, QMetaType::Double, QMC::DefaultPropertyFlags, 10),
        // property 'beatSerial'
        QtMocHelpers::PropertyData<quint64>(64, QMetaType::ULongLong, QMC::DefaultPropertyFlags, 10),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<PlaybackController, qt_meta_tag_ZN18PlaybackControllerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject PlaybackController::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN18PlaybackControllerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN18PlaybackControllerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN18PlaybackControllerE_t>.metaTypes,
    nullptr
} };

void PlaybackController::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<PlaybackController *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->currentTrackChanged(); break;
        case 1: _t->playingChanged(); break;
        case 2: _t->streamLoadingChanged(); break;
        case 3: _t->positionChanged(); break;
        case 4: _t->durationChanged(); break;
        case 5: _t->volumeChanged(); break;
        case 6: _t->lastErrorChanged(); break;
        case 7: _t->modesChanged(); break;
        case 8: _t->queueChanged(); break;
        case 9: _t->audioLevelsChanged(); break;
        case 10: _t->audioReactivityChanged(); break;
        case 11: _t->seeked((*reinterpret_cast<std::add_pointer_t<qint64>>(_a[1]))); break;
        case 12: _t->seekableChanged(); break;
        case 13: _t->playIndex((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 14: _t->playTrackId((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 15: _t->playQueueTrack((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 16: _t->playMix(); break;
        case 17: _t->toggle(); break;
        case 18: _t->play(); break;
        case 19: _t->pause(); break;
        case 20: _t->stop(); break;
        case 21: _t->next(); break;
        case 22: _t->previous(); break;
        case 23: _t->seek((*reinterpret_cast<std::add_pointer_t<qint64>>(_a[1]))); break;
        case 24: _t->toggleFavorite(); break;
        case 25: _t->enqueue((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 26: _t->removeFromQueue((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (PlaybackController::*)()>(_a, &PlaybackController::currentTrackChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (PlaybackController::*)()>(_a, &PlaybackController::playingChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (PlaybackController::*)()>(_a, &PlaybackController::streamLoadingChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (PlaybackController::*)()>(_a, &PlaybackController::positionChanged, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (PlaybackController::*)()>(_a, &PlaybackController::durationChanged, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (PlaybackController::*)()>(_a, &PlaybackController::volumeChanged, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (PlaybackController::*)()>(_a, &PlaybackController::lastErrorChanged, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (PlaybackController::*)()>(_a, &PlaybackController::modesChanged, 7))
            return;
        if (QtMocHelpers::indexOfMethod<void (PlaybackController::*)()>(_a, &PlaybackController::queueChanged, 8))
            return;
        if (QtMocHelpers::indexOfMethod<void (PlaybackController::*)()>(_a, &PlaybackController::audioLevelsChanged, 9))
            return;
        if (QtMocHelpers::indexOfMethod<void (PlaybackController::*)()>(_a, &PlaybackController::audioReactivityChanged, 10))
            return;
        if (QtMocHelpers::indexOfMethod<void (PlaybackController::*)(qint64 )>(_a, &PlaybackController::seeked, 11))
            return;
        if (QtMocHelpers::indexOfMethod<void (PlaybackController::*)()>(_a, &PlaybackController::seekableChanged, 12))
            return;
    }
    if (_c == QMetaObject::RegisterPropertyMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 0:
            *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< AudioEqualizer* >(); break;
        }
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<AudioEqualizer**>(_v) = _t->equalizer(); break;
        case 1: *reinterpret_cast<int*>(_v) = _t->currentIndex(); break;
        case 2: *reinterpret_cast<int*>(_v) = _t->currentTrackId(); break;
        case 3: *reinterpret_cast<QString*>(_v) = _t->currentTitle(); break;
        case 4: *reinterpret_cast<QString*>(_v) = _t->currentArtist(); break;
        case 5: *reinterpret_cast<QString*>(_v) = _t->currentFormat(); break;
        case 6: *reinterpret_cast<QString*>(_v) = _t->currentCoverPath(); break;
        case 7: *reinterpret_cast<QUrl*>(_v) = _t->artworkUrl(); break;
        case 8: *reinterpret_cast<QColor*>(_v) = _t->atmosphereColor(); break;
        case 9: *reinterpret_cast<QColor*>(_v) = _t->atmosphereSecondary(); break;
        case 10: *reinterpret_cast<double*>(_v) = _t->atmosphereShape(); break;
        case 11: *reinterpret_cast<bool*>(_v) = _t->favorite(); break;
        case 12: *reinterpret_cast<bool*>(_v) = _t->hasTrack(); break;
        case 13: *reinterpret_cast<bool*>(_v) = _t->streaming(); break;
        case 14: *reinterpret_cast<bool*>(_v) = _t->streamLoading(); break;
        case 15: *reinterpret_cast<QString*>(_v) = _t->streamKey(); break;
        case 16: *reinterpret_cast<bool*>(_v) = _t->seekable(); break;
        case 17: *reinterpret_cast<bool*>(_v) = _t->playing(); break;
        case 18: *reinterpret_cast<qint64*>(_v) = _t->position(); break;
        case 19: *reinterpret_cast<qint64*>(_v) = _t->duration(); break;
        case 20: *reinterpret_cast<double*>(_v) = _t->volume(); break;
        case 21: *reinterpret_cast<QString*>(_v) = _t->lastError(); break;
        case 22: *reinterpret_cast<bool*>(_v) = _t->shuffle(); break;
        case 23: *reinterpret_cast<int*>(_v) = _t->repeatMode(); break;
        case 24: *reinterpret_cast<QVariantList*>(_v) = _t->queue(); break;
        case 25: *reinterpret_cast<QVariantList*>(_v) = _t->audioLevels(); break;
        case 26: *reinterpret_cast<double*>(_v) = _t->audioEnergy(); break;
        case 27: *reinterpret_cast<double*>(_v) = _t->bassEnergy(); break;
        case 28: *reinterpret_cast<double*>(_v) = _t->beatPulse(); break;
        case 29: *reinterpret_cast<quint64*>(_v) = _t->beatSerial(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 20: _t->setVolume(*reinterpret_cast<double*>(_v)); break;
        case 22: _t->setShuffle(*reinterpret_cast<bool*>(_v)); break;
        case 23: _t->setRepeatMode(*reinterpret_cast<int*>(_v)); break;
        default: break;
        }
    }
}

const QMetaObject *PlaybackController::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PlaybackController::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN18PlaybackControllerE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int PlaybackController::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 27)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 27;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 27)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 27;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 30;
    }
    return _id;
}

// SIGNAL 0
void PlaybackController::currentTrackChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void PlaybackController::playingChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void PlaybackController::streamLoadingChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void PlaybackController::positionChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void PlaybackController::durationChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void PlaybackController::volumeChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void PlaybackController::lastErrorChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void PlaybackController::modesChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}

// SIGNAL 8
void PlaybackController::queueChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 8, nullptr);
}

// SIGNAL 9
void PlaybackController::audioLevelsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 9, nullptr);
}

// SIGNAL 10
void PlaybackController::audioReactivityChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 10, nullptr);
}

// SIGNAL 11
void PlaybackController::seeked(qint64 _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 11, nullptr, _t1);
}

// SIGNAL 12
void PlaybackController::seekableChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 12, nullptr);
}
QT_WARNING_POP
