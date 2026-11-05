#pragma once
// 10-полосный параметрический эквалайзер с предусилением и пресетами.
#include <QObject>
#include <QAudioBuffer>
#include <QVariantList>
#include <array>
#include <vector>

class AudioEqualizer : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY changed)
    Q_PROPERTY(QVariantList gains READ gains NOTIFY changed)
    Q_PROPERTY(QVariantList frequencies READ frequencies CONSTANT)
    Q_PROPERTY(double preamp READ preamp WRITE setPreamp NOTIFY changed)
    Q_PROPERTY(bool autoHeadroom READ autoHeadroom WRITE setAutoHeadroom NOTIFY changed)
    Q_PROPERTY(QString preset READ preset NOTIFY changed)
    Q_PROPERTY(QStringList presets READ presets CONSTANT)
  public:
    explicit AudioEqualizer(QObject* parent = nullptr);
    bool enabled() const
    {
        return m_enabled;
    }
    QVariantList gains() const;
    QVariantList frequencies() const;
    double preamp() const
    {
        return m_preamp;
    }
    bool autoHeadroom() const
    {
        return m_autoHeadroom;
    }
    QString preset() const
    {
        return m_preset;
    }
    QStringList presets() const
    {
        return {"Ровно", "Тёплый бас", "Вокал", "Электроника", "Акустика"};
    }
    void setEnabled(bool value);
    void setPreamp(double value);
    void setAutoHeadroom(bool value);
    // Устанавливает усиление одной полосы (в дБ, от -12 до +12).
    Q_INVOKABLE void setBand(int index, double value);
    // Применяет готовый набор усилений по имени пресета.
    Q_INVOKABLE void selectPreset(const QString& name);
    // Сбрасывает все полосы к нейтральному пресету «Ровно».
    Q_INVOKABLE void reset();
    // Обрабатывает буфер звука эквалайзером (биквадратные фильтры на каждый канал).
    QAudioBuffer process(const QAudioBuffer& buffer);
    // Обнуляет состояние фильтров после смены настроек или формата.
    void clearHistory();
  signals:
    void changed();
    void enabledChanged();

  private:
    // Коэффициенты одного биквадратного (RBJ) фильтра-эквалайзера.
    struct Filter
    {
        double b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0;
    };
    // Состояние фильтра: два предыдущих значения входа/выхода для рекурсии.
    struct History
    {
        double z1 = 0, z2 = 0;
    };
    // Сохраняет настройки в QSettings и оповещает слушателей.
    void save();
    // Пересчитывает коэффициенты фильтров под новый формат/частоту дискретизации.
    void configure(int sampleRate, int channels);
    std::array<double, 10> m_gains{};
    std::array<Filter, 10> m_filters{};
    std::array<Filter, 10> m_targetFilters{};
    std::vector<std::array<History, 10>> m_history;
    bool m_enabled = false, m_autoHeadroom = true, m_dirty = true;
    double m_preamp = 0, m_level = 1, m_targetLevel = 1;
    int m_sampleRate = 0, m_channels = 0;
    int m_transitionFrames = 0;
    QString m_preset = "Ровно";
};
