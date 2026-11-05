#pragma once
// Анализ «энергии» звука для визуальных эффектов и пульсации под бит.
#include <QAudioBuffer>
#include <array>

// Маленькие потоковые огибающие, а не оценка темпа. Анализ ведётся окнами
// по 10 мс независимо от размера буфера декодера и никогда не меняет звук.
class AudioReactivity
{
  public:
    struct State
    {
        double energy = 0, bass = 0, pulse = 0;
        quint64 onsets = 0;
    };
    // Подаёт очередной буфер декодированного звука и накапливает огибающие.
    void process(const QAudioBuffer& buffer);
    // Сбрасывает накопленные значения и возвращает индикаторы в исходное состояние.
    void reset();
    State state() const
    {
        return m_state;
    }

  private:
    // Завершает накопленное окно и пересчитывает показатели энергии/баса/пульса.
    void finishWindow();
    State m_state;
    std::array<double, 8> m_low{}, m_dc{};
    int m_rate = 0, m_channels = 0, m_frames = 0;
    bool m_armed = true;
    double m_power = 0, m_bassPower = 0, m_average = 0, m_previous = 0, m_time = 0, m_lastBeat = -1;
};
