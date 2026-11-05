#pragma once
#include <QString>
#include <QStringConverter>
#include <QByteArray>
namespace VkText
{
// Таблица соответствия CP1251 → UTF-16 для декодирования кириллицы из веб-ответов VK.
inline constexpr char16_t cp1251[] = {
    0x0402, 0x0403, 0x201a, 0x0453, 0x201e, 0x2026, 0x2020, 0x2021, 0x20ac, 0x2030, 0x0409, 0x2039, 0x040a,
    0x040c, 0x040b, 0x040f, 0x0452, 0x2018, 0x2019, 0x201c, 0x201d, 0x2022, 0x2013, 0x2014, 0xfffd, 0x2122,
    0x0459, 0x203a, 0x045a, 0x045c, 0x045b, 0x045f, 0x00a0, 0x040e, 0x045e, 0x0408, 0x00a4, 0x0490, 0x00a6,
    0x00a7, 0x0401, 0x00a9, 0x0404, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x0407, 0x00b0, 0x00b1, 0x0406, 0x0456,
    0x0491, 0x00b5, 0x00b6, 0x00b7, 0x0451, 0x2116, 0x0454, 0x00bb, 0x0458, 0x0405, 0x0455, 0x0457, 0x0410,
    0x0411, 0x0412, 0x0413, 0x0414, 0x0415, 0x0416, 0x0417, 0x0418, 0x0419, 0x041a, 0x041b, 0x041c, 0x041d,
    0x041e, 0x041f, 0x0420, 0x0421, 0x0422, 0x0423, 0x0424, 0x0425, 0x0426, 0x0427, 0x0428, 0x0429, 0x042a,
    0x042b, 0x042c, 0x042d, 0x042e, 0x042f, 0x0430, 0x0431, 0x0432, 0x0433, 0x0434, 0x0435, 0x0436, 0x0437,
    0x0438, 0x0439, 0x043a, 0x043b, 0x043c, 0x043d, 0x043e, 0x043f, 0x0440, 0x0441, 0x0442, 0x0443, 0x0444,
    0x0445, 0x0446, 0x0447, 0x0448, 0x0449, 0x044a, 0x044b, 0x044c, 0x044d, 0x044e, 0x044f};
// Декодирует байты как UTF-8, а при неудаче — как CP1251 (VK иногда отдаёт кириллицу в этой кодировке).
inline QString decode(const QByteArray& bytes)
{
    QStringDecoder utf8(QStringDecoder::Utf8);
    const QString unicode = utf8(bytes);
    if (!utf8.hasError())
        return unicode;
    QString text;
    text.reserve(bytes.size());
    for (const auto b : bytes) {
        const auto c = static_cast<unsigned char>(b);
        text.append(QChar(c < 128 ? char16_t(c) : cp1251[c - 128]));
    }
    return text;
}
inline QString clean(QString text)
{
    // Откатываем ошибочную конверсию UTF-8 → CP1251/Latin1, но только без потерь.
    for (int pass = 0; pass < 2; ++pass) {
        QString best = text;
        // Пробуем оба направления: как Latin1, так и CP1251.
        for (bool legacy : {false, true}) {
            QByteArray bytes;
            bool representable = true;
            for (const auto c : text) {
                int value = c.unicode();
                // В режиме legacy ищем символ в таблице CP1251 и берём его байт.
                if (legacy && value >= 128) {
                    value = -1;
                    for (int i = 0; i < 128; ++i)
                        if (cp1251[i] == c.unicode()) {
                            value = i + 128;
                            break;
                        }
                } else if (value > 255)
                    value = -1;
                if (value < 0) {
                    representable = false;
                    break;
                }
                bytes.append(char(value));
            }
            if (!representable)
                continue;
            QStringDecoder decoder(QStringDecoder::Utf8);
            const QString decoded = decoder(bytes);
            // Принимаем результат только если расшифровка чистая и не раздула строку сверх меры.
            if (!decoder.hasError() && !decoded.contains(QChar(0xfffd)) && decoded.size() * 4 <= text.size() * 3)
                best = decoded;
        }
        if (best == text)
            break;
        text = best;
    }
    return text.normalized(QString::NormalizationForm_C);
}
// Возвращает true, если текст содержит нераскодированные символы или искажённую кодировку.
inline bool broken(const QString& text)
{
    return text.contains(QChar(0xfffd)) || clean(text) != text.normalized(QString::NormalizationForm_C);
}
} // namespace VkText
