/****************************************************************************
** Meta object code from reading C++ file 'audiosources.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.12.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../src/core/downloads/audiosources.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'audiosources.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN12AudioSourcesE_t {};
} // unnamed namespace

template <> constexpr inline auto AudioSources::qt_create_metaobjectdata<qt_meta_tag_ZN12AudioSourcesE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "AudioSources",
        "opened",
        "",
        "targetChanged",
        "resultsChanged",
        "changed",
        "queued",
        "openTrack",
        "id",
        "search",
        "source",
        "query",
        "choose",
        "index",
        "downloadLink",
        "url",
        "cancel",
        "close",
        "recoverTrack",
        "recoverMissing",
        "cancelRecovery",
        "target",
        "QVariantMap",
        "results",
        "QVariantList",
        "busy",
        "status",
        "sources",
        "recovering",
        "recoveryStatus",
        "recoveryPending",
        "automaticRecovery",
        "extendedRecovery"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'opened'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'targetChanged'
        QtMocHelpers::SignalData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'resultsChanged'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'changed'
        QtMocHelpers::SignalData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'queued'
        QtMocHelpers::SignalData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'openTrack'
        QtMocHelpers::MethodData<void(int)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 8 },
        }}),
        // Method 'search'
        QtMocHelpers::MethodData<void(const QString &, const QString &)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 10 }, { QMetaType::QString, 11 },
        }}),
        // Method 'search'
        QtMocHelpers::MethodData<void(const QString &)>(9, 2, QMC::AccessPublic | QMC::MethodCloned, QMetaType::Void, {{
            { QMetaType::QString, 10 },
        }}),
        // Method 'choose'
        QtMocHelpers::MethodData<void(int)>(12, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 13 },
        }}),
        // Method 'downloadLink'
        QtMocHelpers::MethodData<void(const QString &)>(14, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 15 },
        }}),
        // Method 'cancel'
        QtMocHelpers::MethodData<void()>(16, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'close'
        QtMocHelpers::MethodData<void()>(17, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'recoverTrack'
        QtMocHelpers::MethodData<void(int)>(18, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 8 },
        }}),
        // Method 'recoverMissing'
        QtMocHelpers::MethodData<void()>(19, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'cancelRecovery'
        QtMocHelpers::MethodData<void()>(20, 2, QMC::AccessPublic, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'target'
        QtMocHelpers::PropertyData<QVariantMap>(21, 0x80000000 | 22, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 1),
        // property 'results'
        QtMocHelpers::PropertyData<QVariantList>(23, 0x80000000 | 24, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 2),
        // property 'busy'
        QtMocHelpers::PropertyData<bool>(25, QMetaType::Bool, QMC::DefaultPropertyFlags, 3),
        // property 'status'
        QtMocHelpers::PropertyData<QString>(26, QMetaType::QString, QMC::DefaultPropertyFlags, 3),
        // property 'source'
        QtMocHelpers::PropertyData<QString>(10, QMetaType::QString, QMC::DefaultPropertyFlags, 3),
        // property 'sources'
        QtMocHelpers::PropertyData<QStringList>(27, QMetaType::QStringList, QMC::DefaultPropertyFlags | QMC::Constant),
        // property 'recovering'
        QtMocHelpers::PropertyData<bool>(28, QMetaType::Bool, QMC::DefaultPropertyFlags, 3),
        // property 'recoveryStatus'
        QtMocHelpers::PropertyData<QString>(29, QMetaType::QString, QMC::DefaultPropertyFlags, 3),
        // property 'recoveryPending'
        QtMocHelpers::PropertyData<int>(30, QMetaType::Int, QMC::DefaultPropertyFlags, 3),
        // property 'automaticRecovery'
        QtMocHelpers::PropertyData<bool>(31, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 3),
        // property 'extendedRecovery'
        QtMocHelpers::PropertyData<bool>(32, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 3),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<AudioSources, qt_meta_tag_ZN12AudioSourcesE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject AudioSources::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12AudioSourcesE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12AudioSourcesE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN12AudioSourcesE_t>.metaTypes,
    nullptr
} };

void AudioSources::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<AudioSources *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->opened(); break;
        case 1: _t->targetChanged(); break;
        case 2: _t->resultsChanged(); break;
        case 3: _t->changed(); break;
        case 4: _t->queued(); break;
        case 5: _t->openTrack((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 6: _t->search((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 7: _t->search((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 8: _t->choose((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 9: _t->downloadLink((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 10: _t->cancel(); break;
        case 11: _t->close(); break;
        case 12: _t->recoverTrack((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 13: _t->recoverMissing(); break;
        case 14: _t->cancelRecovery(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (AudioSources::*)()>(_a, &AudioSources::opened, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (AudioSources::*)()>(_a, &AudioSources::targetChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (AudioSources::*)()>(_a, &AudioSources::resultsChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (AudioSources::*)()>(_a, &AudioSources::changed, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (AudioSources::*)()>(_a, &AudioSources::queued, 4))
            return;
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<QVariantMap*>(_v) = _t->target(); break;
        case 1: *reinterpret_cast<QVariantList*>(_v) = _t->results(); break;
        case 2: *reinterpret_cast<bool*>(_v) = _t->busy(); break;
        case 3: *reinterpret_cast<QString*>(_v) = _t->status(); break;
        case 4: *reinterpret_cast<QString*>(_v) = _t->source(); break;
        case 5: *reinterpret_cast<QStringList*>(_v) = _t->sources(); break;
        case 6: *reinterpret_cast<bool*>(_v) = _t->recovering(); break;
        case 7: *reinterpret_cast<QString*>(_v) = _t->recoveryStatus(); break;
        case 8: *reinterpret_cast<int*>(_v) = _t->recoveryPending(); break;
        case 9: *reinterpret_cast<bool*>(_v) = _t->automaticRecovery(); break;
        case 10: *reinterpret_cast<bool*>(_v) = _t->extendedRecovery(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 9: _t->setAutomaticRecovery(*reinterpret_cast<bool*>(_v)); break;
        case 10: _t->setExtendedRecovery(*reinterpret_cast<bool*>(_v)); break;
        default: break;
        }
    }
}

const QMetaObject *AudioSources::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *AudioSources::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12AudioSourcesE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int AudioSources::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 15)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 15;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 15)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 15;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    }
    return _id;
}

// SIGNAL 0
void AudioSources::opened()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void AudioSources::targetChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void AudioSources::resultsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void AudioSources::changed()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void AudioSources::queued()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}
QT_WARNING_POP
