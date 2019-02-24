#ifndef GRBLINSTRUCTION_H
#define GRBLINSTRUCTION_H

#include <QString>

class GrblInstruction
{
public:
    GrblInstruction(QString instruction = QString(), int lineNumber =-1);

    void forceBlocking();


    QByteArray getBytes() const {return m_instructionBytes;}
    int getLineNumber() const {return m_lineNumber;}
    bool isBlocking() const {return m_isBlocking;}

    int getLength() const {return m_instructionBytes.length();}
    QString getString() const {return QString::fromLatin1(m_instructionBytes);}

    QString getStringWithLineNumber();

    bool isParameterFetch(void) const;

    //Changes the instruction identifier, so it now become a different instruction with the same properties
    void regenerate();

    friend bool operator== (const GrblInstruction &i1, const GrblInstruction &i2);
    friend bool operator!= (const GrblInstruction &i1, const GrblInstruction &i2);

private:
    uint32_t m_uid;

    QByteArray m_instructionBytes;
    int m_lineNumber;
    bool m_isBlocking;

    bool isInherentlyBlocking();
    static const char *s_blockingInstructionsList[]; //List of EEPROM related instructions, requiring use of simpler "blocking" protocol

    static uint32_t s_uidCounter;
};

#endif // GRBLINSTRUCTION_H
