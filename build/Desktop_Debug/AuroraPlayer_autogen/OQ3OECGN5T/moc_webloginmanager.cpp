/****************************************************************************
** Meta object code from reading C++ file 'webloginmanager.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.12.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../src/core/auth/webloginmanager.h"
#include <QtNetwork/QSslError>
#include <QtCore/qmetatype.h>
#include <QtCore/QList>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'webloginmanager.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN15WebLoginManagerE_t {};
} // unnamed namespace

template <> constexpr inline auto WebLoginManager::qt_create_metaobjectdata<qt_meta_tag_ZN15WebLoginManagerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "WebLoginManager",
        "nativeBrowserChanged",
        "",
        "navigateRequested",
        "QUrl",
        "url",
        "scriptRequested",
        "id",
        "script",
        "collectionReady",
        "QList<Track>",
        "tracks",
        "name",
        "error",
        "searchReady",
        "mediaReady",
        "requestId",
        "busyChanged",
        "statusChanged",
        "sessionsChanged",
        "loginDone",
        "service",
        "ok",
        "message",
        "openImportReport",
        "openNative",
        "setBrowserVisible",
        "visible",
        "pageLoaded",
        "confirmNativeLogin",
        "submitScriptResult",
        "QVariant",
        "result",
        "localBrowserProfiles",
        "QVariantList",
        "availableBrowsers",
        "startLogin",
        "browserIndex",
        "cancel",
        "logout",
        "hasSession",
        "awaitingLogin",
        "nativeProfile",
        "QQuickWebEngineProfile*",
        "browserVisible",
        "pageUrl",
        "pageError",
        "nativeService",
        "busy",
        "status",
        "importReport",
        "vkLoggedIn",
        "yandexLoggedIn"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'nativeBrowserChanged'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'navigateRequested'
        QtMocHelpers::SignalData<void(const QUrl &)>(3, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 4, 5 },
        }}),
        // Signal 'scriptRequested'
        QtMocHelpers::SignalData<void(int, const QString &)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 7 }, { QMetaType::QString, 8 },
        }}),
        // Signal 'collectionReady'
        QtMocHelpers::SignalData<void(const QVector<Track> &, const QString &, const QString &)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 10, 11 }, { QMetaType::QString, 12 }, { QMetaType::QString, 13 },
        }}),
        // Signal 'searchReady'
        QtMocHelpers::SignalData<void(const QVector<Track> &, const QString &)>(14, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 10, 11 }, { QMetaType::QString, 13 },
        }}),
        // Signal 'mediaReady'
        QtMocHelpers::SignalData<void(int, const QUrl &, const QString &)>(15, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 16 }, { 0x80000000 | 4, 5 }, { QMetaType::QString, 13 },
        }}),
        // Signal 'busyChanged'
        QtMocHelpers::SignalData<void()>(17, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'statusChanged'
        QtMocHelpers::SignalData<void()>(18, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'sessionsChanged'
        QtMocHelpers::SignalData<void()>(19, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'loginDone'
        QtMocHelpers::SignalData<void(const QString &, bool, const QString &)>(20, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 21 }, { QMetaType::Bool, 22 }, { QMetaType::QString, 23 },
        }}),
        // Method 'openImportReport'
        QtMocHelpers::MethodData<void()>(24, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'openNative'
        QtMocHelpers::MethodData<void(const QString &)>(25, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 21 },
        }}),
        // Method 'setBrowserVisible'
        QtMocHelpers::MethodData<void(bool)>(26, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 27 },
        }}),
        // Method 'pageLoaded'
        QtMocHelpers::MethodData<void(const QUrl &, bool, const QString &)>(28, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 4, 5 }, { QMetaType::Bool, 22 }, { QMetaType::QString, 13 },
        }}),
        // Method 'pageLoaded'
        QtMocHelpers::MethodData<void(const QUrl &, bool)>(28, 2, QMC::AccessPublic | QMC::MethodCloned, QMetaType::Void, {{
            { 0x80000000 | 4, 5 }, { QMetaType::Bool, 22 },
        }}),
        // Method 'confirmNativeLogin'
        QtMocHelpers::MethodData<void()>(29, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'submitScriptResult'
        QtMocHelpers::MethodData<void(int, const QVariant &)>(30, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 7 }, { 0x80000000 | 31, 32 },
        }}),
        // Method 'localBrowserProfiles'
        QtMocHelpers::MethodData<QVariantList() const>(33, 2, QMC::AccessPublic, 0x80000000 | 34),
        // Method 'availableBrowsers'
        QtMocHelpers::MethodData<QStringList()>(35, 2, QMC::AccessPublic, QMetaType::QStringList),
        // Method 'startLogin'
        QtMocHelpers::MethodData<void(QString, int)>(36, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 21 }, { QMetaType::Int, 37 },
        }}),
        // Method 'cancel'
        QtMocHelpers::MethodData<void()>(38, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'logout'
        QtMocHelpers::MethodData<void(QString)>(39, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 21 },
        }}),
        // Method 'hasSession'
        QtMocHelpers::MethodData<bool(QString) const>(40, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::QString, 21 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'awaitingLogin'
        QtMocHelpers::PropertyData<bool>(41, QMetaType::Bool, QMC::DefaultPropertyFlags, 0),
        // property 'nativeProfile'
        QtMocHelpers::PropertyData<QQuickWebEngineProfile*>(42, 0x80000000 | 43, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 0),
        // property 'browserVisible'
        QtMocHelpers::PropertyData<bool>(44, QMetaType::Bool, QMC::DefaultPropertyFlags, 0),
        // property 'pageUrl'
        QtMocHelpers::PropertyData<QUrl>(45, 0x80000000 | 4, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 0),
        // property 'pageError'
        QtMocHelpers::PropertyData<QString>(46, QMetaType::QString, QMC::DefaultPropertyFlags, 0),
        // property 'nativeService'
        QtMocHelpers::PropertyData<QString>(47, QMetaType::QString, QMC::DefaultPropertyFlags, 0),
        // property 'busy'
        QtMocHelpers::PropertyData<bool>(48, QMetaType::Bool, QMC::DefaultPropertyFlags, 6),
        // property 'status'
        QtMocHelpers::PropertyData<QString>(49, QMetaType::QString, QMC::DefaultPropertyFlags, 7),
        // property 'importReport'
        QtMocHelpers::PropertyData<QString>(50, QMetaType::QString, QMC::DefaultPropertyFlags, 0),
        // property 'vkLoggedIn'
        QtMocHelpers::PropertyData<bool>(51, QMetaType::Bool, QMC::DefaultPropertyFlags, 8),
        // property 'yandexLoggedIn'
        QtMocHelpers::PropertyData<bool>(52, QMetaType::Bool, QMC::DefaultPropertyFlags, 8),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<WebLoginManager, qt_meta_tag_ZN15WebLoginManagerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject WebLoginManager::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15WebLoginManagerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15WebLoginManagerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN15WebLoginManagerE_t>.metaTypes,
    nullptr
} };

void WebLoginManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<WebLoginManager *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->nativeBrowserChanged(); break;
        case 1: _t->navigateRequested((*reinterpret_cast<std::add_pointer_t<QUrl>>(_a[1]))); break;
        case 2: _t->scriptRequested((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 3: _t->collectionReady((*reinterpret_cast<std::add_pointer_t<QList<Track>>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[3]))); break;
        case 4: _t->searchReady((*reinterpret_cast<std::add_pointer_t<QList<Track>>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 5: _t->mediaReady((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QUrl>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[3]))); break;
        case 6: _t->busyChanged(); break;
        case 7: _t->statusChanged(); break;
        case 8: _t->sessionsChanged(); break;
        case 9: _t->loginDone((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<bool>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[3]))); break;
        case 10: _t->openImportReport(); break;
        case 11: _t->openNative((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 12: _t->setBrowserVisible((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 13: _t->pageLoaded((*reinterpret_cast<std::add_pointer_t<QUrl>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<bool>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[3]))); break;
        case 14: _t->pageLoaded((*reinterpret_cast<std::add_pointer_t<QUrl>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<bool>>(_a[2]))); break;
        case 15: _t->confirmNativeLogin(); break;
        case 16: _t->submitScriptResult((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QVariant>>(_a[2]))); break;
        case 17: { QVariantList _r = _t->localBrowserProfiles();
            if (_a[0]) *reinterpret_cast<QVariantList*>(_a[0]) = std::move(_r); }  break;
        case 18: { QStringList _r = _t->availableBrowsers();
            if (_a[0]) *reinterpret_cast<QStringList*>(_a[0]) = std::move(_r); }  break;
        case 19: _t->startLogin((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2]))); break;
        case 20: _t->cancel(); break;
        case 21: _t->logout((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 22: { bool _r = _t->hasSession((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (WebLoginManager::*)()>(_a, &WebLoginManager::nativeBrowserChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (WebLoginManager::*)(const QUrl & )>(_a, &WebLoginManager::navigateRequested, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (WebLoginManager::*)(int , const QString & )>(_a, &WebLoginManager::scriptRequested, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (WebLoginManager::*)(const QVector<Track> & , const QString & , const QString & )>(_a, &WebLoginManager::collectionReady, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (WebLoginManager::*)(const QVector<Track> & , const QString & )>(_a, &WebLoginManager::searchReady, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (WebLoginManager::*)(int , const QUrl & , const QString & )>(_a, &WebLoginManager::mediaReady, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (WebLoginManager::*)()>(_a, &WebLoginManager::busyChanged, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (WebLoginManager::*)()>(_a, &WebLoginManager::statusChanged, 7))
            return;
        if (QtMocHelpers::indexOfMethod<void (WebLoginManager::*)()>(_a, &WebLoginManager::sessionsChanged, 8))
            return;
        if (QtMocHelpers::indexOfMethod<void (WebLoginManager::*)(const QString & , bool , const QString & )>(_a, &WebLoginManager::loginDone, 9))
            return;
    }
    if (_c == QMetaObject::RegisterPropertyMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 1:
            *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QQuickWebEngineProfile* >(); break;
        }
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<bool*>(_v) = _t->awaitingLogin(); break;
        case 1: *reinterpret_cast<QQuickWebEngineProfile**>(_v) = _t->nativeProfile(); break;
        case 2: *reinterpret_cast<bool*>(_v) = _t->browserVisible(); break;
        case 3: *reinterpret_cast<QUrl*>(_v) = _t->pageUrl(); break;
        case 4: *reinterpret_cast<QString*>(_v) = _t->pageError(); break;
        case 5: *reinterpret_cast<QString*>(_v) = _t->nativeService(); break;
        case 6: *reinterpret_cast<bool*>(_v) = _t->busy(); break;
        case 7: *reinterpret_cast<QString*>(_v) = _t->status(); break;
        case 8: *reinterpret_cast<QString*>(_v) = _t->importReport(); break;
        case 9: *reinterpret_cast<bool*>(_v) = _t->vkLoggedIn(); break;
        case 10: *reinterpret_cast<bool*>(_v) = _t->yandexLoggedIn(); break;
        default: break;
        }
    }
}

const QMetaObject *WebLoginManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *WebLoginManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15WebLoginManagerE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int WebLoginManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 23)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 23;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 23)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 23;
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
void WebLoginManager::nativeBrowserChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void WebLoginManager::navigateRequested(const QUrl & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void WebLoginManager::scriptRequested(int _t1, const QString & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1, _t2);
}

// SIGNAL 3
void WebLoginManager::collectionReady(const QVector<Track> & _t1, const QString & _t2, const QString & _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1, _t2, _t3);
}

// SIGNAL 4
void WebLoginManager::searchReady(const QVector<Track> & _t1, const QString & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1, _t2);
}

// SIGNAL 5
void WebLoginManager::mediaReady(int _t1, const QUrl & _t2, const QString & _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 5, nullptr, _t1, _t2, _t3);
}

// SIGNAL 6
void WebLoginManager::busyChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void WebLoginManager::statusChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}

// SIGNAL 8
void WebLoginManager::sessionsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 8, nullptr);
}

// SIGNAL 9
void WebLoginManager::loginDone(const QString & _t1, bool _t2, const QString & _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 9, nullptr, _t1, _t2, _t3);
}
QT_WARNING_POP
