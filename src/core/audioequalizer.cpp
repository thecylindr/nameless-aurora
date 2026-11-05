#include "audioequalizer.h"
#include <QSettings>
#include <cmath>
#include <algorithm>
#include <cstring>

// Реализация эквалайзера: расчёт биквадратных фильтров, пресеты и обработка звука.
namespace
{
// Центральные частоты десяти полос эквалайзера (в герцах).
constexpr std::array<double, 10> centers{31.25, 62.5, 125, 250, 500, 1000, 2000, 4000, 8000, 16000};
}
AudioEqualizer::AudioEqualizer(QObject* parent) : QObject(parent)
{
    // Восстанавливаем сохранённые настройки, ограничивая значения разумными рамками.
    QSettings s;
    m_enabled = s.value("audio/eqEnabled", false).toBool();
    m_autoHeadroom = s.value("audio/eqHeadroom", true).toBool();
    const auto values = s.value("audio/eqBands").toList();
    for (int i = 0; i < 10; ++i) {
        const double v = values.value(i).toDouble();
        m_gains[i] = std::isfinite(v) ? std::clamp(v, -12., 12.) : 0;
    }
    const double p = s.value("audio/eqPreamp", 0).toDouble();
    m_preamp = std::isfinite(p) ? std::clamp(p, -18., 6.) : 0;
    m_preset = s.value("audio/eqPreset", "Ровно").toString();
}
QVariantList AudioEqualizer::gains() const
{
    QVariantList v;
    for (auto g : m_gains)
        v.append(g);
    return v;
}
QVariantList AudioEqualizer::frequencies() const
{
    QVariantList v;
    for (auto f : centers)
        v.append(f);
    return v;
}
// Сохраняет текущее состояние эквалайзера и уведомляет об изменениях.
void AudioEqualizer::save()
{
    m_dirty = true;
    QSettings s;
    s.setValue("audio/eqEnabled", m_enabled);
    s.setValue("audio/eqBands", gains());
    s.setValue("audio/eqPreamp", m_preamp);
    s.setValue("audio/eqHeadroom", m_autoHeadroom);
    s.setValue("audio/eqPreset", m_preset);
    emit changed();
}
void AudioEqualizer::setEnabled(bool v)
{
    if (v == m_enabled)
        return;
    m_enabled = v;
    clearHistory();
    save();
    emit enabledChanged();
}
void AudioEqualizer::setPreamp(double v)
{
    if (!std::isfinite(v))
        return;
    v = std::clamp(v, -18., 6.);
    if (qFuzzyCompare(v + 20, m_preamp + 20))
        return;
    m_preamp = v;
    save();
}
void AudioEqualizer::setAutoHeadroom(bool v)
{
    if (v == m_autoHeadroom)
        return;
    m_autoHeadroom = v;
    save();
}
void AudioEqualizer::setBand(int i, double v)
{
    if (i < 0 || i >= 10 || !std::isfinite(v))
        return;
    v = std::clamp(v, -12., 12.);
    if (qFuzzyCompare(v + 20, m_gains[i] + 20))
        return;
    m_gains[i] = v;
    m_preset = "Свой";
    save();
}
void AudioEqualizer::selectPreset(const QString& name)
{
    const int index = presets().indexOf(name);
    if (index < 0)
        return;
    // Профили усилений для пресетов в порядке presets(): «Ровно», «Тёплый бас»,
    // «Вокал», «Электроника», «Акустика».
    static const std::array<std::array<double, 10>, 5> profiles{{{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                                                 {5, 4, 3, 1, 0, -1, 0, 1, 1, 0},
                                                                 {-3, -2, -1, 0, 2, 3, 3, 1, 0, -1},
                                                                 {4, 3, 1, -1, -2, 0, 2, 3, 3, 2},
                                                                 {-1, 0, 1, 2, 2, 1, 1, 2, 1, 0}}};
    m_gains = profiles[index];
    m_preset = name;
    m_preamp = 0;
    save();
}
void AudioEqualizer::reset()
{
    selectPreset("Ровно");
}
void AudioEqualizer::clearHistory()
{
    for (auto& channel : m_history)
        for (auto& state : channel)
            state = {};
    m_dirty = true;
}
void AudioEqualizer::configure(int rate, int channels)
{
    const bool newFormat = rate != m_sampleRate || channels != m_channels;
    if (newFormat) {
        m_sampleRate = rate;
        m_channels = channels;
        m_history.assign(channels, {});
        m_dirty = true;
    }
    if (!m_dirty)
        return;
    m_dirty = false;
    for (int i = 0; i < 10; ++i) {
        // Коэффициенты полосового (peaking) RBJ-фильтра с добротностью Q = 1.4.
        const double a = std::pow(10., m_gains[i] / 40.),
                     w = 2 * 3.141592653589793 * std::min(centers[i], rate * .45) / rate;
        const double alpha = std::sin(w) / (2 * 1.4), c = std::cos(w), a0 = 1 + alpha / a;
        m_targetFilters[i] = {(1 + alpha * a) / a0, -2 * c / a0, (1 - alpha * a) / a0, -2 * c / a0,
                              (1 - alpha / a) / a0};
    }
    if (newFormat) {
        // Новый формат — применяем фильтры сразу, без плавного перехода.
        m_filters = m_targetFilters;
        m_transitionFrames = 0;
    } else
        // При изменении настроек плавно подтягиваем коэффициенты за ~20 мс.
        m_transitionFrames = std::max(1, rate / 50);
    const double peak = *std::max_element(m_gains.begin(), m_gains.end());
    // При включённом auto-headroom снижаем уровень на величину максимального усиления,
    // чтобы избежать клиппинга.
    m_targetLevel = std::pow(10., (m_preamp - (m_autoHeadroom ? std::max(0., peak) : 0)) / 20.);
}
QAudioBuffer AudioEqualizer::process(const QAudioBuffer& buffer)
{
    if (!m_enabled || !buffer.isValid())
        return buffer;
    const auto format = buffer.format();
    if (format.sampleRate() <= 0 || format.channelCount() <= 0)
        return buffer;
    configure(format.sampleRate(), format.channelCount());
    QByteArray data(buffer.byteCount(), Qt::Uninitialized);
    const auto* input = buffer.constData<char>();
    const int size = format.bytesPerSample();
    // Постоянная времени ~20 мс для плавного изменения общего уровня громкости.
    const double smoothing = 1 - std::exp(-1. / (.02 * m_sampleRate));
    for (int frame = 0; frame < buffer.frameCount(); ++frame) {
        if (m_transitionFrames > 0) {
            // Линейно интерполируем коэффициенты к целевым, чтобы не было щелчков.
            for (int i = 0; i < 10; ++i) {
                auto& f = m_filters[i];
                const auto& t = m_targetFilters[i];
                const double n = m_transitionFrames;
                f.b0 += (t.b0 - f.b0) / n;
                f.b1 += (t.b1 - f.b1) / n;
                f.b2 += (t.b2 - f.b2) / n;
                f.a1 += (t.a1 - f.a1) / n;
                f.a2 += (t.a2 - f.a2) / n;
            }
            --m_transitionFrames;
        }
        m_level += (m_targetLevel - m_level) * smoothing;
        for (int channel = 0; channel < m_channels; ++channel) {
            const int index = frame * m_channels + channel;
            double sample = format.normalizedSampleValue(input + index * size) * m_level;
            // Последовательно пропускаем сэмпл через все 10 биквадратных фильтров.
            for (int band = 0; band < 10; ++band) {
                const auto& f = m_filters[band];
                auto& h = m_history[channel][band];
                const double out = f.b0 * sample + h.z1;
                h.z1 = f.b1 * sample - f.a1 * out + h.z2;
                h.z2 = f.b2 * sample - f.a2 * out;
                sample = out;
            }
            sample = std::isfinite(sample) ? std::clamp(sample, -1., 1.) : 0;
            char* target = data.data() + index * size;
            // Возвращаем сэмпл обратно в исходный формат буфера.
            switch (format.sampleFormat()) {
            case QAudioFormat::Float: {
                float v = float(sample);
                memcpy(target, &v, 4);
                break;
            }
            case QAudioFormat::Int16: {
                qint16 v = qint16(std::clamp(std::lround(sample * 32767), -32768L, 32767L));
                memcpy(target, &v, 2);
                break;
            }
            case QAudioFormat::Int32: {
                qint32 v = qint32(std::clamp(sample * 2147483647., -2147483648., 2147483647.));
                memcpy(target, &v, 4);
                break;
            }
            case QAudioFormat::UInt8:
                *reinterpret_cast<quint8*>(target) = quint8(std::clamp(std::lround(sample * 127 + 128), 0L, 255L));
                break;
            default:
                return buffer;
            }
        }
    }
    return QAudioBuffer(data, format, buffer.startTime());
}
