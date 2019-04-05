#ifndef GRBLCONFIGURATION_H
#define GRBLCONFIGURATION_H

#include <QString>
#include <QVariant>


class GrblConfiguration
{
public:
    GrblConfiguration();

    int getKey() const {return m_key;}
    QVariant getValue() const {return m_value;}
    QString getCaption() const {return m_caption;}

    static bool isAValidParameter(const QString &string);
    static GrblConfiguration fromString(const QString &string);

private:
    int m_key;
    QVariant m_value;
    QString m_caption;

    static const QRegularExpression s_paramExpression;
};

#endif // GRBLCONFIGURATION_H
