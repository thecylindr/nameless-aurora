/****************************************************************************
** Meta object code from reading C++ file 'dependencymanager.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.12.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../src/core/downloads/dependencymanager.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'dependencymanager.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN17DependencyManagerE_t {};
} // unnamed namespace

template <> constexpr inline auto DependencyManager::qt_create_metaobjectdata<qt_meta_tag_ZN17DependencyManagerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "DependencyManager",
        "busyChanged",
        "",
        "statusTextChanged",
        "lastErrorChanged",
        "pythonAvailableChanged",
        "ffmpegAvailableChanged",
        "ytDlpAvailableChanged",
        "spotdlAvailableChanged",
        "runtimeReadyChanged",
        "toolsDirChanged",
        "ytDlpVersionChanged",
        "spotdlVersionChanged",
        "ffmpegVersionChanged",
        "refresh",
        "installOrUpdate",
        "cancel",
        "busy",
        "statusText",
        "lastError",
        "pythonAvailable",
        "ffmpegAvailable",
        "ytDlpAvailable",
        "spotdlAvailable",
        "runtimeReady",
        "toolsDir",
        "ytDlpVersion",
        "spotdlVersion",
        "ffmpegVersion"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'busyChanged'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'statusTextChanged'
        QtMocHelpers::SignalData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'lastErrorChanged'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'pythonAvailableChanged'
        QtMocHelpers::SignalData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'ffmpegAvailableChanged'
        QtMocHelpers::SignalData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'ytDlpAvailableChanged'
        QtMocHelpers::SignalData<void()>(7, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'spotdlAvailableChanged'
        QtMocHelpers::SignalData<void()>(8, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'runtimeReadyChanged'
        QtMocHelpers::SignalData<void()>(9, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'toolsDirChanged'
        QtMocHelpers::SignalData<void()>(10, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'ytDlpVersionChanged'
        QtMocHelpers::SignalData<void()>(11, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'spotdlVersionChanged'
        QtMocHelpers::SignalData<void()>(12, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'ffmpegVersionChanged'
        QtMocHelpers::SignalData<void()>(13, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'refresh'
        QtMocHelpers::MethodData<void()>(14, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'installOrUpdate'
        QtMocHelpers::MethodData<void()>(15, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'cancel'
        QtMocHelpers::MethodData<void()>(16, 2, QMC::AccessPublic, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'busy'
        QtMocHelpers::PropertyData<bool>(17, QMetaType::Bool, QMC::DefaultPropertyFlags, 0),
        // property 'statusText'
        QtMocHelpers::PropertyData<QString>(18, QMetaType::QString, QMC::DefaultPropertyFlags, 1),
        // property 'lastError'
        QtMocHelpers::PropertyData<QString>(19, QMetaType::QString, QMC::DefaultPropertyFlags, 2),
        // property 'pythonAvailable'
        QtMocHelpers::PropertyData<bool>(20, QMetaType::Bool, QMC::DefaultPropertyFlags, 3),
        // property 'ffmpegAvailable'
        QtMocHelpers::PropertyData<bool>(21, QMetaType::Bool, QMC::DefaultPropertyFlags, 4),
        // property 'ytDlpAvailable'
        QtMocHelpers::PropertyData<bool>(22, QMetaType::Bool, QMC::DefaultPropertyFlags, 5),
        // property 'spotdlAvailable'
        QtMocHelpers::PropertyData<bool>(23, QMetaType::Bool, QMC::DefaultPropertyFlags, 6),
        // property 'runtimeReady'
        QtMocHelpers::PropertyData<bool>(24, QMetaType::Bool, QMC::DefaultPropertyFlags, 7),
        // property 'toolsDir'
        QtMocHelpers::PropertyData<QString>(25, QMetaType::QString, QMC::DefaultPropertyFlags, 8),
        // property 'ytDlpVersion'
        QtMocHelpers::PropertyData<QString>(26, QMetaType::QString, QMC::DefaultPropertyFlags, 9),
        // property 'spotdlVersion'
        QtMocHelpers::PropertyData<QString>(27, QMetaType::QString, QMC::DefaultPropertyFlags, 10),
        // property 'ffmpegVersion'
        QtMocHelpers::PropertyData<QString>(28, QMetaType::QString, QMC::DefaultPropertyFlags, 11),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<DependencyManager, qt_meta_tag_ZN17DependencyManagerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject DependencyManager::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN17DependencyManagerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN17DependencyManagerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN17DependencyManagerE_t>.metaTypes,
    nullptr
} };

void DependencyManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<DependencyManager *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->busyChanged(); break;
        case 1: _t->statusTextChanged(); break;
        case 2: _t->lastErrorChanged(); break;
        case 3: _t->pythonAvailableChanged(); break;
        case 4: _t->ffmpegAvailableChanged(); break;
        case 5: _t->ytDlpAvailableChanged(); break;
        case 6: _t->spotdlAvailableChanged(); break;
        case 7: _t->runtimeReadyChanged(); break;
        case 8: _t->toolsDirChanged(); break;
        case 9: _t->ytDlpVersionChanged(); break;
        case 10: _t->spotdlVersionChanged(); break;
        case 11: _t->ffmpegVersionChanged(); break;
        case 12: _t->refresh(); break;
        case 13: _t->installOrUpdate(); break;
        case 14: _t->cancel(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (DependencyManager::*)()>(_a, &DependencyManager::busyChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (DependencyManager::*)()>(_a, &DependencyManager::statusTextChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (DependencyManager::*)()>(_a, &DependencyManager::lastErrorChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (DependencyManager::*)()>(_a, &DependencyManager::pythonAvailableChanged, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (DependencyManager::*)()>(_a, &DependencyManager::ffmpegAvailableChanged, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (DependencyManager::*)()>(_a, &DependencyManager::ytDlpAvailableChanged, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (DependencyManager::*)()>(_a, &DependencyManager::spotdlAvailableChanged, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (DependencyManager::*)()>(_a, &DependencyManager::runtimeReadyChanged, 7))
            return;
        if (QtMocHelpers::indexOfMethod<void (DependencyManager::*)()>(_a, &DependencyManager::toolsDirChanged, 8))
            return;
        if (QtMocHelpers::indexOfMethod<void (DependencyManager::*)()>(_a, &DependencyManager::ytDlpVersionChanged, 9))
            return;
        if (QtMocHelpers::indexOfMethod<void (DependencyManager::*)()>(_a, &DependencyManager::spotdlVersionChanged, 10))
            return;
        if (QtMocHelpers::indexOfMethod<void (DependencyManager::*)()>(_a, &DependencyManager::ffmpegVersionChanged, 11))
            return;
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<bool*>(_v) = _t->busy(); break;
        case 1: *reinterpret_cast<QString*>(_v) = _t->statusText(); break;
        case 2: *reinterpret_cast<QString*>(_v) = _t->lastError(); break;
        case 3: *reinterpret_cast<bool*>(_v) = _t->pythonAvailable(); break;
        case 4: *reinterpret_cast<bool*>(_v) = _t->ffmpegAvailable(); break;
        case 5: *reinterpret_cast<bool*>(_v) = _t->ytDlpAvailable(); break;
        case 6: *reinterpret_cast<bool*>(_v) = _t->spotdlAvailable(); break;
        case 7: *reinterpret_cast<bool*>(_v) = _t->runtimeReady(); break;
        case 8: *reinterpret_cast<QString*>(_v) = _t->toolsDir(); break;
        case 9: *reinterpret_cast<QString*>(_v) = _t->ytDlpVersion(); break;
        case 10: *reinterpret_cast<QString*>(_v) = _t->spotdlVersion(); break;
        case 11: *reinterpret_cast<QString*>(_v) = _t->ffmpegVersion(); break;
        default: break;
        }
    }
}

const QMetaObject *DependencyManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DependencyManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN17DependencyManagerE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int DependencyManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
        _id -= 12;
    }
    return _id;
}

// SIGNAL 0
void DependencyManager::busyChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void DependencyManager::statusTextChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void DependencyManager::lastErrorChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void DependencyManager::pythonAvailableChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void DependencyManager::ffmpegAvailableChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void DependencyManager::ytDlpAvailableChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void DependencyManager::spotdlAvailableChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void DependencyManager::runtimeReadyChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}

// SIGNAL 8
void DependencyManager::toolsDirChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 8, nullptr);
}

// SIGNAL 9
void DependencyManager::ytDlpVersionChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 9, nullptr);
}

// SIGNAL 10
void DependencyManager::spotdlVersionChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 10, nullptr);
}

// SIGNAL 11
void DependencyManager::ffmpegVersionChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 11, nullptr);
}
QT_WARNING_POP
