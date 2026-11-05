/****************************************************************************
** Meta object code from reading C++ file 'browsersession.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.12.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/core/auth/browsersession.h"
#include <QtNetwork/QSslError>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'browsersession.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN14BrowserSessionE_t {};
} // unnamed namespace

template <> constexpr inline auto BrowserSession::qt_create_metaobjectdata<qt_meta_tag_ZN14BrowserSessionE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "BrowserSession",
        "loaded",
        "",
        "QUrl",
        "url",
        "ok",
        "error",
        "evaluated",
        "requestId",
        "QVariant",
        "result",
        "cookiesReceived",
        "service",
        "QJsonArray",
        "cookies",
        "audioRequested",
        "apiReceived",
        "QJsonValue",
        "payload",
        "apiFetched",
        "status",
        "failed"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'loaded'
        QtMocHelpers::SignalData<void(const QUrl &, bool, const QString &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 }, { QMetaType::Bool, 5 }, { QMetaType::QString, 6 },
        }}),
        // Signal 'evaluated'
        QtMocHelpers::SignalData<void(int, const QVariant &)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 8 }, { 0x80000000 | 9, 10 },
        }}),
        // Signal 'cookiesReceived'
        QtMocHelpers::SignalData<void(const QString &, const QJsonArray &)>(11, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 12 }, { 0x80000000 | 13, 14 },
        }}),
        // Signal 'audioRequested'
        QtMocHelpers::SignalData<void(const QUrl &)>(15, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
        // Signal 'apiReceived'
        QtMocHelpers::SignalData<void(const QUrl &, const QJsonValue &)>(16, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 }, { 0x80000000 | 17, 18 },
        }}),
        // Signal 'apiFetched'
        QtMocHelpers::SignalData<void(int, const QUrl &, const QJsonValue &, int)>(19, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 8 }, { 0x80000000 | 3, 4 }, { 0x80000000 | 17, 18 }, { QMetaType::Int, 20 },
        }}),
        // Signal 'failed'
        QtMocHelpers::SignalData<void(const QString &)>(21, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 6 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<BrowserSession, qt_meta_tag_ZN14BrowserSessionE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject BrowserSession::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14BrowserSessionE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14BrowserSessionE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN14BrowserSessionE_t>.metaTypes,
    nullptr
} };

void BrowserSession::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<BrowserSession *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->loaded((*reinterpret_cast<std::add_pointer_t<QUrl>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<bool>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[3]))); break;
        case 1: _t->evaluated((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QVariant>>(_a[2]))); break;
        case 2: _t->cookiesReceived((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QJsonArray>>(_a[2]))); break;
        case 3: _t->audioRequested((*reinterpret_cast<std::add_pointer_t<QUrl>>(_a[1]))); break;
        case 4: _t->apiReceived((*reinterpret_cast<std::add_pointer_t<QUrl>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QJsonValue>>(_a[2]))); break;
        case 5: _t->apiFetched((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QUrl>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<QJsonValue>>(_a[3])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[4]))); break;
        case 6: _t->failed((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (BrowserSession::*)(const QUrl & , bool , const QString & )>(_a, &BrowserSession::loaded, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (BrowserSession::*)(int , const QVariant & )>(_a, &BrowserSession::evaluated, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (BrowserSession::*)(const QString & , const QJsonArray & )>(_a, &BrowserSession::cookiesReceived, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (BrowserSession::*)(const QUrl & )>(_a, &BrowserSession::audioRequested, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (BrowserSession::*)(const QUrl & , const QJsonValue & )>(_a, &BrowserSession::apiReceived, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (BrowserSession::*)(int , const QUrl & , const QJsonValue & , int )>(_a, &BrowserSession::apiFetched, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (BrowserSession::*)(const QString & )>(_a, &BrowserSession::failed, 6))
            return;
    }
}

const QMetaObject *BrowserSession::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *BrowserSession::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14BrowserSessionE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int BrowserSession::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void BrowserSession::loaded(const QUrl & _t1, bool _t2, const QString & _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1, _t2, _t3);
}

// SIGNAL 1
void BrowserSession::evaluated(int _t1, const QVariant & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1, _t2);
}

// SIGNAL 2
void BrowserSession::cookiesReceived(const QString & _t1, const QJsonArray & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1, _t2);
}

// SIGNAL 3
void BrowserSession::audioRequested(const QUrl & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1);
}

// SIGNAL 4
void BrowserSession::apiReceived(const QUrl & _t1, const QJsonValue & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1, _t2);
}

// SIGNAL 5
void BrowserSession::apiFetched(int _t1, const QUrl & _t2, const QJsonValue & _t3, int _t4)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 5, nullptr, _t1, _t2, _t3, _t4);
}

// SIGNAL 6
void BrowserSession::failed(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 6, nullptr, _t1);
}
QT_WARNING_POP
