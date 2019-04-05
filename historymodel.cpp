#include "historymodel.h"


#define DEFAULT_MAX_ITEM_COUNT  1000

HistoryModel::HistoryModel(QObject *parent):
    QAbstractItemModel(parent),
    m_maxItems(DEFAULT_MAX_ITEM_COUNT)
{
    m_rootItem = new HistoryItem();
}

void HistoryModel::setLength(int length){
    if(length > 0)
        m_maxItems = length;
}


void HistoryModel::addNewChild(HistoryItem* child, HistoryItem* parent){
    beginResetModel();
    //beginInsertRows(index(child->row(),0),parent->childCount(),parent->childCount());
    parent->addChild(child);
    //endInsertRows();

    //If parent is root item, we must handle here deletion of older items
    if(parent == m_rootItem && m_rootItem->childCount() > DEFAULT_MAX_ITEM_COUNT){
        //beginRemoveRows(QModelIndex(),0,0);
        m_rootItem->deleteFirstChild();
        //endRemoveRows();
    }
    endResetModel();

    emit modelUpdated(index(m_rootItem->childCount()-1,0));
}

void HistoryModel::onInstructionAcceptedInBoardCharBuffer(GrblInstruction instruction){
    //If not already aware of this instruction
    if(m_rootItem->searchItemMatchingInstruction(instruction) == nullptr){
        addNewChild(new HistoryItem(instruction,m_rootItem),m_rootItem);
    }
}




void HistoryModel::onOkReceived(GrblInstruction instruction){
    HistoryItem* matchingItem = m_rootItem->searchItemMatchingInstruction(instruction);

    //Set the item to OK state
    if(matchingItem){
        matchingItem->setOk();
        dataChanged(index(matchingItem->row(),0),index(matchingItem->row(),0));
    }
}

void HistoryModel::onErrorReceived(GrblInstruction instruction, QString errorMessage){
    HistoryItem* matchingItem = m_rootItem->searchItemMatchingInstruction(instruction);

    //Set the item to ERROR state and add message
    if(matchingItem){
        matchingItem->setError();
        addNewChild(new HistoryItem(errorMessage,matchingItem),matchingItem);
    }
}

void HistoryModel::onFeedbackReceived(GrblInstruction instruction, QString feedbackMessage){
    HistoryItem* matchingItem = m_rootItem->searchItemMatchingInstruction(instruction);

    //If feedback matches an instruction
    if(matchingItem){
        addNewChild(new HistoryItem(feedbackMessage,matchingItem),matchingItem);
    }

    //If it matches a RESET BOARD item
    else if(m_rootItem->lastChild() && m_rootItem->lastChild()->status() == HistoryItem::Reset){
        addNewChild(new HistoryItem(feedbackMessage,m_rootItem->lastChild()),m_rootItem->lastChild());
    }
}

void HistoryModel::onAlarmReceived(GrblInstruction instruction, QString alarmMessage){
    HistoryItem* matchingItem = m_rootItem->searchItemMatchingInstruction(instruction);

    //If alarm matches an instruction
    if(matchingItem){
        matchingItem->setAlarm();
        addNewChild(new HistoryItem(alarmMessage,matchingItem),matchingItem);
    }

    //If it matches a RESET BOARD item
    else if(m_rootItem->lastChild() && m_rootItem->lastChild()->status() == HistoryItem::Reset){
        m_rootItem->lastChild()->setAlarm();
        addNewChild(new HistoryItem(alarmMessage,m_rootItem->lastChild()),m_rootItem->lastChild());
    }

    //Else, it's an alarm, we have to display it even if could match it with an item, so create a new item
    else{
        HistoryItem* loneAlarmItem = new HistoryItem(alarmMessage,m_rootItem);
        loneAlarmItem->setAlarm();
        addNewChild(loneAlarmItem,m_rootItem);
    }
}

void HistoryModel::onBoardStartup(QString version, QList<GrblInstruction> instructionAtStartupList){
    //Cancel running instructions
    foreach(HistoryItem* item, m_rootItem->childList()){
        item->setCancelled();
    }


    //Add a BOARD RESET entry
    HistoryItem* resetItem = new HistoryItem("BOARD RESET COMPLETED",m_rootItem);
    resetItem->setReset();

    foreach(GrblInstruction startupInstruction, instructionAtStartupList){
        addNewChild(new HistoryItem(startupInstruction,resetItem),resetItem);
    }

    addNewChild(new HistoryItem(version,resetItem),resetItem);

    addNewChild(resetItem,m_rootItem);
}





//All those following are taken from Qt simple tree view example

int HistoryModel::columnCount(const QModelIndex &parent) const{
    if (parent.isValid()){
        return static_cast<HistoryItem*>(parent.internalPointer())->columnCount();
    }
    else{
        return m_rootItem->columnCount();
    }
}

QVariant HistoryModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    HistoryItem *item = static_cast<HistoryItem*>(index.internalPointer());

    return item->data(role);
}

Qt::ItemFlags HistoryModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return nullptr;

    return QAbstractItemModel::flags(index);
}

QVariant HistoryModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    Q_UNUSED(orientation);
    Q_UNUSED(role);
    Q_UNUSED(section);

    return QVariant();
}

QModelIndex HistoryModel::index(int row, int column, const QModelIndex &parent)
            const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    HistoryItem *parentItem;

    if (!parent.isValid())
        parentItem = m_rootItem;
    else
        parentItem = static_cast<HistoryItem*>(parent.internalPointer());

    HistoryItem *childItem = parentItem->childList().at(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex HistoryModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    HistoryItem *childItem = static_cast<HistoryItem*>(index.internalPointer());
    HistoryItem *parentItem = childItem->parentItem();

    if (parentItem == m_rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int HistoryModel::rowCount(const QModelIndex &parent) const
{
    HistoryItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = m_rootItem;
    else
        parentItem = static_cast<HistoryItem*>(parent.internalPointer());

    return parentItem->childCount();
}


HistoryModel::~HistoryModel(){
    delete m_rootItem;
}

