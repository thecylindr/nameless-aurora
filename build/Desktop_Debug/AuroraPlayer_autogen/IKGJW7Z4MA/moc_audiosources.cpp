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
        "catalogQueued",
        "trackId",
        "operationFinished",
        "previewResolved",
        "QVariantMap",
        "candidate",
        "error",
        "openTrack",
        "id",
        "search",
        "source",
        "query",
        "searchAll",
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
        "results",
        "QVariantList",
        "busy",
        "status",
        "sources",
        "recovering",
        "recoveryStatus",
        "recoveryPending",
        "automaticRecovery",
        "extendedRecovery",
        "recoverySources"
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
        // Signal 'catalogQueued'
        QtMocHelpers::SignalData<void(int)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 8 },
        }}),
        // Signal 'operationFinished'
        QtMocHelpers::SignalData<void()>(9, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'previewResolved'
        QtMocHelpers::SignalData<void(const QVariantMap &, const QString &)>(10, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 11, 12 }, { QMetaType::QString, 13 },
        }}),
        // Method 'openTrack'
        QtMocHelpers::MethodData<void(int)>(14, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 15 },
        }}),
        // Method 'search'
        QtMocHelpers::MethodData<void(const QString &, const QString &)>(16, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 17 }, { QMetaType::QString, 18 },
        }}),
        // Method 'search'
        QtMocHelpers::MethodData<void(const QString &)>(16, 2, QMC::AccessPublic | QMC::MethodCloned, QMetaType::Void, {{
            { QMetaType::QString, 17 },
        }}),
        // Method 'searchAll'
        QtMocHelpers::MethodData<void(const QString &)>(19, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 18 },
        }}),
        // Method 'searchAll'
        QtMocHelpers::MethodData<void()>(19, 2, QMC::AccessPublic | QMC::MethodCloned, QMetaType::Void),
        // Method 'choose'
        QtMocHelpers::MethodData<void(int)>(20, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 21 },
        }}),
        // Method 'downloadLink'
        QtMocHelpers::MethodData<void(const QString &)>(22, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 23 },
        }}),
        // Method 'cancel'
        QtMocHelpers::MethodData<void()>(24, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'close'
        QtMocHelpers::MethodData<void()>(25, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'recoverTrack'
        QtMocHelpers::MethodData<void(int)>(26, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 15 },
        }}),
        // Method 'recoverMissing'
        QtMocHelpers::MethodData<void()>(27, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'cancelRecovery'
        QtMocHelpers::MethodData<void()>(28, 2, QMC::AccessPublic, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'target'
        QtMocHelpers::PropertyData<QVariantMap>(29, 0x80000000 | 11, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 1),
        // property 'results'
        QtMocHelpers::PropertyData<QVariantList>(30, 0x80000000 | 31, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 2),
        // property 'busy'
        QtMocHelpers::PropertyData<bool>(32, QMetaType::Bool, QMC::DefaultPropertyFlags, 3),
        // property 'status'
        QtMocHelpers::PropertyData<QString>(33, QMetaType::QString, QMC::DefaultPropertyFlags, 3),
        // property 'source'
        QtMocHelpers::PropertyData<QString>(17, QMetaType::QString, QMC::DefaultPropertyFlags, 3),
        // property 'sources'
        QtMocHelpers::PropertyData<QStringList>(34, QMetaType::QStringList, QMC::DefaultPropertyFlags | QMC::Constant),
        // property 'recovering'
        QtMocHelpers::PropertyData<bool>(35, QMetaType::Bool, QMC::DefaultPropertyFlags, 3),
        // property 'recoveryStatus'
        QtMocHelpers::PropertyData<QString>(36, QMetaType::QString, QMC::DefaultPropertyFlags, 3),
        // property 'recoveryPending'
        QtMocHelpers::PropertyData<int>(37, QMetaType::Int, QMC::DefaultPropertyFlags, 3),
        // property 'automaticRecovery'
        QtMocHelpers::PropertyData<bool>(38, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 3),
        // property 'extendedRecovery'
        QtMocHelpers::PropertyData<bool>(39, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 3),
        // property 'recoverySources'
        QtMocHelpers::PropertyData<QStringList>(40, QMetaType::QStringList, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 3),
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
        case 5: _t->catalogQueued((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 6: _t->operationFinished(); break;
        case 7: _t->previewResolved((*reinterpret_cast<std::add_pointer_t<QVariantMap>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 8: _t->openTrack((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 9: _t->search((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 10: _t->search((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 11: _t->searchAll((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 12: _t->searchAll(); break;
        case 13: _t->choose((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 14: _t->downloadLink((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 15: _t->cancel(); break;
        case 16: _t->close(); break;
        case 17: _t->recoverTrack((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 18: _t->recoverMissing(); break;
        case 19: _t->cancelRecovery(); break;
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
        if (QtMocHelpers::indexOfMethod<void (AudioSources::*)(int )>(_a, &AudioSources::catalogQueued, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (AudioSources::*)()>(_a, &AudioSources::operationFinished, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (AudioSources::*)(const QVariantMap & , const QString & )>(_a, &AudioSources::previewResolved, 7))
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
        case 11: *reinterpret_cast<QStringList*>(_v) = _t->recoverySources(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 9: _t->setAutomaticRecovery(*reinterpret_cast<bool*>(_v)); break;
        case 10: _t->setExtendedRecovery(*reinterpret_cast<bool*>(_v)); break;
        case 11: _t->setRecoverySources(*reinterpret_cast<QStringList*>(_v)); break;
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
        if (_id < 20)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 20;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 20)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 20;
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

// SIGNAL 5
void AudioSources::catalogQueued(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 5, nullptr, _t1);
}

// SIGNAL 6
void AudioSources::operationFinished()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void AudioSources::previewResolved(const QVariantMap & _t1, const QString & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 7, nullptr, _t1, _t2);
}
QT_WARNING_POP
