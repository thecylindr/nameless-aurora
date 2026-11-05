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
        "summaryChanged",
        "trackTransferFinished",
        "trackId",
        "success",
        "source",
        "error",
        "enqueue",
        "urls",
        "quality",
        "authorized",
        "titles",
        "enqueueNative",
        "QVariantList",
        "tracks",
        "cancelJob",
        "id",
        "retryJob",
        "clearFinished",
        "cancelAll",
        "retryFailed",
        "openDiagnostics",
        "trackState",
        "QVariantMap",
        "downloadTrack",
        "downloadMissing",
        "enqueueAlternative",
        "candidate",
        "serviceForUrl",
        "url",
        "busy",
        "progress",
        "statusText",
        "lastError",
        "activeCount",
        "totalCount",
        "fileTotal",
        "filesReady",
        "failedCount",
        "succeededCount",
        "currentTitle",
        "currentService",
        "firstFailedTrack",
        "Role",
        "JobIdRole",
        "TitleRole",
        "UrlRole",
        "StatusRole",
        "ProgressRole",
        "ErrorRole",
        "ServiceRole",
        "TrackIdRole"
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
        // Signal 'summaryChanged'
        QtMocHelpers::SignalData<void()>(7, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'trackTransferFinished'
        QtMocHelpers::SignalData<void(int, bool, const QString &, const QString &)>(8, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 9 }, { QMetaType::Bool, 10 }, { QMetaType::QString, 11 }, { QMetaType::QString, 12 },
        }}),
        // Method 'enqueue'
        QtMocHelpers::MethodData<void(const QStringList &, const QString &, bool, const QStringList &)>(13, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QStringList, 14 }, { QMetaType::QString, 15 }, { QMetaType::Bool, 16 }, { QMetaType::QStringList, 17 },
        }}),
        // Method 'enqueue'
        QtMocHelpers::MethodData<void(const QStringList &, const QString &, bool)>(13, 2, QMC::AccessPublic | QMC::MethodCloned, QMetaType::Void, {{
            { QMetaType::QStringList, 14 }, { QMetaType::QString, 15 }, { QMetaType::Bool, 16 },
        }}),
        // Method 'enqueue'
        QtMocHelpers::MethodData<void(const QStringList &, const QString &)>(13, 2, QMC::AccessPublic | QMC::MethodCloned, QMetaType::Void, {{
            { QMetaType::QStringList, 14 }, { QMetaType::QString, 15 },
        }}),
        // Method 'enqueueNative'
        QtMocHelpers::MethodData<void(const QVariantList &, const QString &)>(18, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 19, 20 }, { QMetaType::QString, 15 },
        }}),
        // Method 'cancelJob'
        QtMocHelpers::MethodData<void(int)>(21, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 22 },
        }}),
        // Method 'retryJob'
        QtMocHelpers::MethodData<void(int)>(23, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 22 },
        }}),
        // Method 'clearFinished'
        QtMocHelpers::MethodData<void()>(24, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'cancelAll'
        QtMocHelpers::MethodData<void()>(25, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'retryFailed'
        QtMocHelpers::MethodData<void()>(26, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'openDiagnostics'
        QtMocHelpers::MethodData<void()>(27, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'trackState'
        QtMocHelpers::MethodData<QVariantMap(int) const>(28, 2, QMC::AccessPublic, 0x80000000 | 29, {{
            { QMetaType::Int, 9 },
        }}),
        // Method 'downloadTrack'
        QtMocHelpers::MethodData<void(int)>(30, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 9 },
        }}),
        // Method 'downloadMissing'
        QtMocHelpers::MethodData<void()>(31, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'enqueueAlternative'
        QtMocHelpers::MethodData<bool(int, const QVariantMap &, const QString &)>(32, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::Int, 9 }, { 0x80000000 | 29, 33 }, { QMetaType::QString, 15 },
        }}),
        // Method 'serviceForUrl'
        QtMocHelpers::MethodData<QString(const QString &) const>(34, 2, QMC::AccessPublic, QMetaType::QString, {{
            { QMetaType::QString, 35 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'busy'
        QtMocHelpers::PropertyData<bool>(36, QMetaType::Bool, QMC::DefaultPropertyFlags, 0),
        // property 'progress'
        QtMocHelpers::PropertyData<double>(37, QMetaType::Double, QMC::DefaultPropertyFlags, 1),
        // property 'statusText'
        QtMocHelpers::PropertyData<QString>(38, QMetaType::QString, QMC::DefaultPropertyFlags, 2),
        // property 'lastError'
        QtMocHelpers::PropertyData<QString>(39, QMetaType::QString, QMC::DefaultPropertyFlags, 3),
        // property 'activeCount'
        QtMocHelpers::PropertyData<int>(40, QMetaType::Int, QMC::DefaultPropertyFlags, 4),
        // property 'totalCount'
        QtMocHelpers::PropertyData<int>(41, QMetaType::Int, QMC::DefaultPropertyFlags, 5),
        // property 'fileTotal'
        QtMocHelpers::PropertyData<int>(42, QMetaType::Int, QMC::DefaultPropertyFlags, 5),
        // property 'filesReady'
        QtMocHelpers::PropertyData<int>(43, QMetaType::Int, QMC::DefaultPropertyFlags, 5),
        // property 'failedCount'
        QtMocHelpers::PropertyData<int>(44, QMetaType::Int, QMC::DefaultPropertyFlags, 5),
        // property 'succeededCount'
        QtMocHelpers::PropertyData<int>(45, QMetaType::Int, QMC::DefaultPropertyFlags, 5),
        // property 'currentTitle'
        QtMocHelpers::PropertyData<QString>(46, QMetaType::QString, QMC::DefaultPropertyFlags, 5),
        // property 'currentService'
        QtMocHelpers::PropertyData<QString>(47, QMetaType::QString, QMC::DefaultPropertyFlags, 5),
        // property 'firstFailedTrack'
        QtMocHelpers::PropertyData<int>(48, QMetaType::Int, QMC::DefaultPropertyFlags, 5),
    };
    QtMocHelpers::UintData qt_enums {
        // enum 'Role'
        QtMocHelpers::EnumData<enum Role>(49, 49, QMC::EnumFlags{}).add({
            {   50, Role::JobIdRole },
            {   51, Role::TitleRole },
            {   52, Role::UrlRole },
            {   53, Role::StatusRole },
            {   54, Role::ProgressRole },
            {   55, Role::ErrorRole },
            {   56, Role::ServiceRole },
            {   57, Role::TrackIdRole },
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
        case 5: _t->summaryChanged(); break;
        case 6: _t->trackTransferFinished((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<bool>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[3])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[4]))); break;
        case 7: _t->enqueue((*reinterpret_cast<std::add_pointer_t<QStringList>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<bool>>(_a[3])),(*reinterpret_cast<std::add_pointer_t<QStringList>>(_a[4]))); break;
        case 8: _t->enqueue((*reinterpret_cast<std::add_pointer_t<QStringList>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<bool>>(_a[3]))); break;
        case 9: _t->enqueue((*reinterpret_cast<std::add_pointer_t<QStringList>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 10: _t->enqueueNative((*reinterpret_cast<std::add_pointer_t<QVariantList>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 11: _t->cancelJob((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 12: _t->retryJob((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 13: _t->clearFinished(); break;
        case 14: _t->cancelAll(); break;
        case 15: _t->retryFailed(); break;
        case 16: _t->openDiagnostics(); break;
        case 17: { QVariantMap _r = _t->trackState((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast<QVariantMap*>(_a[0]) = std::move(_r); }  break;
        case 18: _t->downloadTrack((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 19: _t->downloadMissing(); break;
        case 20: { bool _r = _t->enqueueAlternative((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QVariantMap>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[3])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 21: { QString _r = _t->serviceForUrl((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])));
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
        if (QtMocHelpers::indexOfMethod<void (DownloadManager::*)()>(_a, &DownloadManager::summaryChanged, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (DownloadManager::*)(int , bool , const QString & , const QString & )>(_a, &DownloadManager::trackTransferFinished, 6))
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
        case 5: *reinterpret_cast<int*>(_v) = _t->totalCount(); break;
        case 6: *reinterpret_cast<int*>(_v) = _t->fileTotal(); break;
        case 7: *reinterpret_cast<int*>(_v) = _t->filesReady(); break;
        case 8: *reinterpret_cast<int*>(_v) = _t->failedCount(); break;
        case 9: *reinterpret_cast<int*>(_v) = _t->succeededCount(); break;
        case 10: *reinterpret_cast<QString*>(_v) = _t->currentTitle(); break;
        case 11: *reinterpret_cast<QString*>(_v) = _t->currentService(); break;
        case 12: *reinterpret_cast<int*>(_v) = _t->firstFailedTrack(); break;
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
        if (_id < 22)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 22;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 22)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 22;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
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

// SIGNAL 5
void DownloadManager::summaryChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void DownloadManager::trackTransferFinished(int _t1, bool _t2, const QString & _t3, const QString & _t4)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 6, nullptr, _t1, _t2, _t3, _t4);
}
QT_WARNING_POP
