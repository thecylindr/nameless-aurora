/****************************************************************************
** Meta object code from reading C++ file 'libraryexchange.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.12.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/core/libraryexchange.h"
#include <QtNetwork/QSslError>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'libraryexchange.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN15LibraryExchangeE_t {};
} // unnamed namespace

template <> constexpr inline auto LibraryExchange::qt_create_metaobjectdata<qt_meta_tag_ZN15LibraryExchangeE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "LibraryExchange",
        "changed",
        "",
        "completed",
        "playlistId",
        "audioImportRequested",
        "QVariantList",
        "tracks",
        "cancel",
        "importList",
        "QUrl",
        "file",
        "source",
        "importUrl",
        "url",
        "importMyMusic",
        "service",
        "profileId",
        "repairVkMetadata",
        "importFromBrowser",
        "exportPortable",
        "folder",
        "playlist",
        "exportText",
        "busy",
        "status",
        "importedTracks"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'changed'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'completed'
        QtMocHelpers::SignalData<void(int)>(3, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 4 },
        }}),
        // Signal 'audioImportRequested'
        QtMocHelpers::SignalData<void(const QVariantList &)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 6, 7 },
        }}),
        // Method 'cancel'
        QtMocHelpers::MethodData<void()>(8, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'importList'
        QtMocHelpers::MethodData<void(const QUrl &, const QString &)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 10, 11 }, { QMetaType::QString, 12 },
        }}),
        // Method 'importUrl'
        QtMocHelpers::MethodData<void(const QString &)>(13, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 14 },
        }}),
        // Method 'importMyMusic'
        QtMocHelpers::MethodData<void(const QString &, const QString &)>(15, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 16 }, { QMetaType::QString, 17 },
        }}),
        // Method 'importMyMusic'
        QtMocHelpers::MethodData<void(const QString &)>(15, 2, QMC::AccessPublic | QMC::MethodCloned, QMetaType::Void, {{
            { QMetaType::QString, 16 },
        }}),
        // Method 'repairVkMetadata'
        QtMocHelpers::MethodData<void()>(18, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'importFromBrowser'
        QtMocHelpers::MethodData<void(const QString &, const QString &)>(19, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 14 }, { QMetaType::QString, 17 },
        }}),
        // Method 'importFromBrowser'
        QtMocHelpers::MethodData<void(const QString &)>(19, 2, QMC::AccessPublic | QMC::MethodCloned, QMetaType::Void, {{
            { QMetaType::QString, 14 },
        }}),
        // Method 'exportPortable'
        QtMocHelpers::MethodData<void(const QUrl &, int)>(20, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 10, 21 }, { QMetaType::Int, 22 },
        }}),
        // Method 'exportPortable'
        QtMocHelpers::MethodData<void(const QUrl &)>(20, 2, QMC::AccessPublic | QMC::MethodCloned, QMetaType::Void, {{
            { 0x80000000 | 10, 21 },
        }}),
        // Method 'exportText'
        QtMocHelpers::MethodData<void(const QUrl &, int)>(23, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 10, 11 }, { QMetaType::Int, 22 },
        }}),
        // Method 'exportText'
        QtMocHelpers::MethodData<void(const QUrl &)>(23, 2, QMC::AccessPublic | QMC::MethodCloned, QMetaType::Void, {{
            { 0x80000000 | 10, 11 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'busy'
        QtMocHelpers::PropertyData<bool>(24, QMetaType::Bool, QMC::DefaultPropertyFlags, 0),
        // property 'status'
        QtMocHelpers::PropertyData<QString>(25, QMetaType::QString, QMC::DefaultPropertyFlags, 0),
        // property 'importedTracks'
        QtMocHelpers::PropertyData<QVariantList>(26, 0x80000000 | 6, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 0),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<LibraryExchange, qt_meta_tag_ZN15LibraryExchangeE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject LibraryExchange::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15LibraryExchangeE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15LibraryExchangeE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN15LibraryExchangeE_t>.metaTypes,
    nullptr
} };

void LibraryExchange::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<LibraryExchange *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->changed(); break;
        case 1: _t->completed((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 2: _t->audioImportRequested((*reinterpret_cast<std::add_pointer_t<QVariantList>>(_a[1]))); break;
        case 3: _t->cancel(); break;
        case 4: _t->importList((*reinterpret_cast<std::add_pointer_t<QUrl>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 5: _t->importUrl((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 6: _t->importMyMusic((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 7: _t->importMyMusic((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 8: _t->repairVkMetadata(); break;
        case 9: _t->importFromBrowser((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 10: _t->importFromBrowser((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 11: _t->exportPortable((*reinterpret_cast<std::add_pointer_t<QUrl>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2]))); break;
        case 12: _t->exportPortable((*reinterpret_cast<std::add_pointer_t<QUrl>>(_a[1]))); break;
        case 13: _t->exportText((*reinterpret_cast<std::add_pointer_t<QUrl>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2]))); break;
        case 14: _t->exportText((*reinterpret_cast<std::add_pointer_t<QUrl>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (LibraryExchange::*)()>(_a, &LibraryExchange::changed, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (LibraryExchange::*)(int )>(_a, &LibraryExchange::completed, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (LibraryExchange::*)(const QVariantList & )>(_a, &LibraryExchange::audioImportRequested, 2))
            return;
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<bool*>(_v) = _t->busy(); break;
        case 1: *reinterpret_cast<QString*>(_v) = _t->status(); break;
        case 2: *reinterpret_cast<QVariantList*>(_v) = _t->importedTracks(); break;
        default: break;
        }
    }
}

const QMetaObject *LibraryExchange::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *LibraryExchange::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15LibraryExchangeE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int LibraryExchange::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void LibraryExchange::changed()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void LibraryExchange::completed(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void LibraryExchange::audioImportRequested(const QVariantList & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}
QT_WARNING_POP
