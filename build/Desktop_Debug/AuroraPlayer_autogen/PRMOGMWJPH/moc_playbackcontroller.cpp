/****************************************************************************
** Meta object code from reading C++ file 'playbackcontroller.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.12.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../src/core/playbackcontroller.h"
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
        "streamRequested",
        "",
        "QVariantMap",
        "candidate",
        "streamQueueExhausted",
        "currentTrackChanged",
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
        "playSelection",
        "QVariantList",
        "ids",
        "firstId",
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
        "audioLevels",
        "audioEnergy",
        "bassEnergy",
        "beatPulse",
        "beatSerial"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'streamRequested'
        QtMocHelpers::SignalData<void(const QVariantMap &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
        // Signal 'streamQueueExhausted'
        QtMocHelpers::SignalData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'currentTrackChanged'
        QtMocHelpers::SignalData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'playingChanged'
        QtMocHelpers::SignalData<void()>(7, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'streamLoadingChanged'
        QtMocHelpers::SignalData<void()>(8, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'positionChanged'
        QtMocHelpers::SignalData<void()>(9, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'durationChanged'
        QtMocHelpers::SignalData<void()>(10, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'volumeChanged'
        QtMocHelpers::SignalData<void()>(11, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'lastErrorChanged'
        QtMocHelpers::SignalData<void()>(12, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'modesChanged'
        QtMocHelpers::SignalData<void()>(13, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'queueChanged'
        QtMocHelpers::SignalData<void()>(14, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'audioLevelsChanged'
        QtMocHelpers::SignalData<void()>(15, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'audioReactivityChanged'
        QtMocHelpers::SignalData<void()>(16, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'seeked'
        QtMocHelpers::SignalData<void(qint64)>(17, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::LongLong, 18 },
        }}),
        // Signal 'seekableChanged'
        QtMocHelpers::SignalData<void()>(19, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'playIndex'
        QtMocHelpers::MethodData<void(int)>(20, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 21 },
        }}),
        // Method 'playTrackId'
        QtMocHelpers::MethodData<void(int)>(22, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 23 },
        }}),
        // Method 'playQueueTrack'
        QtMocHelpers::MethodData<void(int)>(24, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 23 },
        }}),
        // Method 'playMix'
        QtMocHelpers::MethodData<void()>(25, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'playSelection'
        QtMocHelpers::MethodData<void(const QVariantList &, int)>(26, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 27, 28 }, { QMetaType::Int, 29 },
        }}),
        // Method 'playSelection'
        QtMocHelpers::MethodData<void(const QVariantList &)>(26, 2, QMC::AccessPublic | QMC::MethodCloned, QMetaType::Void, {{
            { 0x80000000 | 27, 28 },
        }}),
        // Method 'toggle'
        QtMocHelpers::MethodData<void()>(30, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'play'
        QtMocHelpers::MethodData<void()>(31, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'pause'
        QtMocHelpers::MethodData<void()>(32, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'stop'
        QtMocHelpers::MethodData<void()>(33, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'next'
        QtMocHelpers::MethodData<void()>(34, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'previous'
        QtMocHelpers::MethodData<void()>(35, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'seek'
        QtMocHelpers::MethodData<void(qint64)>(36, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::LongLong, 18 },
        }}),
        // Method 'toggleFavorite'
        QtMocHelpers::MethodData<void()>(37, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'enqueue'
        QtMocHelpers::MethodData<void(int)>(38, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 23 },
        }}),
        // Method 'removeFromQueue'
        QtMocHelpers::MethodData<void(int)>(39, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 23 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'equalizer'
        QtMocHelpers::PropertyData<AudioEqualizer*>(40, 0x80000000 | 41, QMC::DefaultPropertyFlags | QMC::EnumOrFlag | QMC::Constant),
        // property 'currentIndex'
        QtMocHelpers::PropertyData<int>(42, QMetaType::Int, QMC::DefaultPropertyFlags, 2),
        // property 'currentTrackId'
        QtMocHelpers::PropertyData<int>(43, QMetaType::Int, QMC::DefaultPropertyFlags, 2),
        // property 'currentTitle'
        QtMocHelpers::PropertyData<QString>(44, QMetaType::QString, QMC::DefaultPropertyFlags, 2),
        // property 'currentArtist'
        QtMocHelpers::PropertyData<QString>(45, QMetaType::QString, QMC::DefaultPropertyFlags, 2),
        // property 'currentFormat'
        QtMocHelpers::PropertyData<QString>(46, QMetaType::QString, QMC::DefaultPropertyFlags, 2),
        // property 'currentCoverPath'
        QtMocHelpers::PropertyData<QString>(47, QMetaType::QString, QMC::DefaultPropertyFlags, 2),
        // property 'artworkUrl'
        QtMocHelpers::PropertyData<QUrl>(48, 0x80000000 | 49, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 2),
        // property 'atmosphereColor'
        QtMocHelpers::PropertyData<QColor>(50, 0x80000000 | 51, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 2),
        // property 'atmosphereSecondary'
        QtMocHelpers::PropertyData<QColor>(52, 0x80000000 | 51, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 2),
        // property 'atmosphereShape'
        QtMocHelpers::PropertyData<double>(53, QMetaType::Double, QMC::DefaultPropertyFlags, 2),
        // property 'favorite'
        QtMocHelpers::PropertyData<bool>(54, QMetaType::Bool, QMC::DefaultPropertyFlags, 2),
        // property 'hasTrack'
        QtMocHelpers::PropertyData<bool>(55, QMetaType::Bool, QMC::DefaultPropertyFlags, 2),
        // property 'streaming'
        QtMocHelpers::PropertyData<bool>(56, QMetaType::Bool, QMC::DefaultPropertyFlags, 2),
        // property 'streamLoading'
        QtMocHelpers::PropertyData<bool>(57, QMetaType::Bool, QMC::DefaultPropertyFlags, 4),
        // property 'streamKey'
        QtMocHelpers::PropertyData<QString>(58, QMetaType::QString, QMC::DefaultPropertyFlags, 2),
        // property 'seekable'
        QtMocHelpers::PropertyData<bool>(59, QMetaType::Bool, QMC::DefaultPropertyFlags, 14),
        // property 'playing'
        QtMocHelpers::PropertyData<bool>(60, QMetaType::Bool, QMC::DefaultPropertyFlags, 3),
        // property 'position'
        QtMocHelpers::PropertyData<qint64>(18, QMetaType::LongLong, QMC::DefaultPropertyFlags, 5),
        // property 'duration'
        QtMocHelpers::PropertyData<qint64>(61, QMetaType::LongLong, QMC::DefaultPropertyFlags, 6),
        // property 'volume'
        QtMocHelpers::PropertyData<double>(62, QMetaType::Double, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 7),
        // property 'lastError'
        QtMocHelpers::PropertyData<QString>(63, QMetaType::QString, QMC::DefaultPropertyFlags, 8),
        // property 'shuffle'
        QtMocHelpers::PropertyData<bool>(64, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 9),
        // property 'repeatMode'
        QtMocHelpers::PropertyData<int>(65, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 9),
        // property 'queue'
        QtMocHelpers::PropertyData<QVariantList>(66, 0x80000000 | 27, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 10),
        // property 'audioLevels'
        QtMocHelpers::PropertyData<QVariantList>(67, 0x80000000 | 27, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 11),
        // property 'audioEnergy'
        QtMocHelpers::PropertyData<double>(68, QMetaType::Double, QMC::DefaultPropertyFlags, 12),
        // property 'bassEnergy'
        QtMocHelpers::PropertyData<double>(69, QMetaType::Double, QMC::DefaultPropertyFlags, 12),
        // property 'beatPulse'
        QtMocHelpers::PropertyData<double>(70, QMetaType::Double, QMC::DefaultPropertyFlags, 12),
        // property 'beatSerial'
        QtMocHelpers::PropertyData<quint64>(71, QMetaType::ULongLong, QMC::DefaultPropertyFlags, 12),
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
        case 0: _t->streamRequested((*reinterpret_cast<std::add_pointer_t<QVariantMap>>(_a[1]))); break;
        case 1: _t->streamQueueExhausted(); break;
        case 2: _t->currentTrackChanged(); break;
        case 3: _t->playingChanged(); break;
        case 4: _t->streamLoadingChanged(); break;
        case 5: _t->positionChanged(); break;
        case 6: _t->durationChanged(); break;
        case 7: _t->volumeChanged(); break;
        case 8: _t->lastErrorChanged(); break;
        case 9: _t->modesChanged(); break;
        case 10: _t->queueChanged(); break;
        case 11: _t->audioLevelsChanged(); break;
        case 12: _t->audioReactivityChanged(); break;
        case 13: _t->seeked((*reinterpret_cast<std::add_pointer_t<qint64>>(_a[1]))); break;
        case 14: _t->seekableChanged(); break;
        case 15: _t->playIndex((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 16: _t->playTrackId((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 17: _t->playQueueTrack((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 18: _t->playMix(); break;
        case 19: _t->playSelection((*reinterpret_cast<std::add_pointer_t<QVariantList>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2]))); break;
        case 20: _t->playSelection((*reinterpret_cast<std::add_pointer_t<QVariantList>>(_a[1]))); break;
        case 21: _t->toggle(); break;
        case 22: _t->play(); break;
        case 23: _t->pause(); break;
        case 24: _t->stop(); break;
        case 25: _t->next(); break;
        case 26: _t->previous(); break;
        case 27: _t->seek((*reinterpret_cast<std::add_pointer_t<qint64>>(_a[1]))); break;
        case 28: _t->toggleFavorite(); break;
        case 29: _t->enqueue((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 30: _t->removeFromQueue((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (PlaybackController::*)(const QVariantMap & )>(_a, &PlaybackController::streamRequested, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (PlaybackController::*)()>(_a, &PlaybackController::streamQueueExhausted, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (PlaybackController::*)()>(_a, &PlaybackController::currentTrackChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (PlaybackController::*)()>(_a, &PlaybackController::playingChanged, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (PlaybackController::*)()>(_a, &PlaybackController::streamLoadingChanged, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (PlaybackController::*)()>(_a, &PlaybackController::positionChanged, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (PlaybackController::*)()>(_a, &PlaybackController::durationChanged, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (PlaybackController::*)()>(_a, &PlaybackController::volumeChanged, 7))
            return;
        if (QtMocHelpers::indexOfMethod<void (PlaybackController::*)()>(_a, &PlaybackController::lastErrorChanged, 8))
            return;
        if (QtMocHelpers::indexOfMethod<void (PlaybackController::*)()>(_a, &PlaybackController::modesChanged, 9))
            return;
        if (QtMocHelpers::indexOfMethod<void (PlaybackController::*)()>(_a, &PlaybackController::queueChanged, 10))
            return;
        if (QtMocHelpers::indexOfMethod<void (PlaybackController::*)()>(_a, &PlaybackController::audioLevelsChanged, 11))
            return;
        if (QtMocHelpers::indexOfMethod<void (PlaybackController::*)()>(_a, &PlaybackController::audioReactivityChanged, 12))
            return;
        if (QtMocHelpers::indexOfMethod<void (PlaybackController::*)(qint64 )>(_a, &PlaybackController::seeked, 13))
            return;
        if (QtMocHelpers::indexOfMethod<void (PlaybackController::*)()>(_a, &PlaybackController::seekableChanged, 14))
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
        if (_id < 31)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 31;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 31)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 31;
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
void PlaybackController::streamRequested(const QVariantMap & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void PlaybackController::streamQueueExhausted()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void PlaybackController::currentTrackChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void PlaybackController::playingChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void PlaybackController::streamLoadingChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void PlaybackController::positionChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void PlaybackController::durationChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void PlaybackController::volumeChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}

// SIGNAL 8
void PlaybackController::lastErrorChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 8, nullptr);
}

// SIGNAL 9
void PlaybackController::modesChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 9, nullptr);
}

// SIGNAL 10
void PlaybackController::queueChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 10, nullptr);
}

// SIGNAL 11
void PlaybackController::audioLevelsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 11, nullptr);
}

// SIGNAL 12
void PlaybackController::audioReactivityChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 12, nullptr);
}

// SIGNAL 13
void PlaybackController::seeked(qint64 _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 13, nullptr, _t1);
}

// SIGNAL 14
void PlaybackController::seekableChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 14, nullptr);
}
QT_WARNING_POP
