/****************************************************************************
** Meta object code from reading C++ file 'downloadmanager.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.12.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/core/downloadmanager.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'downloadmanager.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN15DownloadManagerE_t {};
} // unnamed namespace

template <> constexpr inline auto DownloadManager::qt_create_metaobjectdata<qt_meta_tag_ZN15DownloadManagerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "DownloadManager",
        "busyChanged",
        "",
        "progressChanged",
        "statusTextChanged",
        "lastErrorChanged",
        "activeCountChanged",
        "enqueue",
        "urls",
        "quality",
        "authorized",
        "cancelJob",
        "id",
        "retryJob",
        "clearFinished",
        "serviceForUrl",
        "url",
        "busy",
        "progress",
        "statusText",
        "lastError",
        "activeCount",
        "Role",
        "JobIdRole",
        "TitleRole",
        "UrlRole",
        "StatusRole",
        "ProgressRole"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'busyChanged'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'progressChanged'
        QtMocHelpers::SignalData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'statusTextChanged'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'lastErrorChanged'
        QtMocHelpers::SignalData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'activeCountChanged'
        QtMocHelpers::SignalData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'enqueue'
        QtMocHelpers::MethodData<void(const QStringList &, const QString &, bool)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QStringList, 8 }, { QMetaType::QString, 9 }, { QMetaType::Bool, 10 },
        }}),
        // Method 'enqueue'
        QtMocHelpers::MethodData<void(const QStringList &, const QString &)>(7, 2, QMC::AccessPublic | QMC::MethodCloned, QMetaType::Void, {{
            { QMetaType::QStringList, 8 }, { QMetaType::QString, 9 },
        }}),
        // Method 'cancelJob'
        QtMocHelpers::MethodData<void(int)>(11, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 12 },
        }}),
        // Method 'retryJob'
        QtMocHelpers::MethodData<void(int)>(13, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 12 },
        }}),
        // Method 'clearFinished'
        QtMocHelpers::MethodData<void()>(14, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'serviceForUrl'
        QtMocHelpers::MethodData<QString(const QString &) const>(15, 2, QMC::AccessPublic, QMetaType::QString, {{
            { QMetaType::QString, 16 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'busy'
        QtMocHelpers::PropertyData<bool>(17, QMetaType::Bool, QMC::DefaultPropertyFlags, 0),
        // property 'progress'
        QtMocHelpers::PropertyData<double>(18, QMetaType::Double, QMC::DefaultPropertyFlags, 1),
        // property 'statusText'
        QtMocHelpers::PropertyData<QString>(19, QMetaType::QString, QMC::DefaultPropertyFlags, 2),
        // property 'lastError'
        QtMocHelpers::PropertyData<QString>(20, QMetaType::QString, QMC::DefaultPropertyFlags, 3),
        // property 'activeCount'
        QtMocHelpers::PropertyData<int>(21, QMetaType::Int, QMC::DefaultPropertyFlags, 4),
    };
    QtMocHelpers::UintData qt_enums {
        // enum 'Role'
        QtMocHelpers::EnumData<enum Role>(22, 22, QMC::EnumFlags{}).add({
            {   23, Role::JobIdRole },
            {   24, Role::TitleRole },
            {   25, Role::UrlRole },
            {   26, Role::StatusRole },
            {   27, Role::ProgressRole },
        }),
    };
    return QtMocHelpers::metaObjectData<DownloadManager, qt_meta_tag_ZN15DownloadManagerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject DownloadManager::staticMetaObject = { {
    QMetaObject::SuperData::link<QAbstractListModel::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15DownloadManagerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15DownloadManagerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN15DownloadManagerE_t>.metaTypes,
    nullptr
} };

void DownloadManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<DownloadManager *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->busyChanged(); break;
        case 1: _t->progressChanged(); break;
        case 2: _t->statusTextChanged(); break;
        case 3: _t->lastErrorChanged(); break;
        case 4: _t->activeCountChanged(); break;
        case 5: _t->enqueue((*reinterpret_cast<std::add_pointer_t<QStringList>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<bool>>(_a[3]))); break;
        case 6: _t->enqueue((*reinterpret_cast<std::add_pointer_t<QStringList>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 7: _t->cancelJob((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 8: _t->retryJob((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 9: _t->clearFinished(); break;
        case 10: { QString _r = _t->serviceForUrl((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast<QString*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (DownloadManager::*)()>(_a, &DownloadManager::busyChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (DownloadManager::*)()>(_a, &DownloadManager::progressChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (DownloadManager::*)()>(_a, &DownloadManager::statusTextChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (DownloadManager::*)()>(_a, &DownloadManager::lastErrorChanged, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (DownloadManager::*)()>(_a, &DownloadManager::activeCountChanged, 4))
            return;
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<bool*>(_v) = _t->busy(); break;
        case 1: *reinterpret_cast<double*>(_v) = _t->progress(); break;
        case 2: *reinterpret_cast<QString*>(_v) = _t->statusText(); break;
        case 3: *reinterpret_cast<QString*>(_v) = _t->lastError(); break;
        case 4: *reinterpret_cast<int*>(_v) = _t->activeCount(); break;
        default: break;
        }
    }
}

const QMetaObject *DownloadManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DownloadManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15DownloadManagerE_t>.strings))
        return static_cast<void*>(this);
    return QAbstractListModel::qt_metacast(_clname);
}

int DownloadManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAbstractListModel::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 11;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void DownloadManager::busyChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void DownloadManager::progressChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void DownloadManager::statusTextChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void DownloadManager::lastErrorChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void DownloadManager::activeCountChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}
QT_WARNING_POP
