/****************************************************************************
** Meta object code from reading C++ file 'librarymodel.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.12.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../src/core/librarymodel.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'librarymodel.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN12LibraryModelE_t {};
} // unnamed namespace

template <> constexpr inline auto LibraryModel::qt_create_metaobjectdata<qt_meta_tag_ZN12LibraryModelE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "LibraryModel",
        "countChanged",
        "",
        "searchTextChanged",
        "lastErrorChanged",
        "libraryChanged",
        "recommendations",
        "QVariantList",
        "limit",
        "count",
        "searchText",
        "lastError",
        "Role",
        "IdRole",
        "TitleRole",
        "ArtistRole",
        "AlbumRole",
        "FilePathRole",
        "SourceUrlRole",
        "SourceRole",
        "FormatRole",
        "CoverPathRole",
        "DurationRole",
        "BitrateRole",
        "RatingRole",
        "PlayCountRole",
        "LastPlayedRole",
        "SkipCountRole",
        "ListenMsRole"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'countChanged'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'searchTextChanged'
        QtMocHelpers::SignalData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'lastErrorChanged'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'libraryChanged'
        QtMocHelpers::SignalData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'recommendations'
        QtMocHelpers::MethodData<QVariantList(int) const>(6, 2, QMC::AccessPublic, 0x80000000 | 7, {{
            { QMetaType::Int, 8 },
        }}),
        // Method 'recommendations'
        QtMocHelpers::MethodData<QVariantList() const>(6, 2, QMC::AccessPublic | QMC::MethodCloned, 0x80000000 | 7),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'count'
        QtMocHelpers::PropertyData<int>(9, QMetaType::Int, QMC::DefaultPropertyFlags, 0),
        // property 'searchText'
        QtMocHelpers::PropertyData<QString>(10, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 1),
        // property 'lastError'
        QtMocHelpers::PropertyData<QString>(11, QMetaType::QString, QMC::DefaultPropertyFlags, 2),
    };
    QtMocHelpers::UintData qt_enums {
        // enum 'Role'
        QtMocHelpers::EnumData<enum Role>(12, 12, QMC::EnumFlags{}).add({
            {   13, Role::IdRole },
            {   14, Role::TitleRole },
            {   15, Role::ArtistRole },
            {   16, Role::AlbumRole },
            {   17, Role::FilePathRole },
            {   18, Role::SourceUrlRole },
            {   19, Role::SourceRole },
            {   20, Role::FormatRole },
            {   21, Role::CoverPathRole },
            {   22, Role::DurationRole },
            {   23, Role::BitrateRole },
            {   24, Role::RatingRole },
            {   25, Role::PlayCountRole },
            {   26, Role::LastPlayedRole },
            {   27, Role::SkipCountRole },
            {   28, Role::ListenMsRole },
        }),
    };
    return QtMocHelpers::metaObjectData<LibraryModel, qt_meta_tag_ZN12LibraryModelE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject LibraryModel::staticMetaObject = { {
    QMetaObject::SuperData::link<QAbstractListModel::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12LibraryModelE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12LibraryModelE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN12LibraryModelE_t>.metaTypes,
    nullptr
} };

void LibraryModel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<LibraryModel *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->countChanged(); break;
        case 1: _t->searchTextChanged(); break;
        case 2: _t->lastErrorChanged(); break;
        case 3: _t->libraryChanged(); break;
        case 4: { QVariantList _r = _t->recommendations((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast<QVariantList*>(_a[0]) = std::move(_r); }  break;
        case 5: { QVariantList _r = _t->recommendations();
            if (_a[0]) *reinterpret_cast<QVariantList*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (LibraryModel::*)()>(_a, &LibraryModel::countChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (LibraryModel::*)()>(_a, &LibraryModel::searchTextChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (LibraryModel::*)()>(_a, &LibraryModel::lastErrorChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (LibraryModel::*)()>(_a, &LibraryModel::libraryChanged, 3))
            return;
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<int*>(_v) = _t->count(); break;
        case 1: *reinterpret_cast<QString*>(_v) = _t->searchText(); break;
        case 2: *reinterpret_cast<QString*>(_v) = _t->lastError(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 1: _t->setSearchText(*reinterpret_cast<QString*>(_v)); break;
        default: break;
        }
    }
}

const QMetaObject *LibraryModel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *LibraryModel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12LibraryModelE_t>.strings))
        return static_cast<void*>(this);
    return QAbstractListModel::qt_metacast(_clname);
}

int LibraryModel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAbstractListModel::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 6;
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
void LibraryModel::countChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void LibraryModel::searchTextChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void LibraryModel::lastErrorChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void LibraryModel::libraryChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}
QT_WARNING_POP
