/****************************************************************************
** Meta object code from reading C++ file 'mpris.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.12.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../src/core/integration/mpris.h"
#include <QtNetwork/QSslError>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mpris.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN9MprisRootE_t {};
} // unnamed namespace

template <> constexpr inline auto MprisRoot::qt_create_metaobjectdata<qt_meta_tag_ZN9MprisRootE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "MprisRoot",
        "D-Bus Interface",
        "org.mpris.MediaPlayer2",
        "Raise",
        "",
        "Quit",
        "CanQuit",
        "CanRaise",
        "HasTrackList",
        "Identity",
        "DesktopEntry",
        "SupportedUriSchemes",
        "SupportedMimeTypes"
    };

    QtMocHelpers::UintData qt_methods {
        // Slot 'Raise'
        QtMocHelpers::SlotData<void()>(3, 4, QMC::AccessPublic, QMetaType::Void),
        // Slot 'Quit'
        QtMocHelpers::SlotData<void()>(5, 4, QMC::AccessPublic, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'CanQuit'
        QtMocHelpers::PropertyData<bool>(6, QMetaType::Bool, QMC::DefaultPropertyFlags),
        // property 'CanRaise'
        QtMocHelpers::PropertyData<bool>(7, QMetaType::Bool, QMC::DefaultPropertyFlags),
        // property 'HasTrackList'
        QtMocHelpers::PropertyData<bool>(8, QMetaType::Bool, QMC::DefaultPropertyFlags),
        // property 'Identity'
        QtMocHelpers::PropertyData<QString>(9, QMetaType::QString, QMC::DefaultPropertyFlags),
        // property 'DesktopEntry'
        QtMocHelpers::PropertyData<QString>(10, QMetaType::QString, QMC::DefaultPropertyFlags),
        // property 'SupportedUriSchemes'
        QtMocHelpers::PropertyData<QStringList>(11, QMetaType::QStringList, QMC::DefaultPropertyFlags),
        // property 'SupportedMimeTypes'
        QtMocHelpers::PropertyData<QStringList>(12, QMetaType::QStringList, QMC::DefaultPropertyFlags),
    };
    QtMocHelpers::UintData qt_enums {
    };
    QtMocHelpers::UintData qt_constructors {};
    QtMocHelpers::ClassInfos qt_classinfo({
            {    1,    2 },
    });
    return QtMocHelpers::metaObjectData<MprisRoot, qt_meta_tag_ZN9MprisRootE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums, qt_constructors, qt_classinfo);
}
Q_CONSTINIT const QMetaObject MprisRoot::staticMetaObject = { {
    QMetaObject::SuperData::link<QDBusAbstractAdaptor::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN9MprisRootE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN9MprisRootE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN9MprisRootE_t>.metaTypes,
    nullptr
} };

void MprisRoot::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<MprisRoot *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->Raise(); break;
        case 1: _t->Quit(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<bool*>(_v) = _t->yes(); break;
        case 1: *reinterpret_cast<bool*>(_v) = _t->yes(); break;
        case 2: *reinterpret_cast<bool*>(_v) = _t->no(); break;
        case 3: *reinterpret_cast<QString*>(_v) = _t->identity(); break;
        case 4: *reinterpret_cast<QString*>(_v) = _t->desktopEntry(); break;
        case 5: *reinterpret_cast<QStringList*>(_v) = _t->schemes(); break;
        case 6: *reinterpret_cast<QStringList*>(_v) = _t->mimeTypes(); break;
        default: break;
        }
    }
}

const QMetaObject *MprisRoot::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MprisRoot::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN9MprisRootE_t>.strings))
        return static_cast<void*>(this);
    return QDBusAbstractAdaptor::qt_metacast(_clname);
}

int MprisRoot::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDBusAbstractAdaptor::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 2;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
    return _id;
}
namespace {
struct qt_meta_tag_ZN11MprisPlayerE_t {};
} // unnamed namespace

template <> constexpr inline auto MprisPlayer::qt_create_metaobjectdata<qt_meta_tag_ZN11MprisPlayerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "MprisPlayer",
        "D-Bus Interface",
        "org.mpris.MediaPlayer2.Player",
        "Seeked",
        "",
        "position",
        "Next",
        "Previous",
        "Pause",
        "PlayPause",
        "Stop",
        "Play",
        "Seek",
        "offset",
        "SetPosition",
        "QDBusObjectPath",
        "track",
        "OpenUri",
        "PlaybackStatus",
        "LoopStatus",
        "Rate",
        "Shuffle",
        "Metadata",
        "QVariantMap",
        "Volume",
        "Position",
        "MinimumRate",
        "MaximumRate",
        "CanGoNext",
        "CanGoPrevious",
        "CanPlay",
        "CanPause",
        "CanSeek",
        "CanControl"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'Seeked'
        QtMocHelpers::SignalData<void(qlonglong)>(3, 4, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::LongLong, 5 },
        }}),
        // Slot 'Next'
        QtMocHelpers::SlotData<void()>(6, 4, QMC::AccessPublic, QMetaType::Void),
        // Slot 'Previous'
        QtMocHelpers::SlotData<void()>(7, 4, QMC::AccessPublic, QMetaType::Void),
        // Slot 'Pause'
        QtMocHelpers::SlotData<void()>(8, 4, QMC::AccessPublic, QMetaType::Void),
        // Slot 'PlayPause'
        QtMocHelpers::SlotData<void()>(9, 4, QMC::AccessPublic, QMetaType::Void),
        // Slot 'Stop'
        QtMocHelpers::SlotData<void()>(10, 4, QMC::AccessPublic, QMetaType::Void),
        // Slot 'Play'
        QtMocHelpers::SlotData<void()>(11, 4, QMC::AccessPublic, QMetaType::Void),
        // Slot 'Seek'
        QtMocHelpers::SlotData<void(qlonglong)>(12, 4, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::LongLong, 13 },
        }}),
        // Slot 'SetPosition'
        QtMocHelpers::SlotData<void(const QDBusObjectPath &, qlonglong)>(14, 4, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 15, 16 }, { QMetaType::LongLong, 5 },
        }}),
        // Slot 'OpenUri'
        QtMocHelpers::SlotData<void(const QString &)>(17, 4, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 4 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'PlaybackStatus'
        QtMocHelpers::PropertyData<QString>(18, QMetaType::QString, QMC::DefaultPropertyFlags),
        // property 'LoopStatus'
        QtMocHelpers::PropertyData<QString>(19, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable),
        // property 'Rate'
        QtMocHelpers::PropertyData<double>(20, QMetaType::Double, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet),
        // property 'Shuffle'
        QtMocHelpers::PropertyData<bool>(21, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet),
        // property 'Metadata'
        QtMocHelpers::PropertyData<QVariantMap>(22, 0x80000000 | 23, QMC::DefaultPropertyFlags | QMC::EnumOrFlag),
        // property 'Volume'
        QtMocHelpers::PropertyData<double>(24, QMetaType::Double, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet),
        // property 'Position'
        QtMocHelpers::PropertyData<qlonglong>(25, QMetaType::LongLong, QMC::DefaultPropertyFlags),
        // property 'MinimumRate'
        QtMocHelpers::PropertyData<double>(26, QMetaType::Double, QMC::DefaultPropertyFlags),
        // property 'MaximumRate'
        QtMocHelpers::PropertyData<double>(27, QMetaType::Double, QMC::DefaultPropertyFlags),
        // property 'CanGoNext'
        QtMocHelpers::PropertyData<bool>(28, QMetaType::Bool, QMC::DefaultPropertyFlags),
        // property 'CanGoPrevious'
        QtMocHelpers::PropertyData<bool>(29, QMetaType::Bool, QMC::DefaultPropertyFlags),
        // property 'CanPlay'
        QtMocHelpers::PropertyData<bool>(30, QMetaType::Bool, QMC::DefaultPropertyFlags),
        // property 'CanPause'
        QtMocHelpers::PropertyData<bool>(31, QMetaType::Bool, QMC::DefaultPropertyFlags),
        // property 'CanSeek'
        QtMocHelpers::PropertyData<bool>(32, QMetaType::Bool, QMC::DefaultPropertyFlags),
        // property 'CanControl'
        QtMocHelpers::PropertyData<bool>(33, QMetaType::Bool, QMC::DefaultPropertyFlags),
    };
    QtMocHelpers::UintData qt_enums {
    };
    QtMocHelpers::UintData qt_constructors {};
    QtMocHelpers::ClassInfos qt_classinfo({
            {    1,    2 },
    });
    return QtMocHelpers::metaObjectData<MprisPlayer, qt_meta_tag_ZN11MprisPlayerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums, qt_constructors, qt_classinfo);
}
Q_CONSTINIT const QMetaObject MprisPlayer::staticMetaObject = { {
    QMetaObject::SuperData::link<QDBusAbstractAdaptor::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11MprisPlayerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11MprisPlayerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN11MprisPlayerE_t>.metaTypes,
    nullptr
} };

void MprisPlayer::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<MprisPlayer *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->Seeked((*reinterpret_cast<std::add_pointer_t<qlonglong>>(_a[1]))); break;
        case 1: _t->Next(); break;
        case 2: _t->Previous(); break;
        case 3: _t->Pause(); break;
        case 4: _t->PlayPause(); break;
        case 5: _t->Stop(); break;
        case 6: _t->Play(); break;
        case 7: _t->Seek((*reinterpret_cast<std::add_pointer_t<qlonglong>>(_a[1]))); break;
        case 8: _t->SetPosition((*reinterpret_cast<std::add_pointer_t<QDBusObjectPath>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<qlonglong>>(_a[2]))); break;
        case 9: _t->OpenUri((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 8:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QDBusObjectPath >(); break;
            }
            break;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (MprisPlayer::*)(qlonglong )>(_a, &MprisPlayer::Seeked, 0))
            return;
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<QString*>(_v) = _t->status(); break;
        case 1: *reinterpret_cast<QString*>(_v) = _t->loop(); break;
        case 2: *reinterpret_cast<double*>(_v) = _t->rate(); break;
        case 3: *reinterpret_cast<bool*>(_v) = _t->shuffle(); break;
        case 4: *reinterpret_cast<QVariantMap*>(_v) = _t->metadata(); break;
        case 5: *reinterpret_cast<double*>(_v) = _t->volume(); break;
        case 6: *reinterpret_cast<qlonglong*>(_v) = _t->position(); break;
        case 7: *reinterpret_cast<double*>(_v) = _t->rate(); break;
        case 8: *reinterpret_cast<double*>(_v) = _t->rate(); break;
        case 9: *reinterpret_cast<bool*>(_v) = _t->canPlay(); break;
        case 10: *reinterpret_cast<bool*>(_v) = _t->canPlay(); break;
        case 11: *reinterpret_cast<bool*>(_v) = _t->canPlay(); break;
        case 12: *reinterpret_cast<bool*>(_v) = _t->canPlay(); break;
        case 13: *reinterpret_cast<bool*>(_v) = _t->canSeek(); break;
        case 14: *reinterpret_cast<bool*>(_v) = _t->yes(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 1: _t->setLoop(*reinterpret_cast<QString*>(_v)); break;
        case 2: _t->setRate(*reinterpret_cast<double*>(_v)); break;
        case 3: _t->setShuffle(*reinterpret_cast<bool*>(_v)); break;
        case 5: _t->setVolume(*reinterpret_cast<double*>(_v)); break;
        default: break;
        }
    }
}

const QMetaObject *MprisPlayer::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MprisPlayer::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11MprisPlayerE_t>.strings))
        return static_cast<void*>(this);
    return QDBusAbstractAdaptor::qt_metacast(_clname);
}

int MprisPlayer::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDBusAbstractAdaptor::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 15;
    }
    return _id;
}

// SIGNAL 0
void MprisPlayer::Seeked(qlonglong _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}
QT_WARNING_POP
