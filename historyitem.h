#ifndef HISTORYITEM_H
#define HISTORYITEM_H


#include <QVariant>
#include <QIcon>
#include "grblinstruction.h"

class HistoryItem{
public:
    enum ItemStatus{None, Running, Ok, Error, Cancelled, Reset, Alarm};

    explicit HistoryItem(const GrblInstruction &instruction, HistoryItem *parent = 0);
    explicit HistoryItem(const QString &text = QStringLiteral(""), HistoryItem *parent = 0);
    ~HistoryItem();

    ItemStatus status() const;
    GrblInstruction *instruction() const;
    bool hasInstruction(void);

    void setCancelled();
    void setOk();
    void setError();
    void setReset();
    void setAlarm();

    void addChild(HistoryItem *child);
    void deleteFirstChild();

    QList<HistoryItem *> childList();
    HistoryItem *lastChild();


    //Find item corresponding to instruction
    HistoryItem* searchItemMatchingInstruction(const GrblInstruction &instructionToLocate);

    //Model View related
    int childCount() const;
    int columnCount() const;
    QVariant data(int role) const;
    int row() const;
    HistoryItem *parentItem();

private:

    ItemStatus m_status;
    GrblInstruction *m_instruction;
    QString m_text;

    QList<HistoryItem*> m_childItems;
    HistoryItem* m_parentItem;

    static QIcon s_iconRunning;
    static QIcon s_iconOk;
    static QIcon s_iconError;
    static QIcon s_iconCancelled;
    static QIcon s_iconAlarm;
    static QIcon s_iconReset;
    static bool iconsInitialized;
    static void initializeIcons();

};
#endif // HISTORYITEM_H
