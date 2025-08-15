#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QWidget>
#include <QSettings>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>

class QPfSettings : public QWidget
{
  Q_OBJECT

public:
  explicit QPfSettings(const QString& filename, QWidget *parent = nullptr);

private slots:
  void save(void);

private:
  QString m_Filename;
  QLineEdit *biosPathEdit;
  QComboBox *clockCombo;
  QComboBox *fontCombo;
  QCheckBox *showExtendedVram;
  QCheckBox *skipVerificationCheck;
  QCheckBox *tvPowwwCheck;
  QPushButton *saveButton;

  void load(void);
  void process(void);
};

#endif
