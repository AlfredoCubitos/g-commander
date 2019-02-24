#ifndef HISTORYMODEL_H
#define HISTORYMODEL_H



#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include "grblinstruction.h"
#include "historyitem.h"

class HistoryModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit HistoryModel(QObject *parent = 0);
    ~HistoryModel();

    QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
    Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QModelIndex parent(const QModelIndex &index) const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

    void setLength(int length);

signals:
    void modelUpdated(QModelIndex lastItem);

public slots:
    void onBoardStartup(QString version, QList<GrblInstruction> instructionAtStartupList);

    void onInstructionAcceptedInBoardCharBuffer(GrblInstruction instruction);

    void onOkReceived(GrblInstruction instruction);
    void onErrorReceived( GrblInstruction instruction, QString errorMessage);
    void onFeedbackReceived(GrblInstruction instruction, QString feedbackMessage);
    void onAlarmReceived(GrblInstruction instruction, QString alarmMessage);


private:
    void addNewChild(HistoryItem *child, HistoryItem* parent);

    HistoryItem* m_rootItem;

    int m_maxItems;
};

#endif // HISTORYMODEL_H
