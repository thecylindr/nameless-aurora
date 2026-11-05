#include "audioreactivity.h"
#include <algorithm>
#include <cmath>

void AudioReactivity::reset()
{
    *this = AudioReactivity{};
}
void AudioReactivity::process(const QAudioBuffer& buffer)
{
    if (!buffer.isValid())
        return;

    const auto format = buffer.format();
    const int rate = format.sampleRate(), channels = format.channelCount(), bytes = format.bytesPerSample();

    // Поддерживаем только те форматы, с которыми реально умеем работать;
    // всё остальное просто игнорируем.
    if (rate < 1000 || channels < 1 || channels > 8 || bytes < 1)
        return;

    // Сменился поток (другое устройство/формат) — вся накопленная история больше не имеет смысла.
    if (rate != m_rate || channels != m_channels) {
        reset();
        m_rate = rate;
        m_channels = channels;
    }

    // Коэффициенты однополюсных фильтров. `low` оставляет басовую полосу (срез ~170 Гц),
    // `dc` убирает постоянную составляющую, чтобы тихий сигнал не казался «громким».
    const double low = 1 - std::exp(-2 * 3.141592653589793 * 170 / rate);
    const double dc = 1 - std::exp(-2 * 3.141592653589793 * 3 / rate);

    const auto* data = buffer.constData<char>();
    for (int frame = 0; frame < buffer.frameCount(); ++frame) {
        for (int ch = 0; ch < channels; ++ch) {
            double sample = format.normalizedSampleValue(data + (frame * channels + ch) * bytes);

            // Защита от NaN/Inf из битого или нестандартного буфера.
            if (!std::isfinite(sample))
                sample = 0;
            sample = std::clamp(sample, -1.0, 1.0);

            // Убираем постоянную составляющую, затем копим энергию по всему диапазону и по басу.
            m_dc[ch] += dc * (sample - m_dc[ch]);
            sample -= m_dc[ch];
            m_low[ch] += low * (sample - m_low[ch]);
            m_power += sample * sample / channels;
            m_bassPower += m_low[ch] * m_low[ch] / channels;
        }

        // Анализ идёт фиксированными окнами по 10 мс, независимо от размера буфера декодера.
        if (++m_frames >= rate / 100)
            finishWindow();
    }
}
void AudioReactivity::finishWindow()
{
    const double dt = double(m_frames) / m_rate;
    const double energy = std::sqrt(m_power / m_frames), bass = std::sqrt(m_bassPower / m_frames);

    // Сглаживание огибающей: атака быстрая, спад медленный, чтобы интерфейс реагировал
    // живо, но не мерцал на каждом сэмпле.
    const auto envelope = [dt](double current, double target) {
        return current + (target - current) * (1 - std::exp(-dt / (target > current ? .025 : .19)));
    };

    m_time += dt;
    m_state.pulse *= std::exp(-dt / .17);

    // Рост низкочастотной энергии относительно её недавнего «пола» ловит удары.
    // Протяжный тон не сможет дёргать это повторно; рефрактерный период к тому же
    // не даёт одному удару породить несколько вспышек.
    if (bass < m_average * 1.15 + .003 || bass < .012)
        m_armed = true;
    if (m_armed && m_time > .08 && m_time - m_lastBeat > .19 && bass > .018 && bass > m_average * 1.55 + .008 &&
        bass - m_previous > .006) {
        m_state.pulse = std::clamp((bass - m_average) * 7, .35, 1.0);
        ++m_state.onsets;
        m_lastBeat = m_time;
        m_armed = false;
    }

    // Держим скользящее среднее по басу, чтобы «недавний пол» соответствовал реальному треку.
    m_average += (bass - m_average) * (1 - std::exp(-dt / .65));
    m_previous = bass;

    m_state.energy = envelope(m_state.energy, std::clamp(energy * 3, 0.0, 1.0));
    m_state.bass = envelope(m_state.bass, std::clamp(bass * 4, 0.0, 1.0));

    m_power = 0;
    m_bassPower = 0;
    m_frames = 0;
}
