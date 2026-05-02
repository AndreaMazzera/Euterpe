#pragma once
#include <QSortFilterProxyModel>

class PlaybackFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QString filterText READ filterText WRITE setFilterText NOTIFY filterTextChanged)

    public:
        explicit PlaybackFilterModel(QObject* parent = nullptr);

        void setFilterText(const QString &text);
        QString filterText() const;

    protected:
        bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

    signals:
        void filterTextChanged();

    private:
        QString mFilterText;
};
