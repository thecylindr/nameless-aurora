/****************************************************************************
** Meta object code from reading C++ file 'storagemanager.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.12.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../src/core/storage/storagemanager.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'storagemanager.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN14StorageManagerE_t {};
} // unnamed namespace

template <> constexpr inline auto StorageManager::qt_create_metaobjectdata<qt_meta_tag_ZN14StorageManagerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "StorageManager",
        "freeBytesChanged",
        "",
        "totalBytesChanged",
        "libraryBytesChanged",
        "libraryFolderChanged",
        "refresh",
        "formatBytes",
        "bytes",
        "estimateForDuration",
        "durationMs",
        "quality",
        "canFit",
        "estimatedBytes",
        "freeBytes",
        "totalBytes",
        "libraryBytes",
        "libraryFolder"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'freeBytesChanged'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'totalBytesChanged'
        QtMocHelpers::SignalData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'libraryBytesChanged'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'libraryFolderChanged'
        QtMocHelpers::SignalData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'refresh'
        QtMocHelpers::MethodData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'formatBytes'
        QtMocHelpers::MethodData<QString(qint64) const>(7, 2, QMC::AccessPublic, QMetaType::QString, {{
            { QMetaType::LongLong, 8 },
        }}),
        // Method 'estimateForDuration'
        QtMocHelpers::MethodData<qint64(qint64, const QString &) const>(9, 2, QMC::AccessPublic, QMetaType::LongLong, {{
            { QMetaType::LongLong, 10 }, { QMetaType::QString, 11 },
        }}),
        // Method 'canFit'
        QtMocHelpers::MethodData<bool(qint64) const>(12, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::LongLong, 13 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'freeBytes'
        QtMocHelpers::PropertyData<qint64>(14, QMetaType::LongLong, QMC::DefaultPropertyFlags, 0),
        // property 'totalBytes'
        QtMocHelpers::PropertyData<qint64>(15, QMetaType::LongLong, QMC::DefaultPropertyFlags, 1),
        // property 'libraryBytes'
        QtMocHelpers::PropertyData<qint64>(16, QMetaType::LongLong, QMC::DefaultPropertyFlags, 2),
        // property 'libraryFolder'
        QtMocHelpers::PropertyData<QString>(17, QMetaType::QString, QMC::DefaultPropertyFlags, 3),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<StorageManager, qt_meta_tag_ZN14StorageManagerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject StorageManager::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14StorageManagerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14StorageManagerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN14StorageManagerE_t>.metaTypes,
    nullptr
} };

void StorageManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<StorageManager *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->freeBytesChanged(); break;
        case 1: _t->totalBytesChanged(); break;
        case 2: _t->libraryBytesChanged(); break;
        case 3: _t->libraryFolderChanged(); break;
        case 4: _t->refresh(); break;
        case 5: { QString _r = _t->formatBytes((*reinterpret_cast<std::add_pointer_t<qint64>>(_a[1])));
            if (_a[0]) *reinterpret_cast<QString*>(_a[0]) = std::move(_r); }  break;
        case 6: { qint64 _r = _t->estimateForDuration((*reinterpret_cast<std::add_pointer_t<qint64>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2])));
            if (_a[0]) *reinterpret_cast<qint64*>(_a[0]) = std::move(_r); }  break;
        case 7: { bool _r = _t->canFit((*reinterpret_cast<std::add_pointer_t<qint64>>(_a[1])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (StorageManager::*)()>(_a, &StorageManager::freeBytesChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (StorageManager::*)()>(_a, &StorageManager::totalBytesChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (StorageManager::*)()>(_a, &StorageManager::libraryBytesChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (StorageManager::*)()>(_a, &StorageManager::libraryFolderChanged, 3))
            return;
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<qint64*>(_v) = _t->freeBytes(); break;
        case 1: *reinterpret_cast<qint64*>(_v) = _t->totalBytes(); break;
        case 2: *reinterpret_cast<qint64*>(_v) = _t->libraryBytes(); break;
        case 3: *reinterpret_cast<QString*>(_v) = _t->libraryFolder(); break;
        default: break;
        }
    }
}

const QMetaObject *StorageManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *StorageManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14StorageManagerE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int StorageManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 8;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void StorageManager::freeBytesChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void StorageManager::totalBytesChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void StorageManager::libraryBytesChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void StorageManager::libraryFolderChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}
QT_WARNING_POP
