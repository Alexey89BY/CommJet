#pragma once
#include <QString>
#include <QByteArray>


namespace HexExParser
{
bool Encode(QByteArray const &srcData, QString &dstText, bool ForceHex = false);
bool Decode(QString const &srcText, QByteArray &dstData);
}
