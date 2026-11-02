/****************************************************************************
** Meta object code from reading C++ file 'importpreviewer.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.12.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../src/core/downloads/importpreviewer.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'importpreviewer.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN15ImportPreviewerE_t {};
} // unnamed namespace

template <> constexpr inline auto ImportPreviewer::qt_create_metaobjectdata<qt_meta_tag_ZN15ImportPreviewerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "ImportPreviewer",
        "busyChanged",
        "",
        "statusTextChanged",
        "lastErrorChanged",
        "totalDurationChanged",
        "selectedCountChanged",
        "selectedBytesChanged",
        "unknownSizesChanged",
        "sourceUrlChanged",
        "inspect",
        "url",
        "quality",
        "authorized",
        "toggle",
        "index",
        "selectAll",
        "selected",
        "startSelected",
        "busy",
        "statusText",
        "lastError",
        "totalDuration",
        "selectedCount",
        "selectedBytes",
        "unknownSizes",
        "sourceUrl",
        "Role",
        "TitleRole",
        "ArtistRole",
        "UrlRole",
        "DurationRole",
        "SizeRole",
        "SelectedRole"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'busyChanged'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'statusTextChanged'
        QtMocHelpers::SignalData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'lastErrorChanged'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'totalDurationChanged'
        QtMocHelpers::SignalData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'selectedCountChanged'
        QtMocHelpers::SignalData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'selectedBytesChanged'
        QtMocHelpers::SignalData<void()>(7, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'unknownSizesChanged'
        QtMocHelpers::SignalData<void()>(8, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'sourceUrlChanged'
        QtMocHelpers::SignalData<void()>(9, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'inspect'
        QtMocHelpers::MethodData<void(const QString &, const QString &, bool)>(10, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 11 }, { QMetaType::QString, 12 }, { QMetaType::Bool, 13 },
        }}),
        // Method 'inspect'
        QtMocHelpers::MethodData<void(const QString &, const QString &)>(10, 2, QMC::AccessPublic | QMC::MethodCloned, QMetaType::Void, {{
            { QMetaType::QString, 11 }, { QMetaType::QString, 12 },
        }}),
        // Method 'toggle'
        QtMocHelpers::MethodData<void(int)>(14, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 15 },
        }}),
        // Method 'selectAll'
        QtMocHelpers::MethodData<void(bool)>(16, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 17 },
        }}),
        // Method 'startSelected'
        QtMocHelpers::MethodData<void()>(18, 2, QMC::AccessPublic, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'busy'
        QtMocHelpers::PropertyData<bool>(19, QMetaType::Bool, QMC::DefaultPropertyFlags, 0),
        // property 'statusText'
        QtMocHelpers::PropertyData<QString>(20, QMetaType::QString, QMC::DefaultPropertyFlags, 1),
        // property 'lastError'
        QtMocHelpers::PropertyData<QString>(21, QMetaType::QString, QMC::DefaultPropertyFlags, 2),
        // property 'totalDuration'
        QtMocHelpers::PropertyData<qint64>(22, QMetaType::LongLong, QMC::DefaultPropertyFlags, 3),
        // property 'selectedCount'
        QtMocHelpers::PropertyData<int>(23, QMetaType::Int, QMC::DefaultPropertyFlags, 4),
        // property 'selectedBytes'
        QtMocHelpers::PropertyData<qint64>(24, QMetaType::LongLong, QMC::DefaultPropertyFlags, 5),
        // property 'unknownSizes'
        QtMocHelpers::PropertyData<int>(25, QMetaType::Int, QMC::DefaultPropertyFlags, 6),
        // property 'sourceUrl'
        QtMocHelpers::PropertyData<QString>(26, QMetaType::QString, QMC::DefaultPropertyFlags, 7),
    };
    QtMocHelpers::UintData qt_enums {
        // enum 'Role'
        QtMocHelpers::EnumData<enum Role>(27, 27, QMC::EnumFlags{}).add({
            {   28, Role::TitleRole },
            {   29, Role::ArtistRole },
            {   30, Role::UrlRole },
            {   31, Role::DurationRole },
            {   32, Role::SizeRole },
            {   33, Role::SelectedRole },
        }),
    };
    return QtMocHelpers::metaObjectData<ImportPreviewer, qt_meta_tag_ZN15ImportPreviewerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject ImportPreviewer::staticMetaObject = { {
    QMetaObject::SuperData::link<QAbstractListModel::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15ImportPreviewerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15ImportPreviewerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN15ImportPreviewerE_t>.metaTypes,
    nullptr
} };

void ImportPreviewer::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<ImportPreviewer *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->busyChanged(); break;
        case 1: _t->statusTextChanged(); break;
        case 2: _t->lastErrorChanged(); break;
        case 3: _t->totalDurationChanged(); break;
        case 4: _t->selectedCountChanged(); break;
        case 5: _t->selectedBytesChanged(); break;
        case 6: _t->unknownSizesChanged(); break;
        case 7: _t->sourceUrlChanged(); break;
        case 8: _t->inspect((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<bool>>(_a[3]))); break;
        case 9: _t->inspect((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 10: _t->toggle((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 11: _t->selectAll((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 12: _t->startSelected(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (ImportPreviewer::*)()>(_a, &ImportPreviewer::busyChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (ImportPreviewer::*)()>(_a, &ImportPreviewer::statusTextChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (ImportPreviewer::*)()>(_a, &ImportPreviewer::lastErrorChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (ImportPreviewer::*)()>(_a, &ImportPreviewer::totalDurationChanged, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (ImportPreviewer::*)()>(_a, &ImportPreviewer::selectedCountChanged, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (ImportPreviewer::*)()>(_a, &ImportPreviewer::selectedBytesChanged, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (ImportPreviewer::*)()>(_a, &ImportPreviewer::unknownSizesChanged, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (ImportPreviewer::*)()>(_a, &ImportPreviewer::sourceUrlChanged, 7))
            return;
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<bool*>(_v) = _t->busy(); break;
        case 1: *reinterpret_cast<QString*>(_v) = _t->statusText(); break;
        case 2: *reinterpret_cast<QString*>(_v) = _t->lastError(); break;
        case 3: *reinterpret_cast<qint64*>(_v) = _t->totalDuration(); break;
        case 4: *reinterpret_cast<int*>(_v) = _t->selectedCount(); break;
        case 5: *reinterpret_cast<qint64*>(_v) = _t->selectedBytes(); break;
        case 6: *reinterpret_cast<int*>(_v) = _t->unknownSizes(); break;
        case 7: *reinterpret_cast<QString*>(_v) = _t->sourceUrl(); break;
        default: break;
        }
    }
}

const QMetaObject *ImportPreviewer::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ImportPreviewer::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15ImportPreviewerE_t>.strings))
        return static_cast<void*>(this);
    return QAbstractListModel::qt_metacast(_clname);
}

int ImportPreviewer::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAbstractListModel::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 13)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 13)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 13;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void ImportPreviewer::busyChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void ImportPreviewer::statusTextChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void ImportPreviewer::lastErrorChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void ImportPreviewer::totalDurationChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void ImportPreviewer::selectedCountChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void ImportPreviewer::selectedBytesChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void ImportPreviewer::unknownSizesChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void ImportPreviewer::sourceUrlChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}
QT_WARNING_POP
