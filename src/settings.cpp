#include "main.h"
#include "settings.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QFileDialog>
#include <QDir>

QPfSettings::QPfSettings(const QString& filename, QWidget *parent)
    : QWidget(parent), m_Filename(filename)
{
  biosPathEdit = new QLineEdit;
  QPushButton *browseButton = new QPushButton("Browse");
  connect(browseButton, &QPushButton::clicked, [this]() {
    QString dir = QFileDialog::getExistingDirectory(this, "Select BIOS Path");
    if (!dir.isEmpty()) biosPathEdit->setText(dir);
  });

  clockCombo = new QComboBox;
  clockCombo->addItem("NTSC", F8_CLOCK_CHANNEL_F_NTSC);
  clockCombo->addItem("PAL Gen I", F8_CLOCK_CHANNEL_F_PAL_GEN_1);
  clockCombo->addItem("PAL Gen II", F8_CLOCK_CHANNEL_F_PAL_GEN_2);

  fontCombo = new QComboBox;
  fontCombo->addItems({"Fairchild", "Cute", "Skinny"});

  showExtendedVram = new QCheckBox("Show Extended VRAM");
  skipVerificationCheck = new QCheckBox("Skip Cartridge Verification");
  tvPowwwCheck = new QCheckBox("Enable TV POWWW");

  saveButton = new QPushButton("Save");
  connect(saveButton, &QPushButton::clicked, this, &QPfSettings::save);

  QFormLayout *formLayout = new QFormLayout;
  formLayout->addRow("BIOS Path:", biosPathEdit);
  formLayout->addRow("", browseButton);
  formLayout->addRow("Clock Speed:", clockCombo);
  formLayout->addRow("Font:", fontCombo);
  formLayout->addRow("", showExtendedVram);
  formLayout->addRow("", skipVerificationCheck);
  formLayout->addRow("", tvPowwwCheck);

  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addLayout(formLayout);
  mainLayout->addWidget(saveButton);

  setLayout(mainLayout);

  load();
}

void QPfSettings::process(void)
{
  g_BiosPath = biosPathEdit->text();
  g_ChannelF.settings.f3850_clock_speed = clockCombo->currentData().toInt();
  g_ChannelF.settings.cf_full_vram = showExtendedVram->isChecked();
  g_ChannelF.settings.cf_skip_cartridge_verification = skipVerificationCheck->isChecked();
  g_ChannelF.settings.cf_tv_powww = tvPowwwCheck->isChecked();
}

void QPfSettings::load(void)
{
  QSettings settings(m_Filename, QSettings::IniFormat);
  
  biosPathEdit->setText(settings.value("bios_path", QDir::currentPath()).toString());
  clockCombo->setCurrentIndex(settings.value("clock_speed", 0).toInt());
  fontCombo->setCurrentIndex(settings.value("font", 0).toInt());
  showExtendedVram->setChecked(settings.value("screen_size", false).toBool());
  skipVerificationCheck->setChecked(settings.value("skip_cartridge_verification", true).toBool());
  tvPowwwCheck->setChecked(settings.value("tv_powww", false).toBool());
  process();
}

void QPfSettings::save(void)
{
  QSettings settings(m_Filename, QSettings::IniFormat);

  settings.setValue("bios_path", biosPathEdit->text());
  settings.setValue("clock_speed", clockCombo->currentIndex());
  settings.setValue("font", fontCombo->currentIndex());
  settings.setValue("screen_size", showExtendedVram->isChecked());
  settings.setValue("skip_cartridge_verification", skipVerificationCheck->isChecked());
  settings.setValue("tv_powww", tvPowwwCheck->isChecked());
  settings.sync();
  process();
}
