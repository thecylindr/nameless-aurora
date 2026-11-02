/****************************************************************************
** Meta object code from reading C++ file 'playbackcontroller.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.12.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../src/core/playbackcontroller.h"
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
        "positionChanged",
        "durationChanged",
        "volumeChanged",
        "lastErrorChanged",
        "playIndex",
        "index",
        "playTrackId",
        "id",
        "toggle",
        "next",
        "previous",
        "seek",
        "position",
        "currentIndex",
        "currentTrackId",
        "currentTitle",
        "currentArtist",
        "currentFormat",
        "currentCoverPath",
        "hasTrack",
        "playing",
        "duration",
        "volume",
        "lastError"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'currentTrackChanged'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'playingChanged'
        QtMocHelpers::SignalData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'positionChanged'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'durationChanged'
        QtMocHelpers::SignalData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'volumeChanged'
        QtMocHelpers::SignalData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'lastErrorChanged'
        QtMocHelpers::SignalData<void()>(7, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'playIndex'
        QtMocHelpers::MethodData<void(int)>(8, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 9 },
        }}),
        // Method 'playTrackId'
        QtMocHelpers::MethodData<void(int)>(10, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 11 },
        }}),
        // Method 'toggle'
        QtMocHelpers::MethodData<void()>(12, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'next'
        QtMocHelpers::MethodData<void()>(13, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'previous'
        QtMocHelpers::MethodData<void()>(14, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'seek'
        QtMocHelpers::MethodData<void(qint64)>(15, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::LongLong, 16 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'currentIndex'
        QtMocHelpers::PropertyData<int>(17, QMetaType::Int, QMC::DefaultPropertyFlags, 0),
        // property 'currentTrackId'
        QtMocHelpers::PropertyData<int>(18, QMetaType::Int, QMC::DefaultPropertyFlags, 0),
        // property 'currentTitle'
        QtMocHelpers::PropertyData<QString>(19, QMetaType::QString, QMC::DefaultPropertyFlags, 0),
        // property 'currentArtist'
        QtMocHelpers::PropertyData<QString>(20, QMetaType::QString, QMC::DefaultPropertyFlags, 0),
        // property 'currentFormat'
        QtMocHelpers::PropertyData<QString>(21, QMetaType::QString, QMC::DefaultPropertyFlags, 0),
        // property 'currentCoverPath'
        QtMocHelpers::PropertyData<QString>(22, QMetaType::QString, QMC::DefaultPropertyFlags, 0),
        // property 'hasTrack'
        QtMocHelpers::PropertyData<bool>(23, QMetaType::Bool, QMC::DefaultPropertyFlags, 0),
        // property 'playing'
        QtMocHelpers::PropertyData<bool>(24, QMetaType::Bool, QMC::DefaultPropertyFlags, 1),
        // property 'position'
        QtMocHelpers::PropertyData<qint64>(16, QMetaType::LongLong, QMC::DefaultPropertyFlags, 2),
        // property 'duration'
        QtMocHelpers::PropertyData<qint64>(25, QMetaType::LongLong, QMC::DefaultPropertyFlags, 3),
        // property 'volume'
        QtMocHelpers::PropertyData<double>(26, QMetaType::Double, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 4),
        // property 'lastError'
        QtMocHelpers::PropertyData<QString>(27, QMetaType::QString, QMC::DefaultPropertyFlags, 5),
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
        case 2: _t->positionChanged(); break;
        case 3: _t->durationChanged(); break;
        case 4: _t->volumeChanged(); break;
        case 5: _t->lastErrorChanged(); break;
        case 6: _t->playIndex((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 7: _t->playTrackId((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 8: _t->toggle(); break;
        case 9: _t->next(); break;
        case 10: _t->previous(); break;
        case 11: _t->seek((*reinterpret_cast<std::add_pointer_t<qint64>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (PlaybackController::*)()>(_a, &PlaybackController::currentTrackChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (PlaybackController::*)()>(_a, &PlaybackController::playingChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (PlaybackController::*)()>(_a, &PlaybackController::positionChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (PlaybackController::*)()>(_a, &PlaybackController::durationChanged, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (PlaybackController::*)()>(_a, &PlaybackController::volumeChanged, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (PlaybackController::*)()>(_a, &PlaybackController::lastErrorChanged, 5))
            return;
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<int*>(_v) = _t->currentIndex(); break;
        case 1: *reinterpret_cast<int*>(_v) = _t->currentTrackId(); break;
        case 2: *reinterpret_cast<QString*>(_v) = _t->currentTitle(); break;
        case 3: *reinterpret_cast<QString*>(_v) = _t->currentArtist(); break;
        case 4: *reinterpret_cast<QString*>(_v) = _t->currentFormat(); break;
        case 5: *reinterpret_cast<QString*>(_v) = _t->currentCoverPath(); break;
        case 6: *reinterpret_cast<bool*>(_v) = _t->hasTrack(); break;
        case 7: *reinterpret_cast<bool*>(_v) = _t->playing(); break;
        case 8: *reinterpret_cast<qint64*>(_v) = _t->position(); break;
        case 9: *reinterpret_cast<qint64*>(_v) = _t->duration(); break;
        case 10: *reinterpret_cast<double*>(_v) = _t->volume(); break;
        case 11: *reinterpret_cast<QString*>(_v) = _t->lastError(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 10: _t->setVolume(*reinterpret_cast<double*>(_v)); break;
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
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 12)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 12;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
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
void PlaybackController::positionChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void PlaybackController::durationChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void PlaybackController::volumeChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void PlaybackController::lastErrorChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}
QT_WARNING_POP
