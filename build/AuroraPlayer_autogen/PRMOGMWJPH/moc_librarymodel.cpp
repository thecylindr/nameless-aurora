/****************************************************************************
** Meta object code from reading C++ file 'librarymodel.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.12.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/core/librarymodel.h"
#include <QtNetwork/QSslError>
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
        "editRequested",
        "",
        "id",
        "albumEditRequested",
        "title",
        "countChanged",
        "filterChanged",
        "searchTextChanged",
        "lastErrorChanged",
        "libraryChanged",
        "queueDetails",
        "QVariantList",
        "ids",
        "trackDetails",
        "QVariantMap",
        "allTrackDetails",
        "updateMetadata",
        "artist",
        "album",
        "setTrackCover",
        "QUrl",
        "file",
        "setAlbumTracks",
        "previous",
        "cover",
        "requestEdit",
        "requestAlbumEdit",
        "createPlaylist",
        "name",
        "renamePlaylist",
        "deletePlaylist",
        "addToPlaylist",
        "playlist",
        "track",
        "removeFromPlaylist",
        "setRating",
        "rating",
        "removeTrack",
        "recommendations",
        "limit",
        "refresh",
        "count",
        "totalCount",
        "downloadedCount",
        "missingCount",
        "playlistFilter",
        "sortOrder",
        "playlists",
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
        "ListenMsRole",
        "AvailableRole"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'editRequested'
        QtMocHelpers::SignalData<void(int)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 3 },
        }}),
        // Signal 'albumEditRequested'
        QtMocHelpers::SignalData<void(const QString &)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 5 },
        }}),
        // Signal 'countChanged'
        QtMocHelpers::SignalData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'filterChanged'
        QtMocHelpers::SignalData<void()>(7, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'searchTextChanged'
        QtMocHelpers::SignalData<void()>(8, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'lastErrorChanged'
        QtMocHelpers::SignalData<void()>(9, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'libraryChanged'
        QtMocHelpers::SignalData<void()>(10, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'queueDetails'
        QtMocHelpers::MethodData<QVariantList(const QVariantList &) const>(11, 2, QMC::AccessPublic, 0x80000000 | 12, {{
            { 0x80000000 | 12, 13 },
        }}),
        // Method 'trackDetails'
        QtMocHelpers::MethodData<QVariantMap(int) const>(14, 2, QMC::AccessPublic, 0x80000000 | 15, {{
            { QMetaType::Int, 3 },
        }}),
        // Method 'allTrackDetails'
        QtMocHelpers::MethodData<QVariantList() const>(16, 2, QMC::AccessPublic, 0x80000000 | 12),
        // Method 'updateMetadata'
        QtMocHelpers::MethodData<bool(int, const QString &, const QString &, const QString &)>(17, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::Int, 3 }, { QMetaType::QString, 5 }, { QMetaType::QString, 18 }, { QMetaType::QString, 19 },
        }}),
        // Method 'setTrackCover'
        QtMocHelpers::MethodData<bool(int, const QUrl &)>(20, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::Int, 3 }, { 0x80000000 | 21, 22 },
        }}),
        // Method 'setAlbumTracks'
        QtMocHelpers::MethodData<bool(const QString &, const QString &, const QVariantList &, const QUrl &)>(23, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::QString, 24 }, { QMetaType::QString, 5 }, { 0x80000000 | 12, 13 }, { 0x80000000 | 21, 25 },
        }}),
        // Method 'setAlbumTracks'
        QtMocHelpers::MethodData<bool(const QString &, const QString &, const QVariantList &)>(23, 2, QMC::AccessPublic | QMC::MethodCloned, QMetaType::Bool, {{
            { QMetaType::QString, 24 }, { QMetaType::QString, 5 }, { 0x80000000 | 12, 13 },
        }}),
        // Method 'requestEdit'
        QtMocHelpers::MethodData<void(int)>(26, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 3 },
        }}),
        // Method 'requestAlbumEdit'
        QtMocHelpers::MethodData<void(const QString &)>(27, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 5 },
        }}),
        // Method 'requestAlbumEdit'
        QtMocHelpers::MethodData<void()>(27, 2, QMC::AccessPublic | QMC::MethodCloned, QMetaType::Void),
        // Method 'createPlaylist'
        QtMocHelpers::MethodData<int(const QString &)>(28, 2, QMC::AccessPublic, QMetaType::Int, {{
            { QMetaType::QString, 29 },
        }}),
        // Method 'renamePlaylist'
        QtMocHelpers::MethodData<void(int, const QString &)>(30, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 3 }, { QMetaType::QString, 29 },
        }}),
        // Method 'deletePlaylist'
        QtMocHelpers::MethodData<void(int)>(31, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 3 },
        }}),
        // Method 'addToPlaylist'
        QtMocHelpers::MethodData<void(int, int)>(32, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 33 }, { QMetaType::Int, 34 },
        }}),
        // Method 'removeFromPlaylist'
        QtMocHelpers::MethodData<void(int, int)>(35, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 33 }, { QMetaType::Int, 34 },
        }}),
        // Method 'setRating'
        QtMocHelpers::MethodData<void(int, int)>(36, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 3 }, { QMetaType::Int, 37 },
        }}),
        // Method 'removeTrack'
        QtMocHelpers::MethodData<void(int)>(38, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 3 },
        }}),
        // Method 'recommendations'
        QtMocHelpers::MethodData<QVariantList(int) const>(39, 2, QMC::AccessPublic, 0x80000000 | 12, {{
            { QMetaType::Int, 40 },
        }}),
        // Method 'recommendations'
        QtMocHelpers::MethodData<QVariantList() const>(39, 2, QMC::AccessPublic | QMC::MethodCloned, 0x80000000 | 12),
        // Method 'refresh'
        QtMocHelpers::MethodData<void()>(41, 2, QMC::AccessPublic, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'count'
        QtMocHelpers::PropertyData<int>(42, QMetaType::Int, QMC::DefaultPropertyFlags, 2),
        // property 'totalCount'
        QtMocHelpers::PropertyData<int>(43, QMetaType::Int, QMC::DefaultPropertyFlags, 6),
        // property 'downloadedCount'
        QtMocHelpers::PropertyData<int>(44, QMetaType::Int, QMC::DefaultPropertyFlags, 6),
        // property 'missingCount'
        QtMocHelpers::PropertyData<int>(45, QMetaType::Int, QMC::DefaultPropertyFlags, 6),
        // property 'playlistFilter'
        QtMocHelpers::PropertyData<int>(46, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 3),
        // property 'sortOrder'
        QtMocHelpers::PropertyData<QString>(47, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 3),
        // property 'playlists'
        QtMocHelpers::PropertyData<QVariantList>(48, 0x80000000 | 12, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 6),
        // property 'searchText'
        QtMocHelpers::PropertyData<QString>(49, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 4),
        // property 'lastError'
        QtMocHelpers::PropertyData<QString>(50, QMetaType::QString, QMC::DefaultPropertyFlags, 5),
    };
    QtMocHelpers::UintData qt_enums {
        // enum 'Role'
        QtMocHelpers::EnumData<enum Role>(51, 51, QMC::EnumFlags{}).add({
            {   52, Role::IdRole },
            {   53, Role::TitleRole },
            {   54, Role::ArtistRole },
            {   55, Role::AlbumRole },
            {   56, Role::FilePathRole },
            {   57, Role::SourceUrlRole },
            {   58, Role::SourceRole },
            {   59, Role::FormatRole },
            {   60, Role::CoverPathRole },
            {   61, Role::DurationRole },
            {   62, Role::BitrateRole },
            {   63, Role::RatingRole },
            {   64, Role::PlayCountRole },
            {   65, Role::LastPlayedRole },
            {   66, Role::SkipCountRole },
            {   67, Role::ListenMsRole },
            {   68, Role::AvailableRole },
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
        case 0: _t->editRequested((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 1: _t->albumEditRequested((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 2: _t->countChanged(); break;
        case 3: _t->filterChanged(); break;
        case 4: _t->searchTextChanged(); break;
        case 5: _t->lastErrorChanged(); break;
        case 6: _t->libraryChanged(); break;
        case 7: { QVariantList _r = _t->queueDetails((*reinterpret_cast<std::add_pointer_t<QVariantList>>(_a[1])));
            if (_a[0]) *reinterpret_cast<QVariantList*>(_a[0]) = std::move(_r); }  break;
        case 8: { QVariantMap _r = _t->trackDetails((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast<QVariantMap*>(_a[0]) = std::move(_r); }  break;
        case 9: { QVariantList _r = _t->allTrackDetails();
            if (_a[0]) *reinterpret_cast<QVariantList*>(_a[0]) = std::move(_r); }  break;
        case 10: { bool _r = _t->updateMetadata((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[3])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[4])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 11: { bool _r = _t->setTrackCover((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QUrl>>(_a[2])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 12: { bool _r = _t->setAlbumTracks((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<QVariantList>>(_a[3])),(*reinterpret_cast<std::add_pointer_t<QUrl>>(_a[4])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 13: { bool _r = _t->setAlbumTracks((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<QVariantList>>(_a[3])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 14: _t->requestEdit((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 15: _t->requestAlbumEdit((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 16: _t->requestAlbumEdit(); break;
        case 17: { int _r = _t->createPlaylist((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast<int*>(_a[0]) = std::move(_r); }  break;
        case 18: _t->renamePlaylist((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 19: _t->deletePlaylist((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 20: _t->addToPlaylist((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2]))); break;
        case 21: _t->removeFromPlaylist((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2]))); break;
        case 22: _t->setRating((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2]))); break;
        case 23: _t->removeTrack((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 24: { QVariantList _r = _t->recommendations((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast<QVariantList*>(_a[0]) = std::move(_r); }  break;
        case 25: { QVariantList _r = _t->recommendations();
            if (_a[0]) *reinterpret_cast<QVariantList*>(_a[0]) = std::move(_r); }  break;
        case 26: _t->refresh(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (LibraryModel::*)(int )>(_a, &LibraryModel::editRequested, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (LibraryModel::*)(const QString & )>(_a, &LibraryModel::albumEditRequested, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (LibraryModel::*)()>(_a, &LibraryModel::countChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (LibraryModel::*)()>(_a, &LibraryModel::filterChanged, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (LibraryModel::*)()>(_a, &LibraryModel::searchTextChanged, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (LibraryModel::*)()>(_a, &LibraryModel::lastErrorChanged, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (LibraryModel::*)()>(_a, &LibraryModel::libraryChanged, 6))
            return;
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<int*>(_v) = _t->count(); break;
        case 1: *reinterpret_cast<int*>(_v) = _t->totalCount(); break;
        case 2: *reinterpret_cast<int*>(_v) = _t->downloadedCount(); break;
        case 3: *reinterpret_cast<int*>(_v) = _t->missingCount(); break;
        case 4: *reinterpret_cast<int*>(_v) = _t->playlistFilter(); break;
        case 5: *reinterpret_cast<QString*>(_v) = _t->sortOrder(); break;
        case 6: *reinterpret_cast<QVariantList*>(_v) = _t->playlists(); break;
        case 7: *reinterpret_cast<QString*>(_v) = _t->searchText(); break;
        case 8: *reinterpret_cast<QString*>(_v) = _t->lastError(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 4: _t->setPlaylistFilter(*reinterpret_cast<int*>(_v)); break;
        case 5: _t->setSortOrder(*reinterpret_cast<QString*>(_v)); break;
        case 7: _t->setSearchText(*reinterpret_cast<QString*>(_v)); break;
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
        if (_id < 27)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 27;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 27)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 27;
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
void LibraryModel::editRequested(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void LibraryModel::albumEditRequested(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void LibraryModel::countChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void LibraryModel::filterChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void LibraryModel::searchTextChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void LibraryModel::lastErrorChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void LibraryModel::libraryChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}
QT_WARNING_POP
