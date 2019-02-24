#include "historyitem.h"

bool HistoryItem::iconsInitialized = false;
QIcon HistoryItem::s_iconRunning;
QIcon HistoryItem::s_iconOk;
QIcon HistoryItem::s_iconError;
QIcon HistoryItem::s_iconCancelled;
QIcon HistoryItem::s_iconAlarm;
QIcon HistoryItem::s_iconReset;

void HistoryItem::initializeIcons(){
    s_iconRunning = QIcon(":/monitor-running");
    s_iconOk = QIcon(":/monitor-ok");
    s_iconError = QIcon(":/monitor-error");
    s_iconCancelled = QIcon(":/monitor-cancelled");
    s_iconAlarm = QIcon(":/monitor-alarm");
    s_iconReset = QIcon(":/monitor-reset");
    iconsInitialized = true;
}

HistoryItem::HistoryItem(const GrblInstruction &instruction, HistoryItem *parent):
    m_status(Running),
    m_parentItem(parent)
{
    m_instruction = new GrblInstruction(instruction);

    m_text = m_instruction->getStringWithLineNumber();

    if(!iconsInitialized){
        initializeIcons();
    }
}

HistoryItem::HistoryItem(const QString &caption, HistoryItem* parent):
    m_status(None),
    m_instruction(nullptr),
    m_text(caption),
    m_parentItem(parent)
{
    if(!iconsInitialized){
        initializeIcons();
    }
}

HistoryItem::ItemStatus HistoryItem::status() const {
    return m_status;
}

GrblInstruction* HistoryItem::instruction() const {
    return m_instruction;
}

void HistoryItem::setCancelled(){
    if(m_status == Running){
        m_status = Cancelled;
    }
}

void HistoryItem::setOk() {
    if(m_status != Alarm){
        m_status = Ok;
    }
}

void HistoryItem::setError() {
    if(m_status != Alarm){
        m_status = Error;
    }
}

void HistoryItem::setReset(){
    if(m_status == None){
        m_status = Reset;
    }
}

void HistoryItem::setAlarm(){
    m_status = Alarm;
}

void HistoryItem::addChild(HistoryItem* child){
    m_childItems.append(child);
}

void HistoryItem::deleteFirstChild(){
    if(!m_childItems.isEmpty()){
        delete m_childItems.takeFirst();
    }
}

bool HistoryItem::hasInstruction(void){
    return (m_instruction != nullptr);
}



QList<HistoryItem *> HistoryItem::childList(){
    return m_childItems;
}

HistoryItem *HistoryItem::lastChild(){
    if(m_childItems.isEmpty()){
        return nullptr;
    }
    return m_childItems.last();
}

int HistoryItem::childCount() const{
    return m_childItems.count();
}

HistoryItem* HistoryItem::searchItemMatchingInstruction(const GrblInstruction &instructionToLocate){
    //Check is this item has the matching instruction
    if(hasInstruction() && *m_instruction == instructionToLocate){
        return this;
    }

    //Else check every child
    foreach( HistoryItem* child, m_childItems){
        HistoryItem* childResult = child->searchItemMatchingInstruction(instructionToLocate);
        if(childResult != nullptr){
            return childResult;
        }
    }

    return nullptr;
}


int HistoryItem::columnCount() const{
    return 1;
}

QVariant HistoryItem::data(int role) const{
    switch(role){
    case Qt::DisplayRole:
        return m_text.simplified();
    case Qt::DecorationRole:{
        switch(m_status){
        case Running:
            return s_iconRunning;
        case Ok:
            return s_iconOk;
        case Error:
            return s_iconError;
        case Cancelled:
            return s_iconCancelled;
        case Alarm:
            return s_iconAlarm;
        case Reset:
            return s_iconReset;
        default:
            return QIcon();
        }
    }
    default:
        return QVariant();
    }
}

HistoryItem *HistoryItem::parentItem(){
    return m_parentItem;
}

int HistoryItem::row() const{
    if (m_parentItem){
        return m_parentItem->m_childItems.indexOf(const_cast<HistoryItem*>(this));
    }
    return 0;
}





HistoryItem::~HistoryItem(){
    if(m_instruction != nullptr){
        delete m_instruction;
    }
    qDeleteAll(m_childItems);
}
