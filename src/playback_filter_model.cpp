#include "include/playback_filter_model.h"
#include "include/song_model.h"

PlaybackFilterModel::PlaybackFilterModel(QObject* parent) : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
    setSortRole(SongModel::SortRole);
    setSortCaseSensitivity(Qt::CaseInsensitive);
    setFilterCaseSensitivity(Qt::CaseInsensitive);
    sort(0, Qt::AscendingOrder);
}

void PlaybackFilterModel::setFilterText(const QString &text)
{
    if (mFilterText == text)
        return;

    mFilterText = text;
    setFilterFixedString(text);

    emit filterTextChanged();
}

QString PlaybackFilterModel::filterText() const
{
    return mFilterText;
}

bool PlaybackFilterModel::filterAcceptsRow(int source_row, const QModelIndex &parent) const
{
    if (mFilterText.isEmpty())
        return true;

    if (!sourceModel())
        return false;

    QModelIndex index = sourceModel()->index(source_row, 0, parent);

    QString searchKey = sourceModel()->data(index, SongModel::SortRole).toString();
    QString album = sourceModel()->data(index, SongModel::AlbumRole).toString();

    return searchKey.contains(mFilterText, Qt::CaseInsensitive) ||
           album.contains(mFilterText, Qt::CaseInsensitive);
}
