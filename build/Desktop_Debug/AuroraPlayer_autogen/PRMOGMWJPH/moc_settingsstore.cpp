/****************************************************************************
** Meta object code from reading C++ file 'settingsstore.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.12.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../src/core/settingsstore.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'settingsstore.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN13SettingsStoreE_t {};
} // unnamed namespace

template <> constexpr inline auto SettingsStore::qt_create_metaobjectdata<qt_meta_tag_ZN13SettingsStoreE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "SettingsStore",
        "libraryFolderChanged",
        "",
        "volumeChanged",
        "qualityChanged",
        "downloadConcurrencyChanged",
        "browserChanged",
        "browserProfileChanged",
        "confirmBulkDownloadChanged",
        "ytDlpPathChanged",
        "spotdlPathChanged",
        "resetPreferences",
        "libraryFolder",
        "volume",
        "quality",
        "downloadConcurrency",
        "browser",
        "browserProfile",
        "confirmBulkDownload",
        "ytDlpPath",
        "spotdlPath"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'libraryFolderChanged'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'volumeChanged'
        QtMocHelpers::SignalData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'qualityChanged'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'downloadConcurrencyChanged'
        QtMocHelpers::SignalData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'browserChanged'
        QtMocHelpers::SignalData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'browserProfileChanged'
        QtMocHelpers::SignalData<void()>(7, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'confirmBulkDownloadChanged'
        QtMocHelpers::SignalData<void()>(8, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'ytDlpPathChanged'
        QtMocHelpers::SignalData<void()>(9, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'spotdlPathChanged'
        QtMocHelpers::SignalData<void()>(10, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'resetPreferences'
        QtMocHelpers::MethodData<void()>(11, 2, QMC::AccessPublic, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'libraryFolder'
        QtMocHelpers::PropertyData<QString>(12, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 0),
        // property 'volume'
        QtMocHelpers::PropertyData<double>(13, QMetaType::Double, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 1),
        // property 'quality'
        QtMocHelpers::PropertyData<QString>(14, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 2),
        // property 'downloadConcurrency'
        QtMocHelpers::PropertyData<int>(15, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 3),
        // property 'browser'
        QtMocHelpers::PropertyData<QString>(16, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 4),
        // property 'browserProfile'
        QtMocHelpers::PropertyData<QString>(17, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 5),
        // property 'confirmBulkDownload'
        QtMocHelpers::PropertyData<bool>(18, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 6),
        // property 'ytDlpPath'
        QtMocHelpers::PropertyData<QString>(19, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 7),
        // property 'spotdlPath'
        QtMocHelpers::PropertyData<QString>(20, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 8),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<SettingsStore, qt_meta_tag_ZN13SettingsStoreE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject SettingsStore::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13SettingsStoreE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13SettingsStoreE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN13SettingsStoreE_t>.metaTypes,
    nullptr
} };

void SettingsStore::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<SettingsStore *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->libraryFolderChanged(); break;
        case 1: _t->volumeChanged(); break;
        case 2: _t->qualityChanged(); break;
        case 3: _t->downloadConcurrencyChanged(); break;
        case 4: _t->browserChanged(); break;
        case 5: _t->browserProfileChanged(); break;
        case 6: _t->confirmBulkDownloadChanged(); break;
        case 7: _t->ytDlpPathChanged(); break;
        case 8: _t->spotdlPathChanged(); break;
        case 9: _t->resetPreferences(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (SettingsStore::*)()>(_a, &SettingsStore::libraryFolderChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsStore::*)()>(_a, &SettingsStore::volumeChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsStore::*)()>(_a, &SettingsStore::qualityChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsStore::*)()>(_a, &SettingsStore::downloadConcurrencyChanged, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsStore::*)()>(_a, &SettingsStore::browserChanged, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsStore::*)()>(_a, &SettingsStore::browserProfileChanged, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsStore::*)()>(_a, &SettingsStore::confirmBulkDownloadChanged, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsStore::*)()>(_a, &SettingsStore::ytDlpPathChanged, 7))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsStore::*)()>(_a, &SettingsStore::spotdlPathChanged, 8))
            return;
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<QString*>(_v) = _t->libraryFolder(); break;
        case 1: *reinterpret_cast<double*>(_v) = _t->volume(); break;
        case 2: *reinterpret_cast<QString*>(_v) = _t->quality(); break;
        case 3: *reinterpret_cast<int*>(_v) = _t->downloadConcurrency(); break;
        case 4: *reinterpret_cast<QString*>(_v) = _t->browser(); break;
        case 5: *reinterpret_cast<QString*>(_v) = _t->browserProfile(); break;
        case 6: *reinterpret_cast<bool*>(_v) = _t->confirmBulkDownload(); break;
        case 7: *reinterpret_cast<QString*>(_v) = _t->ytDlpPath(); break;
        case 8: *reinterpret_cast<QString*>(_v) = _t->spotdlPath(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setLibraryFolder(*reinterpret_cast<QString*>(_v)); break;
        case 1: _t->setVolume(*reinterpret_cast<double*>(_v)); break;
        case 2: _t->setQuality(*reinterpret_cast<QString*>(_v)); break;
        case 3: _t->setDownloadConcurrency(*reinterpret_cast<int*>(_v)); break;
        case 4: _t->setBrowser(*reinterpret_cast<QString*>(_v)); break;
        case 5: _t->setBrowserProfile(*reinterpret_cast<QString*>(_v)); break;
        case 6: _t->setConfirmBulkDownload(*reinterpret_cast<bool*>(_v)); break;
        case 7: _t->setYtDlpPath(*reinterpret_cast<QString*>(_v)); break;
        case 8: _t->setSpotdlPath(*reinterpret_cast<QString*>(_v)); break;
        default: break;
        }
    }
}

const QMetaObject *SettingsStore::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SettingsStore::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13SettingsStoreE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int SettingsStore::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 10;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void SettingsStore::libraryFolderChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void SettingsStore::volumeChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void SettingsStore::qualityChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void SettingsStore::downloadConcurrencyChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void SettingsStore::browserChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void SettingsStore::browserProfileChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void SettingsStore::confirmBulkDownloadChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void SettingsStore::ytDlpPathChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}

// SIGNAL 8
void SettingsStore::spotdlPathChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 8, nullptr);
}
QT_WARNING_POP
