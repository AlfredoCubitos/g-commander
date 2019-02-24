#include "grblconfiguration.h"
#include <QRegularExpression>

const QRegularExpression GrblConfiguration::s_paramExpression = QRegularExpression("^\\$(?<key>[\\d]{1,3})=(?<value>[\\d][\\d|\\.]*)(\\s\\((?<caption>.+)\\))?$");


bool GrblConfiguration::isAValidParameter(QString string){
    QRegularExpressionMatch match = s_paramExpression.match(string);
    return match.hasMatch();
}

GrblConfiguration GrblConfiguration::fromString(QString string){
    GrblConfiguration param;
    QRegularExpressionMatch match = s_paramExpression.match(string);

    if(match.hasMatch()){
        //Now extract infos
        param.m_key = match.captured("key").toInt();
        param.m_value = match.captured("value");
        param.m_caption = match.captured("caption");
    }

    return param;
}


GrblConfiguration::GrblConfiguration():
    m_key(),
    m_value(),
    m_caption()
{

}
